#pragma once

#include <KR3/main.h>
#include <KR3/math/coord.h>
#include "common.h"
#include "palette.h"

// gcc: cannot forward reference of enum
// typedef enum _D3DFORMAT D3DFORMAT;

namespace kr
{
	namespace image
	{
		template <PixelFormat pf> struct Pixel;

#pragma pack(push, 1)
		namespace px
		{
			template <int _bitcount, int _offset = -1>
			struct PixelBitInfo: public PixelBitInfo<_bitcount, -1>
			{
				using Super = PixelBitInfo<_bitcount, -1>;

				static constexpr int offset = _offset;
				static constexpr int bitcount = _bitcount;
				static constexpr int mask = ((1 << (_offset + _bitcount)) - 1) ^ ((1 << _offset) - 1);

				template <int _tobit>
				static int fromValue(int _rawValue) noexcept
				{
					return Super::template fromComponent<_tobit>((_rawValue& mask) >> _offset);
				}

				template <int _frombit>
				static int toValue(int _fromValue) noexcept
				{
					return Super::template toComponent<_frombit>(_fromValue) << offset;
				}

				static float floatFromValue(int _rawValue) noexcept
				{
					return (float)(_rawValue & mask) / mask;
				}

				static int floatToValue(float _from) noexcept
				{
					return (int)math::lround(_from * mask) & mask;
				}
			};

			template <>
			struct PixelBitInfo<0, -1>
			{
				static constexpr int offset = 0;
				static constexpr int bitcount = 0;
				static constexpr int mask = 0;

				template <int _tobit>
				static int fromComponent(int _comp)
				{
					return (1 << _tobit) - 1;
				}

				template <int _tobit>
				static int fromValue(int _rawValue) noexcept
				{
					return fromComponent<_tobit>(0);
				}

				template <int _frombit>
				static int toValue(int _fromValue) noexcept
				{
					return 0;
				}

				template <int _frombit>
				static int toComponent(int _fromComp) noexcept
				{
					return 0;
				}

				static float floatFromValue(int _rawValue) noexcept
				{
					return 1.f;
				}

				static int floatToValue(float _from) noexcept
				{
					return 0;
				}
			};

			template <int _offset>
			struct PixelBitInfo<0, _offset> : PixelBitInfo<0, -1>
			{
			};

			template <int _bitcount>
			struct PixelBitInfo<_bitcount, -1>
			{
				static constexpr int offset = 0;
				static constexpr int bitcount = _bitcount;
				static constexpr int mask = (1 << (_bitcount)) - 1;

				template <int _tobit>
				static int fromComponent(int _comp) noexcept
				{
					if (_tobit == _bitcount) return _comp;
					constexpr int tovalue = (1 << _tobit) - 1;
					constexpr int fromvalue = (1 << _bitcount) - 1;
					return (_comp * tovalue + fromvalue / 2) / fromvalue;
				}

				template <int _frombit>
				static int toComponent(int _fromComp) noexcept
				{
					if (_frombit == _bitcount) return _fromComp;
					constexpr int tovalue = (1 << _bitcount) - 1;
					constexpr int fromvalue = (1 << _frombit) - 1;
					return (_fromComp * tovalue + fromvalue / 2) / fromvalue;
				}

				static float floatFromComponent(int _comp) noexcept
				{
					return (float)_comp / mask;
				}

				static int floatToComponent(float _from) noexcept
				{
					return (int)math::lround(_from * mask);
				}
			};

			using PixelBit8 = PixelBitInfo<8>;

			template <typename Derived, int pxsz, bool _hasAlpha>
			struct PixelCommon
			{
				static constexpr int size = pxsz;
				static constexpr bool hasAlpha = _hasAlpha;

				byte data[pxsz];

				const Derived* base() const noexcept
				{
					return static_cast<const Derived*>(this);
				}
				Derived* base() noexcept
				{
					return static_cast<Derived*>(this);
				}

				template <int _tobit> int alpha() const
				{
					return (1 << (_tobit + 1)) - 1;
				}
				float alphaf() const
				{
					return 1.f;
				}
				vec4a getf() const noexcept
				{
					vec4a out;
					out.r = base()->redf();
					out.g = base()->greenf();
					out.b = base()->bluef();
					out.a = base()->alphaf();
					return out;
				}
				color get() const noexcept
				{
					color out;
					out.r = (byte)base()->template red<8>();
					out.g = (byte)base()->template green<8>();
					out.b = (byte)base()->template blue<8>();
					out.a = (byte)base()->template alpha<8>();
					return out;
				}

				void set(const vec4a& _color) noexcept
				{
					static_cast<Derived*>(this)->set((Pixel<PixelFormatRGBA32F>&)_color);
				}
				void setAlpha(byte _alpha) noexcept
				{
				}
			};

