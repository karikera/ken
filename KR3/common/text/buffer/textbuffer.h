#pragma once

#include "../container.h"
#include "membuffer.h"
#include <string>

namespace kr
{
	namespace buffer
	{
		namespace _pri_
		{
			template <typename Derived, typename Parent>
			class MemBuffer_c_str :public Parent
			{
				using Super = Parent;
			public:
				INHERIT_COMPONENT();

				const Component* c_str() const
				{
					return static_cast<const Derived*>(this)->$begin();
				}
			};

			template <class Derived, class Next>
			class TextCommon: public meta::if_t<Next::szable, _pri_::MemBuffer_c_str<Derived, Next>, Next>
			{
				CLASS_HEADER(meta::if_t<Next::szable, _pri_::MemBuffer_c_str<Derived, Next>, Next>);
			public:
				INHERIT_COMPONENT();

				using Super::Super;
				using Super::data;
				using Super::begin;
				using Super::end;
				using Super::size;
				using Super::find_ny;
				using Super::find_nry;
				using Super::endIndex;

				static const Component(&WHITE_SPACE)[5];

				friend std::basic_ostream<Component>& operator <<(std::basic_ostream<Component>& os, const TextCommon& method)
				{
					os.write(method.begin(), method.size());
					return os;
				}

				uint64_t to_uint64_x(uint _radix, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return memm::template toint<uint64_t>(begin(), _len, _radix);
				}
				int64_t to_int64_x(uint _radix, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return memm::template toint<int64_t>(begin(), _len, _radix);
				}
				uint32_t to_uint_x(uint _radix, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return memm::template toint<uint32_t>(begin(), _len, _radix);
				}
				int32_t to_int_x(uint _radix, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return memm::template toint<int32_t>(begin(), _len, _radix);
				}
				uintptr_t to_uintp_x(uint _radix, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return memm::template toint<uintptr_t>(begin(), _len, _radix);
				}
				intptr_t to_intp_x(uint _radix, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return memm::template toint<intptr_t>(begin(), _len, _radix);
				}
				float to_ufloat_x(size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					const Component* beg = begin();
					const Component* finded = memm::find(beg, '.', _len);
					if (finded == nullptr) return (float)to_uint_x(10, _len);
					size_t len2 = finded - beg;
					_len = _len - len2 - 1;
					float res = (float)to_uint_x(10, len2);
					res += (float)(memm::template toint<dword>(finded + 1, _len, 10)) / math::pow((size_t)10, _len);
					return res;
				}
				float to_float_x(size_t _len) const noexcept
				{
					if (!_len) return 0;
					if (*begin() == (Component)'-')
						return -((*this + 1).to_ufloat_x(_len - 1));
					else return to_ufloat_x(_len);
				}
				bool numberonly_x(size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					return (memm::numberonly(begin(), _len));
				}

				uint64_t to_uint64_limit_x(uint _radix, size_t _len) const throws(OutOfRangeException)
				{
					KR_DEFINE_MMEM();
					return memm::template toint_limit<uint64_t>(data(), _len, _radix);
				}
				int64_t to_int64_limit_x(uint _radix, size_t _len) const throws(OutOfRangeException)
				{
					KR_DEFINE_MMEM();
					return memm::template toint_limit<int64_t>(data(), _len, _radix);
				}
				uint32_t to_uint_limit_x(uint _radix, size_t _len) const throws(OutOfRangeException)
				{
					KR_DEFINE_MMEM();
					return memm::template toint_limit<uint32_t>(data(), _len, _radix);
				}
				int32_t to_int_limit_x(uint _radix, size_t _len) const throws(OutOfRangeException)
				{
					KR_DEFINE_MMEM();
					return memm::template toint_limit<int32_t>(data(), _len, _radix);
				}
				uintptr_t to_uintp_limit_x(uint _radix, size_t _len) const throws(OutOfRangeException)
				{
					KR_DEFINE_MMEM();
					return memm::template toint_limit<uintptr_t>(data(), _len, _radix);
				}
				intptr_t to_intp_limit_x(uint _radix, size_t _len) const throws(OutOfRangeException)
				{
					KR_DEFINE_MMEM();
					return memm::template toint_limit<intptr_t>(data(), _len, _radix);
				}

				uint64_t to_uint64(uint _radix = 10) const noexcept
				{
					return to_uint64_x(_radix, size());
				}
				int64_t to_int64(uint _radix = 10) const noexcept
				{
					return to_int64_x(_radix, size());
				}
				uint32_t to_uint(uint _radix = 10) const noexcept
				{
					return to_uint_x(_radix, size());
				}
				int32_t to_int(uint _radix = 10) const noexcept
				{
					return to_int_x(_radix, size());
				}
				uintptr_t to_uintp(uint _radix = 10) const noexcept
				{
					return to_uintp_x(_radix, size());
				}
				intptr_t to_intp(uint _radix = 10) const noexcept
				{
					return to_intp_x(_radix, size());
				}
				float to_ufloat() const noexcept
				{
					return to_ufloat_x(size());
				}
				float to_float() const noexcept
				{
					return to_float_x(size());
				}

