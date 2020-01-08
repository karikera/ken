#pragma once

#include "../meta/align.h"
#include "../meta/number.h"
#include "autovar.h"

#include <memory.h>

namespace kr
{
	template <size_t size>
	class IntableValue
	{
	public:
		template <typename T>
		inline IntableValue(T v) noexcept;
		template <typename T>
		inline operator T&() noexcept;
		template <typename T>
		inline operator const T&() const noexcept;

	private:
		int_sz_t<size> m_value;
	};
	template <size_t size>
	template <typename T>
	inline IntableValue<size>::IntableValue(T v) noexcept
	{
		m_value = (int_sz_t<size>)v;
	}
	template <size_t size>
	template <typename T>
	inline IntableValue<size>::operator T&() noexcept
	{
		static_assert(sizeof(T) == size, "size not match");
		return (T&)m_value;
	}
	template <size_t size>
	template <typename T>
	inline IntableValue<size>::operator const T&() const noexcept
	{
		static_assert(sizeof(T) == size, "size not match");
		return (T&)m_value;
	}

	#define _VOID_				ATTR_NOALIAS static void
	#define _RETURN_			ATTR_NO_DISCARD ATTR_NOALIAS static
	#define _NULLABLE_			ATTR_NULLABLE _RETURN_
	#define _NOTNULL_			ATTR_NONULL _RETURN_
	namespace _pri_
	{
		template <typename T>
		struct _tconst
		{
			using type = const T;
		};
	}

	template <size_t BASE> 
	struct memt
	{
		using type = uint_sz_t<BASE>;
		using stype = int_sz_t<BASE>;
		using atype = IntableValue<BASE>;
		template <typename T>
		using tconst = typename _pri_::_tconst<T>::type;
	
		_VOID_ zero(ptr _dst, size_t _len) noexcept;
		_VOID_ set(ptr _dst, atype chr, size_t _len) noexcept;
		_VOID_ copy(ptr _dst, cptr _src, size_t _len) noexcept;
		_VOID_ move(ptr _dst, cptr _src, size_t _len) noexcept;
		_VOID_ xor_copy(ptr _dst, cptr _src, size_t _len, dword _key) noexcept;
		_RETURN_ bool equals(cptr _dst, cptr _src, size_t _len) noexcept;
		_RETURN_ bool equals_i(cptr _dst, cptr _src, size_t _len) noexcept;
		ATTR_NO_DISCARD _RETURN_ int compare(cptr _dst, cptr _src, size_t _len) noexcept;
		template <typename T>
		_RETURN_ T* compare_p(T* _dst, tconst<T>* _src, size_t _len) noexcept;
		
