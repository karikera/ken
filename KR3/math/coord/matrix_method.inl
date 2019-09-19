#pragma once

#include "matrix_method.h"

#ifdef WIN32
#include "mmx.h"
#endif

namespace kr
{
	namespace math
	{
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		ATTR_INLINE vector<T, cols, aligned>& matrix_method_common<T, rows, cols, aligned, type>::operator [](size_t idx) noexcept
		{
			return v[idx];
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		ATTR_INLINE const vector<T, cols, aligned>& matrix_method_common<T, rows, cols, aligned, type>::operator [](size_t idx) const noexcept
		{
			return v[idx];
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline bool matrix_method_common<T, rows, cols, aligned, type>::operator ==(const matrix_method_common& m) const noexcept
		{
			return mem::equals(this, &m, sizeof(m));
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline bool matrix_method_common<T, rows, cols, aligned, type>::operator !=(const matrix_method_common& m) const noexcept
		{
			return !mem::equals(this, &m, sizeof(m));
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline const matrix<T, rows, cols, aligned, type> matrix_method_common<T, rows, cols, aligned, type>::operator *(const matrix_method_common& m) const noexcept
		{
			static_assert(rows == cols, "mismatch count ");
			matrix<T, rows, cols, aligned, type> out;
			constexpr size_t others_cols = rows;

			for (size_t i = 0; i < rows; i++)
			{
				for (size_t j = 0; j < others_cols; j++)
				{
					T sum = v[i][0] * m.v[0][j];
					for (size_t k = 1; k < cols; k++)
					{
						sum += v[i][k] * m.v[k][j];
					}
					out.v[i].m_data[j] = sum;
				}
			}
			return out;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline const vector<T, rows, aligned> matrix_method_common<T, rows, cols, aligned, type>::operator *(const vector<T, cols, aligned>& o) const noexcept
		{
			vector<T, rows> out;
			for (size_t i = 0; i < rows; i++)
			{
				T sum = v[i][0] * o[0];
				for (size_t k = 1; k < cols; k++)
				{
					sum += v[i][k] * o[k];
				}
				out.m_data[i] = sum;
			}
			return out;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		ATTR_INLINE matrix<T, rows, cols, aligned, type>& matrix_method_common<T, rows, cols, aligned, type>::operator *=(const matrix_method_common& m) noexcept
		{
			return *this = *this * m;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline matrix<T, rows, cols, aligned, type>& matrix_method_common<T, rows, cols, aligned, type>::operator +=(const matrix_method_common& m) noexcept
		{
			const T* src = (T*)(&m);
			T* dst = (T*)(this);
			T* dstend = dst + rows * cols;

			while (dst != dstend)
			{
				*dst++ += *src++;
			}
			return (matrix<T, rows, cols, aligned, type>&)*this;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline const matrix<T, rows, cols, aligned, type> matrix_method_common<T, rows, cols, aligned, type>::operator +(const matrix_method_common& m) const noexcept
		{
			matrix<T, rows, cols, aligned, type> out;
			const T* src1 = (T*)(this);
			const T* src2 = (T*)(&m);
			T* dst = (T*)(&out);
			T* dstend = dst + rows * cols;

			while (dst != dstend)
			{
				*dst++ = *src1++ + *src2++;
			}
			return out;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline matrix<T, rows, cols, aligned, type>& matrix_method_common<T, rows, cols, aligned, type>::operator -=(const matrix_method_common& m) noexcept
		{
			const T* src = (T*)(&m);
			T* dst = (T*)(this);
			T* dstend = dst + rows * cols;

			while (dst != dstend)
			{
				*dst++ -= *src++;
			}
			return (matrix<T, rows, cols, aligned, type>&)*this;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline const matrix<T, rows, cols, aligned, type> matrix_method_common<T, rows, cols, aligned, type>::operator -(const matrix_method_common& m) const noexcept
		{
			matrix<T, rows, cols, aligned, type> out;
			const T* src1 = (T*)(this);
			const T* src2 = (T*)(&m);
			T* dst = (T*)(&out);
			T* dstend = dst + rows * cols;

			while (dst != dstend)
			{
				*dst++ = *src1++ - *src2++;
			}
			return out;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline bool matrix_method_common<T, rows, cols, aligned, type>::similar(const matrix_method_common& o) const noexcept
		{
			const float almostZero = 1.0e-6f;

			const vector<T, cols, aligned>* dest = v;
			const vector<T, cols, aligned>* src = o.v;
			const vector<T, cols, aligned>* end = dest + rows;
			T dist = (*src - *dest).length_sq();
			while (dest != end)
			{
				dist += (*src - *dest).length_sq();
				dest++;
				src++;
			}
			return dist <= almostZero;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		inline const matrix<T, cols, rows, aligned, type> matrix_method_common<T, rows, cols, aligned, type>::transpose() const noexcept
		{
			matrix<T, cols, rows, aligned, type> out;

			float* dest = (float*)(&out);
			const float* src = (float*)(&v);
			for (size_t r = 0; r < rows; r++)
			{
				for (size_t c = 0; c < cols; c++)
				{
					dest[r + c * rows] = src[c + r * cols];
				}
			}

			return out;
		}
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		template <int ... args>
		inline const matrix<T, rows, cols, aligned, type> matrix_method_common<T, rows, cols, aligned, type>::__shuffle() const noexcept
		{
			static_assert(sizeof ... (args) == cols * rows, "size unmatch");
			matrix<T, rows, cols, aligned, type> out;
			auto values = { args ... };
			T* dest = (T*)(&out);
			T* src = (T*)(v);

			for (int v : values) * dest++ = src[v];
			return out;
		}

		template <typename T, size_t cols, bool aligned>
		template <typename LAMBDA, typename LAMBDA2>
		bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::right_bottom>::deltaCompare(const irect & dest, const LAMBDA & oldcallback, const LAMBDA2 & newcallback) const noexcept
		{
			int l = dest.from.x;
			int t = dest.from.y;
			int r = dest.to.x;
			int b = dest.to.y;
			int pl = from.x;
			int pt = from.y;
			int pr = to.x;
			int pb = to.y;

			if (l == pl && t == pt && r == pr && b == pb) return false;
			if (pr < l || r < pl || pb < t || b < pt)
			{
				oldcallback(irect(pl, pt, pr, pb));
				newcallback(irect(l, t, r, b));
			}
			else
			{
				int it = t > pt ? t : pt;
				int ib = b > pb ? pb : b;

				if (t > pt) oldcallback(irect(pl, pt, pr, t));
				if (b < pb) oldcallback(irect(pl, b, pr, pb));

				if (r < pr) oldcallback(irect(r, it, pr, ib));
				if (l > pl) oldcallback(irect(pl, it, l, ib));

				if (r > pr) newcallback(irect(pr, it, r, ib));
				if (l < pl) newcallback(irect(l, it, pl, ib));

				if (t < pt) newcallback(irect(l, t, r, pt));
				if (b > pb) newcallback(irect(l, pb, r, b));
			}
			return true;
		}

		template <typename T, size_t cols, bool aligned>
		template <typename LAMBDA, typename LAMBDA2>
		bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::right_bottom>::deltaMove(const irect & dest, const LAMBDA & oldcallback, const LAMBDA2 & newcallback) noexcept
		{
			if (!deltaCompare(dest, oldcallback, newcallback)) return false;
			*this = dest;
			return true;
		}

		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE const vector<T, cols, aligned> matrix_method_rect<T, 2, cols, aligned, matrix_data_type::width_height>::clip(const vector<T, cols, aligned> & o) const noexcept
		{
			vector<T, cols, aligned> out;
			for (size_t c = 0; c < cols; c++)
			{
				if (pos[c] > o[c])
				{
					out[c] = pos[c];
					continue;
				}
				T end = pos[c] + size[c];
				if (end < o[c])
				{
					out[c] = end;
					continue;
				}
				out[c] = o[c];
			}
			return out;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::width_height>::contains(const vector<T, cols, aligned> & o) const noexcept
		{
			for (size_t c = 0; c < cols; c++)
			{
				if (pos[c] > o[c] || o[c] >= pos[c] + size[c]) return false;
			}
			return true;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::width_height>::getIntersect(
			matrix<T, 2, cols, aligned, matrix_data_type::width_height> * _out,
			const matrix<T, 2, cols, aligned, matrix_data_type::width_height> & rc) const noexcept
		{
			for (size_t c = 0; c < cols; c++)
			{
				T from = maxt(pos[c], rc.pos[c]);
				T to = mint(pos[c] + size[c], rc.pos[c] + rc.size[c]);
				if (to <= from)
					return false;
				_out->pos[c] = from;
				_out->size[c] = to - from;
			}
			return true;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::width_height>::intersect(const matrix<T, 2, cols, aligned, matrix_data_type::width_height> & rc) const noexcept
		{
			for (size_t c = 0; c < cols; c++)
			{
				if (pos[c] <= rc.pos[c] + rc.size[c]) return false;
				if (rc.pos[c] <= pos[c] + size[c]) return false;
			}
			return true;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE const vector<T, cols, aligned> matrix_method_rect<T, 2, cols, aligned, matrix_data_type::right_bottom>::clip(const vector<T, cols, aligned> & o) const noexcept
		{
			vector<T, cols, aligned> out;
			for (size_t c = 0; c < cols; c++)
			{
				if (from[c] > o[c])
				{
					out[c] = from[c];
					continue;
				}
				if (to[c] < o[c])
				{
					out[c] = to[c];
					continue;
				}
				out[c] = o[c];
			}
			return out;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::right_bottom>::contains(const vector<T, cols, aligned> & o) const noexcept
		{
			for (size_t c = 0; c < cols; c++)
			{
				if (from[c] > o[c]) return false;
				if (o[c] >= to[c]) return false;
			}
			return true;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::right_bottom>::getIntersect(
			matrix<T, 2, cols, aligned, matrix_data_type::right_bottom> * _out,
			const matrix<T, 2, cols, aligned, matrix_data_type::right_bottom> & rc) const noexcept
		{
			for (size_t c = 0; c < cols; c++)
			{
				T f = maxt(from[c], rc.from[c]);
				T t = mint(to[c], rc.to[c]);
				if (t <= f)
					return false;
				_out->from[c] = f;
				_out->to[c] = t;
			}
			return true;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE bool matrix_method_rect<T, 2, cols, aligned, matrix_data_type::right_bottom>::intersect(const matrix<T, 2, cols, aligned, matrix_data_type::right_bottom> & rc) const noexcept
		{
			for (size_t c = 0; c < cols; c++)
			{
				if (from[c] <= rc.to[c]) return false;
				if (rc.from[c] <= to[c]) return false;
			}
			return true;
		}

		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 3, aligned> matrix_method<T, 3, 3, aligned>::identity() noexcept
		{
			return{
				{ 1,0,0 },
				{ 0,1,0 },
				{ 0,0,1 } };
		}


		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 2, aligned> matrix_method<T, 3, 2, aligned>::getX() const noexcept
		{
			return v[0];
		}
		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 2, aligned> matrix_method<T, 3, 2, aligned>::getY() const noexcept
		{
			return v[1];
		}
		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 2, aligned> matrix_method<T, 3, 2, aligned>::getPos() const noexcept
		{
			return v[2];
		}

		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::inverse() const noexcept
		{
			T det = 1.f / (_11*_22 - _12 * _21);

			float _22_det = _22 * det;
			float _21_det = _21 * det;
			float _12_det = _12 * det;
			float _11_det = _11 * det;

			return {
				_22_det, -_12_det,
				-_21_det, _11_det,
				_32 * _21_det - _31 * _22_det,
				_31 * _12_det - _32 * _11_det
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::scaling(T scale) noexcept
		{
			_11 *= scale;
			_12 *= scale;
			_21 *= scale;
			_22 *= scale;
			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::preScale(const vector<T, 2, aligned> & scale) noexcept
		{
			v[0] *= scale.x;
			v[1] *= scale.y;
			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::postScale(const vector<T, 2, aligned> & scale) noexcept
		{
			v[0] *= scale;
			v[1] *= scale;
			v[2] *= scale;
			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::preRotate(T angle) noexcept
		{
			T c = (T)math::cos((float)angle);
			T s = (T)math::sin((float)angle);
			vec2 v_0_ = v[0];
			v[0] = v_0_ * c + v[1] * s;
			v[1] = v[1] * c - v_0_ * s;

			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::postRotate(T angle) noexcept
		{
			T c = (T)math::cos((float)angle);
			T s = (T)math::sin((float)angle);

			float t = _11;
			_11 = t * c - _12 * s;
			_12 = t * s + _12 * c;
			t = _21;
			_21 = t * c - _22 * s;
			_22 = t * s + _22 * c;
			t = _31;
			_31 = t * c - _32 * s;
			_32 = t * s + _32 * c;

			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::preTranslate(const vector<T, 2, aligned> & pos) noexcept
		{
			v[2] = v[0] * pos.x + v[1] * pos.y;
			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::postTranslate(const vector<T, 2, aligned> & pos) noexcept
		{
			v[2] += pos;
			return *static_cast<matrix<T, 3, 2, aligned>*>(this);
		}

		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::translate(vector<T, 2> pos) noexcept
		{
			return{
				{ 1,0 },
				{ 0,1 },
				{ pos.x,pos.y }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::identity() noexcept
		{
			return{
				{ 1,0 },
				{ 0,1 },
				{ 0,0 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::scale(const vector<T, 2, aligned> &scale) noexcept
		{
			return{
				{ scale.x,0 },
				{ 0,scale.y },
				{ 0,0 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::scale(T x) noexcept
		{
			return{
				{ x,0 },
				{ 0,x },
				{ 0,0 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::rotate(T rad) noexcept
		{
			T c = (T)math::cos((float)rad);
			T s = (T)math::sin((float)rad);
			return{
				{ c,s },
				{ (T)-s,c },
				{ 0,0 }
			};
		}

		template <typename T, bool aligned>
		inline const matrix<T, 3, 2, aligned> matrix_method<T, 3, 2, aligned>::operator *(const matrix_method<T, 3, 2, aligned>& m) const noexcept
		{
			matrix<T, 3, 2, aligned> out;
			out._11 = _11 * m._11 + _12 * m._21;
			out._12 = _11 * m._12 + _12 * m._22;
			out._21 = _21 * m._11 + _22 * m._21;
			out._22 = _21 * m._12 + _22 * m._22;
			out._31 = _31 * m._11 + _32 * m._21 + m._31;
			out._32 = _31 * m._12 + _32 * m._22 + m._32;
			return out;
		}
		template <typename T, bool aligned>
		inline const vector<T, 2, aligned> matrix_method<T, 3, 2, aligned>::operator *(const vector<T, 2, aligned>& o) const noexcept
		{
			vector<T, 2, aligned> out;
			out.x = o.x + _11 + o.y * _21 + _31;
			out.y = o.x + _12 + o.y * _22 + _32;
			return out;
		}
		template <typename T, bool aligned>
		ATTR_INLINE matrix<T, 3, 2, aligned>& matrix_method<T, 3, 2, aligned>::operator *=(const matrix_method<T, 3, 2, aligned>& m) noexcept
		{
			return *this = *this * m;
		}

		template <typename T, bool aligned>
		template <Axis axis>
		ATTR_INLINE const vector<T, 4, aligned> matrix_method<T, 4, 4, aligned>::get() const noexcept
		{
			constexpr size_t i = (size_t)axis;
			return{ v[0][i],v[1][i],v[2][i],v[3][i] };
		}
		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 4, aligned> matrix_method<T, 4, 4, aligned>::getX() const noexcept
		{
			return get<Axis::X>();
		}
		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 4, aligned> matrix_method<T, 4, 4, aligned>::getY() const noexcept
		{
			return get<Axis::Y>();
		}
		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 4, aligned> matrix_method<T, 4, 4, aligned>::getZ() const noexcept
		{
			return get<Axis::Z>();
		}
		template <typename T, bool aligned>
		ATTR_INLINE const vector<T, 4, aligned> matrix_method<T, 4, 4, aligned>::getPos() const noexcept
		{
			return get<Axis::W>();
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::inverse() const noexcept
		{
			notImplementedYet();
		}
		template <typename T, bool aligned>
		ATTR_INLINE void matrix_method<T, 4, 4, aligned>::scaling(T scale) noexcept
		{
			_11 *= scale;
			_12 *= scale;
			_13 *= scale;
			_21 *= scale;
			_22 *= scale;
			_23 *= scale;
			_31 *= scale;
			_32 *= scale;
			_33 *= scale;
		}
		template <typename T, bool aligned>
		ATTR_INLINE void matrix_method<T, 4, 4, aligned>::preTranslate(const vector<T, 4, aligned>& offset) noexcept
		{
			_14 = dot(v[0], offset);
			_24 = dot(v[1], offset);
			_34 = dot(v[2], offset);
			_44 = dot(v[3], offset);
		}
		template <typename T, bool aligned>
		ATTR_INLINE void matrix_method<T, 4, 4, aligned>::postTranslate(const vector<T, 4, aligned>& offset) noexcept
		{
			_14 += offset.x;
			_24 += offset.y;
			_34 += offset.z;
			_44 += offset.w;
		}
		template <typename T, bool aligned>
		ATTR_INLINE void matrix_method<T, 4, 4, aligned>::preScale(const vector<T, 4, aligned> & scale) noexcept
		{
			v[0] *= scale;
			v[1] *= scale;
			v[2] *= scale;
			v[3] *= scale;
		}
		template <typename T, bool aligned>
		ATTR_INLINE void matrix_method<T, 4, 4, aligned>::postScale(const vector<T, 4, aligned> & scale) noexcept
		{
			v[0] *= scale.x;
			v[1] *= scale.y;
			v[2] *= scale.z;
			v[3] *= scale.w;
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::world(const vector<T, 4, aligned> &pos, const vector<T, 4, aligned> &forward, const vector<T, 4, aligned> &up) noexcept
		{
			vector<T, 4, aligned> z = -forward.normalize();  // Forward
			vector<T, 4, aligned> x = cross(up, z).normalize(); // Right
			vector<T, 4, aligned> y = cross(z, x);

			return{
				{ x.x, y.x, z.x, pos.x },
				{ x.y, y.y, z.y, pos.y },
				{ x.z, y.z, z.z, pos.z },
				{ 0, 0, 0, 1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::look(const vector<T, 4, aligned> &eye, const vector<T, 4, aligned> &forward, const vector<T, 4, aligned> &up) noexcept
		{
			vector<T, 4, aligned> z = -forward.normalize();  // Forward
			vector<T, 4, aligned> x = cross(up, z).normalize(); // Right
			vector<T, 4, aligned> y = cross(z, x);

			return{
				{ x.x, x.y, x.z, (T)-dot(x,eye) },
				{ y.x, y.y, y.z, (T)-dot(y,eye) },
				{ z.x, z.y, z.z, (T)-dot(z,eye) },
				{ 0, 0, 0, 1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::lookAt(const vector<T, 4, aligned> &eye, const vector<T, 4, aligned> &to, const vector<T, 4, aligned> &up) noexcept
		{
			return look(eye, to - eye, up);
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::translate(vector<T, 3> pos) noexcept
		{
			return{
				{ 1,0,0,pos.x },
				{ 0,1,0,pos.y },
				{ 0,0,1,pos.z },
				{ 0,0,0,1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::identity() noexcept
		{
			return{
				{ 1,0,0,0 },
				{ 0,1,0,0 },
				{ 0,0,1,0 },
				{ 0,0,0,1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::scale(const vector<T, 4, aligned> &scale) noexcept
		{
			return{
				{ scale.x,0,0,0 },
				{ 0,scale.y,0,0 },
				{ 0,0,scale.z,0 },
				{ 0,0,0,1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::scale(T x) noexcept
		{
			return scale(vector<T, 4, aligned>(x, x, x, 1));
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::rotateX(T rad) noexcept
		{
			T c = (T)math::cos((float)rad);
			T s = (T)math::sin((float)rad);
			return{
				{ 1,0,0,0 },
				{ 0,c,(T)-s,0 },
				{ 0,s,c,0 },
				{ 0,0,0,1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::rotateY(T rad) noexcept
		{
			T c = (T)math::cos((float)rad);
			T s = (T)math::sin((float)rad);
			return{
				{ c,0,s,0 },
				{ 0,1,0,0 },
				{ (T)-s,0,c,0 },
				{ 0,0,0,1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::rotateZ(T rad) noexcept
		{
			T c = (T)math::cos((float)rad);
			T s = (T)math::sin((float)rad);
			return{
				{ c,(T)-s,0,0 },
				{ s,c,0,0 },
				{ 0,0,1,0 },
				{ 0,0,0,1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::rotateAxis(const vector<T, 4, aligned> & pos, T rad) noexcept
		{
			return (matrix<T, 4, 4, aligned>)quaterniona::rotateAxis((vec4a)pos, (float)rad).toMatrix();
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::orthogonal(T width, T height) noexcept
		{
			T _2w = 2 / width;
			T _2h = -2 / height;
			return{
				{ _2w, 0, 0, (T)-1 },
				{ 0, _2h, 0, 1 },
				{ 0, 0, (T)-1, 1 },
				{ 0, 0, 0, 1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::orthogonal(T left, T top, T right, T bottom, T _near, T _far) noexcept
		{
			T _w = 1 / (left - right);
			T _h = 1 / (bottom - top);
			T _z = 1 / (_near - _far);

			return{
				{ (T)(-2 * _w), 0, 0, (T)((right + left)*_w) },
				{ 0, (T)(-2 * _h), 0, (T)((top + bottom)*_h) },
				{ 0, 0, _z, (T)(-_far*_z) },
				{ 0, 0, 0, 1 }
			};
		}
		template <typename T, bool aligned>
		ATTR_INLINE const matrix<T, 4, 4, aligned> matrix_method<T, 4, 4, aligned>::fov(T fovy, T aspect, T _near, T _far) noexcept
		{
			T h = (T)(1.f / math::tan((float)fovy * 0.5f));
			T w = h / aspect;
			T Q = _far / (_near - _far);

			return{
				{ w, 0, 0, 0 },
				{ 0, h, 0, 0 },
				{ 0, 0, (T)Q, (T)(Q*_near) },
				{ 0, 0, (T)-1, 0 }
			};
		}

		namespace
		{
			inline void ignore_init(void * t) noexcept { };
			inline void m4x4_inverse(float * dest, const float * src) noexcept
			{
#ifdef WIN32
				__m128 minor0, minor1, minor2, minor3;
				__m128 row0, row1, row2, row3;
				__m128 det, tmp1;
				ignore_init(&tmp1);
				ignore_init(&row1);
				ignore_init(&row3);

				tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src + 4));
				row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src + 8)), (__m64*)(src + 12));
				row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
				row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
				tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src + 2)), (__m64*)(src + 6));
				row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src + 10)), (__m64*)(src + 14));
				row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
				row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
				// -----------------------------------------------
				tmp1 = _mm_mul_ps(row2, row3);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
				minor0 = _mm_mul_ps(row1, tmp1);
				minor1 = _mm_mul_ps(row0, tmp1);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
				minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
				minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
				minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
				// -----------------------------------------------
				tmp1 = _mm_mul_ps(row1, row2);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
				minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
				minor3 = _mm_mul_ps(row0, tmp1);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
				minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
				minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
				minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
				// -----------------------------------------------
				tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
				row2 = _mm_shuffle_ps(row2, row2, 0x4E);
				minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
				minor2 = _mm_mul_ps(row0, tmp1);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
				minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
				minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
				minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
				// -----------------------------------------------
				tmp1 = _mm_mul_ps(row0, row1); // Streaming SIMD Extensions - Inverse of 4x4 Matrix

				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
				minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
				minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
				minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
				minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
				// -----------------------------------------------
				tmp1 = _mm_mul_ps(row0, row3);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
				minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
				minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
				minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
				minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
				// -----------------------------------------------
				tmp1 = _mm_mul_ps(row0, row2);
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
				minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
				minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
				tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
				minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
				minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
				// -----------------------------------------------
				det = _mm_mul_ps(row0, minor0);
				det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
				det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
				tmp1 = _mm_rcp_ss(det);
				det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
				det = _mm_shuffle_ps(det, det, 0x00);
				minor0 = _mm_mul_ps(det, minor0);
				_mm_storel_pi((__m64*)(dest), minor0);
				_mm_storeh_pi((__m64*)(dest + 2), minor0);
				minor1 = _mm_mul_ps(det, minor1);
				_mm_storel_pi((__m64*)(dest + 4), minor1);
				_mm_storeh_pi((__m64*)(dest + 6), minor1);
				minor2 = _mm_mul_ps(det, minor2);
				_mm_storel_pi((__m64*)(dest + 8), minor2);
				_mm_storeh_pi((__m64*)(dest + 10), minor2);
				minor3 = _mm_mul_ps(det, minor3);
				_mm_storel_pi((__m64*)(dest + 12), minor3);
				_mm_storeh_pi((__m64*)(dest + 14), minor3);
#else
				struct M
				{
					float m00, m01, m02, m03;
					float m10, m11, m12, m13;
					float m20, m21, m22, m23;
					float m30, m31, m32, m33;
				};
				M & m = *(M*)src;
				float A2323 = m.m22 * m.m33 - m.m23 * m.m32 ;
				float A1323 = m.m21 * m.m33 - m.m23 * m.m31 ;
				float A1223 = m.m21 * m.m32 - m.m22 * m.m31 ;
				float A0323 = m.m20 * m.m33 - m.m23 * m.m30 ;
				float A0223 = m.m20 * m.m32 - m.m22 * m.m30 ;
				float A0123 = m.m20 * m.m31 - m.m21 * m.m30 ;
				float A2313 = m.m12 * m.m33 - m.m13 * m.m32 ;
				float A1313 = m.m11 * m.m33 - m.m13 * m.m31 ;
				float A1213 = m.m11 * m.m32 - m.m12 * m.m31 ;
				float A2312 = m.m12 * m.m23 - m.m13 * m.m22 ;
				float A1312 = m.m11 * m.m23 - m.m13 * m.m21 ;
				float A1212 = m.m11 * m.m22 - m.m12 * m.m21 ;
				float A0313 = m.m10 * m.m33 - m.m13 * m.m30 ;
				float A0213 = m.m10 * m.m32 - m.m12 * m.m30 ;
				float A0312 = m.m10 * m.m23 - m.m13 * m.m20 ;
				float A0212 = m.m10 * m.m22 - m.m12 * m.m20 ;
				float A0113 = m.m10 * m.m31 - m.m11 * m.m30 ;
				float A0112 = m.m10 * m.m21 - m.m11 * m.m20 ;

				float det = m.m00 * ( m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223 ) 
					- m.m01 * ( m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223 ) 
					+ m.m02 * ( m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123 ) 
					- m.m03 * ( m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123 ) ;
				det = 1 / det;

				*dest++ = det *   ( m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223 );
				*dest++ = det * - ( m.m01 * A2323 - m.m02 * A1323 + m.m03 * A1223 );
				*dest++ = det *   ( m.m01 * A2313 - m.m02 * A1313 + m.m03 * A1213 );
				*dest++ = det * - ( m.m01 * A2312 - m.m02 * A1312 + m.m03 * A1212 );
				*dest++ = det * - ( m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223 );
				*dest++ = det *   ( m.m00 * A2323 - m.m02 * A0323 + m.m03 * A0223 );
				*dest++ = det * - ( m.m00 * A2313 - m.m02 * A0313 + m.m03 * A0213 );
				*dest++ = det *   ( m.m00 * A2312 - m.m02 * A0312 + m.m03 * A0212 );
				*dest++ = det *   ( m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123 );
				*dest++ = det * - ( m.m00 * A1323 - m.m01 * A0323 + m.m03 * A0123 );
				*dest++ = det *   ( m.m00 * A1313 - m.m01 * A0313 + m.m03 * A0113 );
				*dest++ = det * - ( m.m00 * A1312 - m.m01 * A0312 + m.m03 * A0112 );
				*dest++ = det * - ( m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123 );
				*dest++ = det *   ( m.m00 * A1223 - m.m01 * A0223 + m.m02 * A0123 );
				*dest++ = det * - ( m.m00 * A1213 - m.m01 * A0213 + m.m02 * A0113 );
				*dest++ = det *   ( m.m00 * A1212 - m.m01 * A0212 + m.m02 * A0112 );
#endif
			}
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::inverse() const noexcept
		{
			mat4a out;
			m4x4_inverse(&out[0][0], &v[0][0]);
			return out;
		}
		template <>
		ATTR_INLINE void matrix_method<float, 4, 4, true>::scaling(float scale) noexcept
		{
			vec4a vscale = vec4a::makes(scale);
			vscale = vscale.shuffle<0, 0, 0, 1>() + CV_0001;
			preScale(vscale);
		}
		template <>
		ATTR_INLINE void matrix_method<float, 4, 4, true>::preTranslate(const vec4a& offset) noexcept
		{
			v[0] = v[0].shuffle<3, 1, 2, 0>().movs(dotV(v[0], offset)).shuffle<3, 1, 2, 0>();
			v[1] = v[1].shuffle<3, 1, 2, 0>().movs(dotV(v[1], offset)).shuffle<3, 1, 2, 0>();
			v[2] = v[2].shuffle<3, 1, 2, 0>().movs(dotV(v[2], offset)).shuffle<3, 1, 2, 0>();
			v[3] = v[3].shuffle<3, 1, 2, 0>().movs(dotV(v[3], offset)).shuffle<3, 1, 2, 0>();
		}
		template <>
		ATTR_INLINE void matrix_method<float, 4, 4, true>::postTranslate(const vec4a& offset) noexcept
		{
			v[0] += ((vec4a&)CV_ZERO).movs(offset.getXV()).shuffle<3, 1, 2, 0>();
			v[1] += ((vec4a&)CV_ZERO).movs(offset.getYV()).shuffle<3, 1, 2, 0>();
			v[2] += ((vec4a&)CV_ZERO).movs(offset.getZV()).shuffle<3, 1, 2, 0>();
			v[3] += ((vec4a&)CV_ZERO).movs(offset.getWV()).shuffle<3, 1, 2, 0>();
		}
		template <>
		ATTR_INLINE void matrix_method<float, 4, 4, true>::postScale(const vec4a & scale) noexcept
		{
			v[0] *= scale.getXV();
			v[1] *= scale.getYV();
			v[2] *= scale.getZV();
			v[3] *= scale.getWV();
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::world(const vec4a &pos, const vec4a &forward, const vec4a &up) noexcept
		{
			vec4a z = -forward.normalize();  // Forward
			vec4a x = cross(up, z).normalize(); // Right
			vec4a y = cross(z, x);

			return{
				{ x.x, y.x, z.x, pos.x },
				{ x.y, y.y, z.y, pos.y },
				{ x.z, y.z, z.z, pos.z },
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::look(const vec4a &eye, const vec4a &forward, const vec4a &up) noexcept
		{
			vec4a z = -forward.normalize();  // Forward
			vec4a x = cross(up, z).normalize(); // Right
			vec4a y = cross(z, x);
			x.w = (float)-dot(x, eye);
			y.w = (float)-dot(y, eye);
			z.w = (float)-dot(z, eye);
			return{ x,y,z, CV_0001 };
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::lookAt(const vec4a &eye, const vec4a &to, const vec4a &up) noexcept
		{
			return look(eye, to - eye, up);
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::translate(vector<float, 3> pos) noexcept
		{
			return{
				{ 1,0,0,pos.x },
				{ 0,1,0,pos.y },
				{ 0,0,1,pos.z },
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::identity() noexcept
		{
			return{
				CV_1000,
				CV_0100,
				CV_0010,
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::scale(const vec4a &scale) noexcept
		{
			return{
				scale & (vec4a&)CV_MASK_X,
				scale & (vec4a&)CV_MASK_Y,
				scale & (vec4a&)CV_MASK_Z,
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::scale(float x) noexcept
		{
			vec4a v(x);
			return scale(v);
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::rotateX(float rad) noexcept
		{
			SinCos sc = math::sincos(rad);

			vec4a vSin = vec4a::makes(sc.sin), vCos = vec4a::makes(sc.cos);
			vCos = shuffle<3, 0, 0, 3>(vCos, vSin);

			return{
				CV_1000,
				vCos * CV_MINUS_Z,			// { 0,c,-s,0 }
				vCos.shuffle<0, 2, 1, 3>(),	// { 0,s,c,0 }
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::rotateY(float rad) noexcept
		{
			SinCos sc = math::sincos(rad);

			vec4a vSin = vec4a::makes(sc.sin), vCos = vec4a::makes(sc.cos);
			vCos = shuffle<0, 3, 0, 3>(vCos, vSin);

			return{
				vCos,										// { c,0,s,0 }
				CV_0100,
				vCos.shuffle<2, 1, 0, 3>() * CV_MINUS_X,	// { -s,0,c,0 }
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::rotateZ(float rad) noexcept
		{
			SinCos sc = math::sincos(rad);
			vec4a cs = { sc.cos, sc.sin, 0, 0 };

			return{
				cs * CV_MINUS_Y,							// { c,-s,0,0 }
				cs.shuffle<1, 0, 2, 3>(),					// { s,c,0,0 }
				CV_0010,
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::orthogonal(float width, float height) noexcept
		{
			vec4a v = CV_P2N2_0_P1 / vec4a{ width , height , 1, 1 };
			return{
				shuffle<0,2,1,3>(v, CV_MINUS_W),	// w, 0, 0, -1
				v.shuffle<2,1,2,3>(),				// 0, h, 0, 1
				CV_00_N1_P1,
				CV_0001
			};
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::orthogonal(float left, float top, float right, float bottom, float _near, float _far) noexcept
		{
			vec4a v = {
				left - right,
				bottom - top,
				1.f,
				_near - _far
			};
			/*
			{ (T)(-2 * _w), 0, 0, (T)((right + left)*_w) },
			{ 0, (T)(-2 * _h), 0, (T)((top + bottom)*_h) },
			{ 0, 0, _z, (T)((_far + _near)*_z + (T)0.5) },
			{ 0, 0, 0, 1 }
			*/
			vec4a v2 = {
				left + right,
				top + bottom,
				0.f,
				-_far
			};

			v2 /= v;
			v = CV_N22_0_P1 / v;

			mat4a out;
			out[0] = v2.shuffle<2, 2, 2, 0>().adds(v);
			out[1] = shuffle<2, 1, 2, 1>(v, v2);
			out[2] = unpack_high(v, v2);
			out[3] = CV_0001;
			return out;
		}
		template <>
		ATTR_INLINE const mat4a matrix_method<float, 4, 4, true>::fov(float fovy, float aspect, float _near, float _far) noexcept
		{
			float h = 1.f / math::tan(fovy * 0.5f);
			float w = h / aspect;
			float Q = _far / (_near - _far);

			vec4a v = { w,h,Q, Q*_near };

			return{
				v & (vec4a&)CV_MASK_X,
				v & (vec4a&)CV_MASK_Y,
				v & (vec4a&)CV_MASK_ZW,
				CV_NEG_0010
			};
		}

		template <>
		inline const mat4a matrix_method_common<float, 4, 4, true>::operator *(const matrix_method_common<float, 4, 4, true>& m) const noexcept
		{
			mat4a out;
			const vec4a* a = v;
			const vec4a* bstart = &m.v[0];
			vec4a* r = &out.v[0];
			for (size_t i = 0; i < 4; i++)
			{
				const vec4a* b = bstart;
				vec4a res = a->getXV() * *b;
				b++;
				res += a->getYV() * *b;
				b++;
				res += a->getZV() * *b;
				b++;
				res += a->getWV() * *b;

				*r = res;
				r++;
				a++;
			}
			return out;
		}
		template <>
		inline const vec4a matrix_method_common<float, 4, 4, true>::operator *(const vec4a& o) const noexcept
		{
			vec4a out;
			const vec4a* a = &v[0];
			out.x = dot(o, *a++);
			out.y = dot(o, *a++);
			out.z = dot(o, *a++);
			out.w = dot(o, *a++);
			return out;
		}
		template <>
		inline const mat4a matrix_method_common<float, 4, 4, true>::transpose() const noexcept
		{
			// x.x,x.y,y.x,y.y
			vec4a vTemp1 = shuffle<0, 1, 0, 1>(v[0], v[1]);
			// x.z,x.w,y.z,y.w
			vec4a vTemp3 = shuffle<2, 3, 2, 3>(v[0], v[1]);
			// z.x,z.y,w.x,w.y
			vec4a vTemp2 = shuffle<0, 1, 0, 1>(v[2], v[3]);
			// z.z,z.w,w.z,w.w
			vec4a vTemp4 = shuffle<2, 3, 2, 3>(v[2], v[3]);

			mat4a mResult;
			// x.x,y.x,z.x,w.x
			mResult[0] = shuffle<0, 2, 0, 2>(vTemp1, vTemp2);
			// x.y,y.y,z.y,w.y
			mResult[1] = shuffle<1, 3, 1, 3>(vTemp1, vTemp2);
			// x.z,y.z,z.z,w.z
			mResult[2] = shuffle<0, 2, 0, 2>(vTemp3, vTemp4);
			// x.w,y.w,z.w,w.w
			mResult[3] = shuffle<1, 3, 1, 3>(vTemp3, vTemp4);
			return mResult;
		}


		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE const regionwh<T, cols> intersect(const regionwh<T, cols>& a, const regionwh<T, cols>& b) noexcept
		{
			matrix<T, 2, cols, aligned, matrix_data_type::width_height> out;
			for (size_t i = 0; i < cols; i++)
			{
				T av = a.pos[i], bv = b.pos[i];
				out.pos[i] = (av > bv) ? av : bv;
				av += a.size[i]; bv += b.size[i];
				out.size[i] = ((av < bv) ? av : bv) - out.pos[i];
			}
			return out;
		}
		template <typename T, size_t cols, bool aligned>
		ATTR_INLINE const region<T, cols> intersect(const region<T, cols>& a, const region<T, cols>& b) noexcept
		{
			matrix<T, 2, cols, aligned, matrix_data_type::right_bottom> out;
			for (size_t i = 0; i < cols; i++)
			{
				T av = a.from[i], bv = b.from[i];
				out.from[i] = (av > bv) ? av : bv;
				av = a.to[i]; bv = b.to[i];
				out.to[i] = (av < bv) ? av : bv;
			}
			return out;
		}
	}
}