			template <typename Derived, typename Type, int pxsz, int abit, int rbit, int gbit, int bbit>
			struct BitPixel: PixelCommon<Derived, pxsz, abit != 0>
			{
				using Super = PixelCommon<Derived, pxsz, abit != 0>;

				using Super::data;

				using A = PixelBitInfo<abit, rbit + gbit + bbit>;
				using R = PixelBitInfo<rbit, gbit + bbit>;
				using G = PixelBitInfo<gbit, bbit>;
				using B = PixelBitInfo<bbit, 0>;

				Type & rawValue() noexcept
				{
					return *(Type*)data;
				}
				const Type & rawValue() const noexcept
				{
					return *(Type*)data;
				}
				template <int _tobit>
				int alpha() const noexcept
				{
					return A::template fromValue<_tobit>(rawValue());
				}
				template <int _tobit>
				int red() const noexcept
				{
					return R::template fromValue<_tobit>(rawValue());
				}
				template <int _tobit>
				int green() const noexcept
				{
					return G::template fromValue<_tobit>(rawValue());
				}
				template <int _tobit>
				int blue() const noexcept
				{
					return B::template fromValue<_tobit>(rawValue());
				}
				float alphaf() const noexcept
				{
					return A::floatFromValue(rawValue());
				}
				float redf() const noexcept
				{
					return R::floatFromValue(rawValue());
				}
				float greenf() const noexcept
				{
					return G::floatFromValue(rawValue());
				}
				float bluef() const noexcept
				{
					return B::floatFromValue(rawValue());
				}

				void setAlpha(byte _alpha) noexcept
				{
					rawValue() = (Type)((*(Type*)data & (R::mask | G::mask | B::mask)) | A::template toValue<8>(_alpha));
				}
				void set(const vec4a & _color) noexcept
				{
					int res = A::floatToValue(_color.a);
					res |= R::floatToValue(_color.r);
					res |= G::floatToValue(_color.g);
					res |= B::floatToValue(_color.b);
					rawValue() = (Type)res;
				}
				template <typename _Parent, int _pxsize, bool _hasalpha>
				void set(const px::PixelCommon<_Parent, _pxsize, _hasalpha> &_src) noexcept
				{
					int res = _src.base()->template alpha<abit>() << A::offset;
					res |= _src.base()->template red<rbit>() << R::offset;
					res |= _src.base()->template green<gbit>() << G::offset;
					res |= _src.base()->template blue<bbit>() << B::offset;
					rawValue() = (Type)res;
				}
			};

			struct NullColorByteData
			{
				NullColorByteData() noexcept = default;
				NullColorByteData(byte) noexcept
				{
				}

				inline NullColorByteData& operator =(byte data) noexcept
				{
					return *this;
				}
				operator byte() const noexcept
				{
					return 0xff;
				}
			};

			template <int offset>
			struct ColorByteData
			{
				using type = byte;
			};

			template <>
			struct ColorByteData<-1>
			{
				using type = NullColorByteData;
			};

			template <int offset>
			using color_byte = typename ColorByteData<offset>::type;

			template <typename Derived, int pxsz, int a, int r, int g, int b>
			struct BytePixel: PixelCommon<Derived, pxsz, a != 0>
			{
				using Super = PixelCommon<Derived, pxsz, a != 0>;

				using Super::base;
				using Super::data;
				using AByte = color_byte<a>;
				using RByte = color_byte<r>;
				using GByte = color_byte<g>;
				using BByte = color_byte<b>;
				AByte & rawAlpha() noexcept
				{
					return (AByte&)data[a];
				}
				const AByte & rawAlpha() const noexcept
				{
					return (AByte&)data[a];
				}
				RByte & rawRed() noexcept
				{
					return (RByte&)data[r];
				}
				const RByte & rawRed() const noexcept
				{
					return (RByte&)data[r];
				}
				GByte & rawGreen() noexcept
				{
					return (GByte&)data[g];
				}
				const GByte & rawGreen() const noexcept
				{
					return (GByte&)data[g];
				}
				BByte & rawBlue() noexcept
				{
					return (BByte&)data[b];
				}
				const BByte & rawBlue() const noexcept
				{
					return (BByte&)data[b];
				}