				uint64_t to_uint64_limit(uint _radix = 10) const
				{
					return to_uint64_limit_x(_radix, size());
				}
				int64_t to_int64_limit(uint _radix = 10) const
				{
					return to_uint64_limit_x(_radix, size());
				}
				uint32_t to_uint_limit(uint _radix = 10) const
				{
					return to_uint_limit_x(_radix, size());
				}
				int32_t to_int_limit(uint _radix = 10) const
				{
					return to_int_limit_x(_radix, size());
				}
				uintptr_t to_uintp_limit(uint _radix = 10) const
				{
					return to_uintp_limit_x(_radix, size());
				}
				intptr_t to_intp_limit(uint _radix = 10) const
				{
					return to_intp_limit_x(_radix, size());
				}

				uint64_t to_qword_l(size_t _len, uint _radix) const noexcept
				{
					return to_uint64_x(_radix, mint(_len, size()));
				}
				int64_t to_llong_l(size_t _len, uint _radix) const noexcept
				{
					return to_int64_x(_radix, mint(_len, size()));
				}
				uint32_t to_uint_l(size_t _len, uint _radix) const noexcept
				{
					return to_uint_x(_radix, mint(_len, size()));
				}
				int32_t to_int_l(size_t _len, uint _radix) const noexcept
				{
					return to_int_x(_radix, mint(_len, size()));
				}
				uintptr_t to_uintp_l(size_t _len, uint _radix) const noexcept
				{
					return to_uintp_x(_radix, mint(_len, size()));
				}
				intptr_t to_intp_l(size_t _len, uint _radix) const noexcept
				{
					return to_intp_x(_radix, mint(_len, size()));
				}
				float to_ufloat_l(size_t _len) const noexcept
				{
					return to_ufloat_x(mint(_len, size()));
				}
				float to_float_l(size_t _len) const noexcept
				{
					return to_float_x(mint(_len, size()));
				}
				bool numberonly() const noexcept
				{
					return numberonly_x(size());
				}

				intptr_t compare(Ref _str) const noexcept
				{
					KR_DEFINE_MMEM();
					size_t len1 = size();
					size_t len2 = _str.size();

					int order = memm::compare(begin(), _str.begin(), mint(len1, len2));
					if (order == 0) return len1 - len2;
					return order;
				}
				bool equals_i(Ref _v) const noexcept
				{
					return equals_ix(_v, size());
				}
				bool equals_ix(Ref _v, size_t _len) const noexcept
				{
					KR_DEFINE_MMEM();
					size_t len2 = _v.size();
					if (_len != len2) return false;
					else return memm::equals_i(begin(), _v.begin(), _len);
				}

				Ref trim() const noexcept
				{
					const Component* beg = find_ny(WHITE_SPACE);
					if (beg == nullptr)
						return endIndex();
					const Component* end = find_nry(WHITE_SPACE);
					return Ref(beg, end + 1);
				}

				constexpr TextCommon() = default;
				constexpr TextCommon(const TextCommon&) = default;
				constexpr TextCommon(TextCommon&&) = default;
				explicit TextCommon(const Component* str) noexcept
					:Super(str, memt<sizeof(InternalComponent)>::pos(str, 0))
				{
				}

				explicit operator std::basic_string<InternalComponent>() noexcept
				{
					return std::basic_string<InternalComponent>(begin(), size());
				}
			};
		}
		template <class Derived, class Info>
		class Text;

		template <class Derived, typename Component, template <typename, typename> class Method, size_t szable, size_t readonly, typename Parent>
		class Text<Derived, BufferInfo<Component, Method, szable, readonly, Parent> > 
			:public _pri_::TextCommon<Derived, Memory<Derived, BufferInfo<Component, Method, szable, readonly, Parent> > >
		{
			CLASS_HEADER(_pri_::TextCommon<Derived, Memory<Derived, BufferInfo<Component, Method, szable, readonly, Parent> > >);
		public:
			INHERIT_COMPONENT();

			using Super::Super;
			using Super::begin;
			using Super::end;
			using Super::size;
		};

		template <class Derived, class Info>
		class WText:public _pri_::TextCommon<Derived, WMemory<Derived, Info> >
		{
			CLASS_HEADER(_pri_::TextCommon<Derived, WMemory<Derived, Info> >);
		public:
			INHERIT_COMPONENT();

			using Super::Super;
			using Super::begin;
			using Super::end;
			using Super::size;

			void toLowerCase() noexcept
			{
				static_assert(std::is_trivially_default_constructible<Component>::value, "Need to use non class component");
				KR_DEFINE_MMEM();
				memm::tolower(begin(), size());
			}
			void toUpperCase() noexcept
			{
				static_assert(std::is_trivially_default_constructible<Component>::value, "Need to use non class component");
				KR_DEFINE_MMEM();
				memm::toupper(begin(), size());
			}
		};
	}
}

template <class Derived, typename Next>
const typename kr::buffer::_pri_::TextCommon<Derived, Next>::Component(&kr::buffer::_pri_::TextCommon<Derived, Next>::WHITE_SPACE)[5] = kr::Names<Component>::_whitspace;