		_NOTNULL_ ptr alloc(cptr _src, size_t _len) noexcept;
		template <typename T>
		_RETURN_ size_t strlen(T* _dst) noexcept;
		template <typename T, typename READCB>
		_NOTNULL_ auto find_callback(const READCB &read, T* _tar, size_t _tarlen)->decltype(read());
		template <typename T> 
		_NOTNULL_ T* find(T* _dst, atype _tar) noexcept;
		template <typename T>
		_NULLABLE_ T* find(T* _dst, atype _tar, size_t _dstsize) noexcept;
		template <typename T>
		_NOTNULL_ T* find(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept;
		template <typename T>
		_NOTNULL_ T* find_e(T* _src, atype _tar, size_t _srclen) noexcept;
		template <typename T>
		_NULLABLE_ T* find(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_r(T* _src, atype _tar, size_t _len) noexcept;
		template <typename T>
		_NULLABLE_ T* find_r(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_ry(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_NOTNULL_ T* find_re(T* _src, atype _tar, size_t _len) noexcept;
		template <typename T>
		_NOTNULL_ T* find_rye(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_y(T* _src, tconst<T>* _tar,size_t _tarlen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_y(T* _src,tconst<T>* _tar,size_t _srclen,size_t _tarlen) noexcept;
		template <typename T>
		_NOTNULL_ T* find_ye(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_NOTNULL_ T* find_n(T* _src, atype _skp) noexcept;
		template <typename T>
		_NULLABLE_ T* find_n(T* _src, atype _skp,size_t _srclen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_nr(T* _src, atype _skp, size_t _srclen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_ne(T* _src, atype _skp, size_t _srclen) noexcept;
		template <typename T>
		_NOTNULL_ T* find_ny(T* _src, tconst<T>* _skp,size_t _skplen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_ny(T* _src, tconst<T>* _skp, size_t _srclen, size_t _skplen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_nre(T* _src, atype _skp, size_t _srclen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_nry(T* _src, tconst<T>* _skp, size_t _srclen, size_t _skplen) noexcept;
		template <typename T>
		_NULLABLE_ T* find_nye(T* _src, tconst<T>* _skp, size_t _srclen, size_t _skplen) noexcept;
		template <typename T>
		_RETURN_ size_t pos(T* _src, atype _tar) noexcept;
		template <typename T>
		_RETURN_ size_t pos(T* _src, atype _tar, size_t _srclen) noexcept;
		template <typename T>
		_RETURN_ size_t pos(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept;
		template <typename T>
		_RETURN_ size_t pos(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_r(T* _src, atype _tar, size_t _srclen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_ry(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_y(T* _src, tconst<T>* _tar, size_t _tarlen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_y(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_n(T* _src, atype _tar) noexcept;
		template <typename T>
		_RETURN_ size_t pos_n(T* _src, atype _tar, size_t _srclen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_nr(T* _src, atype _tar, size_t _srclen) noexcept;
		template <typename T>
		_RETURN_ size_t pos_nry(T* _src, tconst<T>* _tar, size_t _srclen, size_t _tarlen) noexcept;

		_RETURN_ bool contains(cptr _src, atype _tar, size_t _srclen) noexcept;
		_RETURN_ bool filled(cptr _src, atype _tar, size_t _srclen) noexcept;
		_RETURN_ size_t count(cptr _src, atype _tar, size_t _srclen) noexcept;
		_RETURN_ size_t count_y(cptr _src, cptr _tar, size_t _srclen, size_t _tarlen) noexcept;
		_VOID_ change(ptr _src, atype _tar, atype _to, size_t _srclen) noexcept;
		_VOID_ tolower(ptr _src, size_t _srclen) noexcept;
		_VOID_ toupper(ptr _src, size_t _srclen) noexcept;
	
		_RETURN_ size_t hash(cptr _src, size_t _len) noexcept; // adler2

		_RETURN_ bool numberonly(cptr _src, size_t _len) noexcept;

		template <typename T>
		_RETURN_ T toint(cptr _src, size_t _len, uint _radix) noexcept;
		template <typename T>
		_RETURN_ T toint_limit(cptr _src, size_t _len, uint _radix) throws(OutOfRangeException);
		template <typename T>
		_RETURN_ T tosint(cptr _src, size_t _len, uint _radix) noexcept;
		template <typename T>
		_RETURN_ T tosint_limit(cptr _src, size_t _len, uint _radix) throws(OutOfRangeException);
		template <typename T>
		_RETURN_ T touint(cptr _src, size_t _len, uint _radix) noexcept;
		template <typename T>
		_RETURN_ T touint_limit(cptr _src, size_t _len, uint _radix) throws(OutOfRangeException);
		template <typename T>
		_VOID_ fromint(ptr _dst, size_t _cipher, T number, uint _radix) noexcept;
		template <typename T>
		_RETURN_ type inttochr(T number) noexcept
		{
			return (type)((number >= 10)*('A' - '0' - 10) + number + '0');
		}
		_RETURN_ bool equals_i(type a, type b) noexcept
		{
			if (a == b)
				return true;
			if (a <= (type)'Z')
			{
				if (a < (type)'A')
					return false;
				else
				{
					a += (type)'a' - (type)'A';
					return a == b;
				}
			}
			else
			{
				if (a < (type)'a')
					return false;
				if (a <= (type)'z')
				{
					a = a + ((stype)'A' - (stype)'a');
					return a == b;
				}
				return false;
			}
		}

	};

	using mem = memt<1>;
	using mem16 = memt<2>;
	using mem32 = memt<4>;
	using mem64 = memt<8>;
	using memptr = memt<sizeof(void*)>;

	namespace _pri_
	{
		template <size_t BASE, bool b> 
		struct AutoFunc;
		template <size_t BASE>
		struct AutoFunc<BASE, false>: public memt<BASE>
		{
			using Super = memt<BASE>;
			template <typename T>
			static T toint(cptr _src, size_t _len, uint _radix)
			{
				return Super::template touint<T>(_src, _len, _radix);
			}
			template <typename T>
			static T toint_limit(cptr _src, size_t _len, uint _radix)
			{
				return Super::template touint_limit<T>(_src, _len, _radix);
			}
		};
		template <size_t BASE>
		struct AutoFunc<BASE, true> : public memt<BASE>
		{
			using Super = memt<BASE>;
			template <typename T>
			static T toint(cptr _src, size_t _len, uint _radix)
			{
				return Super::template tosint<T>(_src, _len, _radix);
			}
			template <typename T>
			static T toint_limit(cptr _src, size_t _len, uint _radix)
			{
				return Super::template tosint_limit<T>(_src, _len, _radix);
			}
		};

		template <size_t SIZE> struct SIZE_MEM
		{
			static constexpr size_t align = meta::align_of<SIZE>::value;
			static constexpr size_t blocksize = SIZE / align;
			using almem = memt<align>;

			_VOID_ fill(void* dest, const void* src, size_t size) noexcept;
			_VOID_ copy(void* dest, const void* src, size_t size) noexcept;
			_VOID_ move(void* dest, const void* src, size_t size) noexcept;
			_VOID_ zero(void* dest, size_t size) noexcept;
		};;

		template <size_t SIZE> struct SIZE_MEM_SINGLE
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<1>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<2>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<3>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<4>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<5>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<6>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<8>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<9>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<10>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		template <> struct SIZE_MEM_SINGLE<12>
		{
			_VOID_ copy(void* dest, const void* src) noexcept;
			_VOID_ zero(void* dest) noexcept;
		};;
		
		template <bool OP, bool is_void> struct ARRCOPY;
		template <> struct ARRCOPY<true, true>
		{
			template <typename T> _VOID_ subs_copy(void* dest, const void* src, size_t size) noexcept;
			template <typename T> _VOID_ subs_move(void* dest, void* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor(void* dest, void* dest_end) noexcept;
			template <typename T> _VOID_ dtor(void* dest, void* dest_end) noexcept;
			template <typename T> _VOID_ ctor_copy(void* dest, const void* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move(void* dest, void* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_r(void* dest, void* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_d(void* dest, void* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_rd(void* dest, void* src, size_t size) noexcept;
		};
		template <> struct ARRCOPY<true, false>
		{
			template <typename T> _VOID_ subs_fill(T* dest, const T& src, size_t size) noexcept;
			template <typename T> _VOID_ subs_copy(T* dest, const T* src, size_t size) noexcept;
			template <typename T> _VOID_ subs_move(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor(T* dest, T* dest_end) noexcept;
			template <typename T> _VOID_ dtor(T* dest, T* dest_end) noexcept;
			template <typename T> _VOID_ ctor_fill(T* dest, const T& src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_copy(T* dest, const T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_r(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_d(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_rd(T* dest, T* src, size_t size) noexcept;
		};

		template <> struct ARRCOPY<false, false>
		{
			template <typename T> _VOID_ subs_fill(T* dest, const T& src, size_t size) noexcept;
			template <typename T> _VOID_ subs_copy(T* dest, const T* src, size_t size) noexcept;
			template <typename T> _VOID_ subs_move(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor(T* dest, T* dest_end) noexcept;
			template <typename T> _VOID_ dtor(T* dest, T* dest_end) noexcept;
			template <typename T> _VOID_ ctor_fill(T* dest, const T& src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_copy(T* dest, const T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_r(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_d(T* dest, T* src, size_t size) noexcept;
			template <typename T> _VOID_ ctor_move_rd(T* dest, T* src, size_t size) noexcept;
		};
	}

	struct mema
	{
	#pragma warning(push)
	#pragma warning(disable:6326)
		template <typename T> _NOTNULL_ T* find(T* _src, add_const_t<T> &_tar) noexcept;
		template <typename T> _NULLABLE_ T* find(T* _src, add_const_t<T> &_tar, size_t _len) noexcept;
		template <typename T> _RETURN_ size_t pos(const T* _src, const T &_tar) noexcept;
		template <typename T> _RETURN_ size_t pos(const T* _src, const T &_tar, size_t _len) noexcept;
		template <typename T> _VOID_ zero(T& _dst) noexcept;
		template <typename T> _VOID_ zero(T* _dst, size_t _len) noexcept;
		template <typename T> _VOID_ copy(T& _dst, const T &_src) noexcept;
		template <typename T> _VOID_ copy(T* _dst, const T* _src, size_t _len) noexcept;
		template <typename T> _VOID_ copy_r(T* _dst, const T* _src, size_t _len) noexcept;
		template <typename T, size_t size> _VOID_ copy(T(&dest)[size], const T(&src)[size]) noexcept;

		template <typename T> _NOTNULL_ T* alloc(const T &_src) noexcept;
		template <typename T> _NOTNULL_ T* alloc(const T* _src, size_t _len) noexcept;

		template <typename T> _VOID_ ctor(T* dest, T* dest_end) noexcept;
		template <typename T> _VOID_ dtor(T* dest, T* dest_end) noexcept;
		template <typename T> _VOID_ subs_fill(T* dest, const T& src, size_t size) noexcept;
		template <typename T> _VOID_ subs_copy(T* dest, const T* src, size_t size) noexcept;
		template <typename T> _VOID_ subs_move(T* dest, T* src, size_t size) noexcept;
		template <typename T> _VOID_ ctor_fill(T* dest, const T& src, size_t size) noexcept;
		template <typename T> _VOID_ ctor_copy(T* dest, const T* src, size_t size) noexcept;
		template <typename T> _VOID_ ctor_move(T* dest, T* src, size_t size) noexcept;
		template <typename T> _VOID_ ctor_move_r(T* dest, T* src, size_t size) noexcept;
		template <typename T> _VOID_ ctor_move_d(T* dest, T* src, size_t size) noexcept;
		template <typename T> _VOID_ ctor_move_rd(T* dest, T* src, size_t size) noexcept;
		template <typename T, size_t size> _VOID_ subs_copy(T(&dest)[size], const T(&src)[size]) noexcept;
		template <typename T, size_t size> _VOID_ subs_move(T(&dest)[size], const T(&src)[size]) noexcept;
	#pragma warning(pop)
	};

	struct memtri
	{
		_NULLABLE_ ptr find(cptr _src,  cptr _tar) noexcept;
		_NULLABLE_ ptr find(cptr _src,  cptr _tar, size_t _len) noexcept;
		_RETURN_ size_t pos(cptr _src,  cptr _tar) noexcept;
		_RETURN_ size_t pos(cptr _src,  cptr _tar, size_t _len) noexcept;
	};

	template <size_t sz>
	struct memc
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return mem::equals(a, b, sz);
		}
	};

	template <> 
	struct memc<1>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(byte*)a == *(byte*)b;
		}
	};

	template <>
	struct memc<2>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(word*)a == *(word*)b;
		}
	};

	template <>
	struct memc<3>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(word*)a == *(word*)b && ((byte*)a)[2] == ((byte*)b)[2];
		}
	};

	template <>
	struct memc<4>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(dword*)a == *(dword*)b;
		}
	};

	template <>
	struct memc<5>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(dword*)a == *(dword*)b && ((byte*)a)[4] == ((byte*)b)[4];
		}
	};

	template <>
	struct memc<6>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(dword*)a == *(dword*)b && ((word*)a)[4] == ((word*)b)[4];
		}
	};

	template <>
	struct memc<7>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(dword*)a == *(dword*)b && ((word*)a)[4] == ((word*)b)[4] && ((byte*)a)[6] == ((byte*)b)[6];
		}
	};

	template <>
	struct memc<8>
	{
		static bool equals(const void * a, const void * b) noexcept
		{
			return *(qword*)a == *(qword*)b;
		}
	};

	#undef _VOID_
	#undef _RETURN_
	#undef _NULLABLE_

}

#include "memlib.inl"

extern template struct kr::memt<1>;
extern template struct kr::memt<2>;
extern template struct kr::memt<4>;