				template <int _tobit>
				int alpha() const noexcept
				{
					return PixelBit8::fromComponent<_tobit>(rawAlpha());
				}
				template <int _tobit>
				int red() const noexcept
				{
					return PixelBit8::fromComponent<_tobit>(rawRed());
				}
				template <int _tobit>
				int green() const noexcept
				{
					return PixelBit8::fromComponent<_tobit>(rawGreen());
				}
				template <int _tobit>
				int blue() const noexcept
				{
					return PixelBit8::fromComponent<_tobit>(rawBlue());
				}
				float alphaf() const noexcept
				{
					return PixelBit8::floatFromComponent(rawAlpha());
				}
				float redf() const noexcept
				{
					return PixelBit8::floatFromComponent(rawRed());
				}
				float greenf() const noexcept
				{
					return PixelBit8::floatFromComponent(rawGreen());
				}
				float bluef() const noexcept
				{
					return PixelBit8::floatFromComponent(rawBlue());
				}
				color get() const noexcept
				{
					color out;
					out.r = rawRed();
					out.g = rawGreen();
					out.b = rawBlue();
					out.a = rawAlpha();
					return out;
				}

				void setAlpha(byte _alpha) noexcept
				{
					rawAlpha() = _alpha;
				}
				using Super::set;
				template <typename _Parent, int _pxsize, bool _hasalpha>
				void set(const px::PixelCommon<_Parent, _pxsize, _hasalpha> &_src) noexcept
				{
					if (is_same<Derived, _Parent>::value)
					{
						*base() = *(Derived*)_src.base();
						return;
					}
					base()->rawAlpha() = _src.base()->template alpha<8>();
					base()->rawRed() = _src.base()->template red<8>();
					base()->rawGreen() = _src.base()->template green<8>();
					base()->rawBlue() = _src.base()->template blue<8>();
				}
			};
		}

		template <> struct Pixel<PixelFormatIndex>
			: px::PixelCommon<Pixel<PixelFormatIndex>, 1, true>
		{
			using Super = PixelCommon<Pixel<PixelFormatIndex>, 1, true>;

			vec4a getf() noexcept
			{
				color c = Palette::defaultPalette->get(*data);
				vec4a out;
				out.r = c.r / 255.f;
				out.g = c.g / 255.f;
				out.b = c.b / 255.f;
				out.a = c.a / 255.f;
				return out;
			}
			color get() const noexcept
			{
				return Palette::defaultPalette->get(*data);
			}

			template <int _tobit>
			int alpha() const noexcept
			{
				return px::PixelBit8::fromComponent<_tobit>(Palette::defaultPalette->get(*data).a);
			}
			template <int _tobit>
			int red() const noexcept
			{
				return px::PixelBit8::fromComponent<_tobit>(Palette::defaultPalette->get(*data).r);
			}
			template <int _tobit>
			int green() const noexcept
			{
				return px::PixelBit8::fromComponent<_tobit>(Palette::defaultPalette->get(*data).g);
			}
			template <int _tobit>
			int blue() const noexcept
			{
				return px::PixelBit8::fromComponent<_tobit>(Palette::defaultPalette->get(*data).b);
			}

			using Super::set;
			template <typename _Parent, int _pxsize, bool _hasalpha>
			void set(const px::PixelCommon<_Parent, _pxsize, _hasalpha> &_src) noexcept
			{
				color c;
				c.r = (byte)_src.base()->template red<8>();
				c.g = (byte)_src.base()->template green<8>();
				c.b = (byte)_src.base()->template blue<8>();
				c.a = (byte)_src.base()->template alpha<8>();
				Palette::defaultPalette->getNearstColor(c);
			}
		};
		template <> struct Pixel<PixelFormatA8>
			: px::BytePixel<Pixel<PixelFormatA8>, 1, 0, -1, -1, -1>
		{
		};
		template <> struct Pixel<PixelFormatR5G6B5>
			: px::BitPixel<Pixel<PixelFormatR5G6B5>, word, 2, 0, 5, 6, 5>
		{
		};
		template <> struct Pixel<PixelFormatA1RGB5> 
			:px::BitPixel<Pixel<PixelFormatA1RGB5>, word, 2, 1, 5, 5, 5>
		{
		};
		template <> struct Pixel<PixelFormatX1RGB5>
			:px::BitPixel<Pixel<PixelFormatX1RGB5>, word, 2, 0, 5, 5, 5>
		{
			using Super = px::BitPixel<Pixel<PixelFormatX1RGB5>, word, 2, 0, 5, 5, 5>;
			using Super::set;

			void set(const Pixel<PixelFormatA1RGB5> &_src) noexcept
			{
				*this = (Pixel&)_src;
			}
		};
		template <> struct Pixel<PixelFormatARGB4>
			:px::BitPixel<Pixel<PixelFormatARGB4>, word, 2, 4, 4, 4, 4>
		{
		};
		template <> struct Pixel<PixelFormatRGB8>
			:px::BytePixel<Pixel<PixelFormatRGB8>, 3, -1, 2, 1, 0>
		{

		};
		template <> struct Pixel<PixelFormatARGB8>
			:px::BytePixel<Pixel<PixelFormatARGB8>, 4, 3, 2, 1, 0>
		{
			color get() const noexcept
			{
				return *(color*)data;
			}
		};
		template <> struct Pixel<PixelFormatXRGB8>
			:px::BytePixel<Pixel<PixelFormatXRGB8>, 4, -1, 2, 1, 0>
		{
			using Super = px::BytePixel<Pixel<PixelFormatXRGB8>, 4, -1, 2, 1, 0>;
			using Super::set;

			void set(const Pixel<PixelFormatARGB8> &_src) noexcept
			{
				*this = (Pixel&)_src;
			}
			color get() const noexcept
			{
				color out = *(color*)data;
				out.a = 255;
				return out;
			}
		};
		template <> struct Pixel<PixelFormatBGR8>
			:px::BytePixel<Pixel<PixelFormatBGR8>, 3, -1, 0, 1, 2>
		{
		};
		template <> struct Pixel<PixelFormatABGR8>
			:px::BytePixel<Pixel<PixelFormatABGR8>, 4, 3, 0, 1, 2>
		{
		};
		template <> struct Pixel<PixelFormatXBGR8>
			:px::BytePixel<Pixel<PixelFormatXBGR8>, 4, -1, 0, 1, 2>
		{
			using Super = px::BytePixel<Pixel<PixelFormatXBGR8>, 4, -1, 0, 1, 2>;
			using Super::set;

			void set(const Pixel<PixelFormatABGR8> &_src) noexcept
			{
				*this = (Pixel&)_src;
			}
		};
		template <> struct Pixel<PixelFormatRGBA32F> :
			px::PixelCommon<Pixel<PixelFormatRGBA32F>, 16, true>
		{	
			using Super = px::PixelCommon<Pixel<PixelFormatRGBA32F>, 16, true>;

			template <int _tobit> int alpha() const noexcept
			{
				return math::lround(alphaf() * ((1 << _tobit) - 1));
			}
			template <int _tobit> int red() const noexcept
			{
				return math::lround(((float*)data)[0] * ((1 << _tobit) - 1));
			}
			template <int _tobit> int green() const noexcept
			{
				return math::lround(((float*)data)[1] * ((1 << _tobit) - 1));
			}
			template <int _tobit> int blue() const noexcept
			{
				return math::lround(((float*)data)[2] * ((1 << _tobit) - 1));
			}
			
			float alphaf() const noexcept
			{
				return ((float*)data)[3];
			}
			float redf() const noexcept
			{
				return ((float*)data)[0];
			}
			float greenf() const noexcept
			{
				return ((float*)data)[1];
			}
			float bluef() const noexcept
			{
				return ((float*)data)[2];
			}

			void setAlpha(byte _alpha) noexcept
			{
				((float*)data)[3] = _alpha / 255.f;
			}

			using Super::set;
			template <typename _Parent, int _pxsize, bool _hasalpha>
			void set(const px::PixelCommon<_Parent, _pxsize, _hasalpha> &_src) noexcept
			{
				((float*)data)[0] = _src.base()->redf();
				((float*)data)[1] = _src.base()->greenf();
				((float*)data)[2] = _src.base()->bluef();
				((float*)data)[3] = _src.base()->alphaf();
			}
		};
#pragma pack(pop)
		
		struct FormatInfo
		{
			int size;
			bool hasAlpha;
#ifdef WIN32
			int d3d9Format;
			int dxgiFormat;
#endif

			void(*reformat[(size_t)PixelFormatCount])(ImageData * _dest, const ImageData * _src);
			dword(*makeColor)(color);
			void(*copyStretch)(ImageData * _dest, const ImageData * _src);
			void(*imageProcessing)(ImageData * _dest, const ImageData * _src, const float * weightTable, int _weightX, int _weightY, int _weightWidth, int _weightHeight);
			void(*setAlpha)(ImageData * _dest, const ImageData * _alpha);
			void(*fill)(ImageData * _dest, color _color);
			void(*maskedRecolor)(ImageData * _dest, color _color, const ImageData * _mask);
			void(*recolor)(ImageData * _dest, color _color);
		};

		const FormatInfo * getFormatInfo(PixelFormat pf) noexcept;
#ifdef WIN32
		PixelFormat getFromD3D9Format(int format) noexcept;
		PixelFormat getFromDXGIFormat(int format) noexcept;
#endif
		void reformat(ImageData * dest, const ImageData * src) noexcept;
		const ImageData * reformat(ImageData * buffer, const ImageData * src, PixelFormat format) noexcept;
		const ImageData * reformat(ImageData * buffer, const ImageData * src, PixelFormat format, int pitchBytes) noexcept;
	}
}