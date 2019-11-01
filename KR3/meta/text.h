#pragma once

#include <KR3/main.h>

#include "../main.h"
#include "types.h"

namespace kr
{
	namespace meta
	{
		template <char ... chars> struct text;

		namespace _pri_
		{
			template <typename ... t>
			struct textcat;
			template <char ... chars>
			struct textcat<text<chars ...>>
			{
				using type = text<chars...>;
			};;
			template <char ... chars, char... chars2, class ... texts>
			struct textcat<text<chars ...>, text<chars2...>, texts...>
			{
				using type = typename textcat<text<chars..., chars2...>, texts ...>::type;
			};;
		}

		// text
		template <> struct text<>
		{
			static const char * getValue()
			{
				return "";
			}
		};;
		template <char f, char ... chars> struct text<f,chars...>
		{
			constexpr static char first = f;
			using next = text<chars ... >;

			template <typename T>
			static const T * getValueAs() noexcept
			{
				static const T value[] = { (T)first, ((T)chars) ... , (T)'\0' };
				return value;
			}
			static const char * getValue() noexcept
			{
				return getValueAs<char>();
			}
		};;

		using emptytext = text<>;
		template <class ... texts> using textcat = typename _pri_::textcat<texts ... >::type;


		constexpr size_t strcount(const char * str, size_t sz, char niddle) noexcept
		{
			return
				sz == 0 ? 0 :
				(*str == niddle ? 1 : 0) +
				strcount(str + 1 + (sz - 1) / 2, sz / 2, niddle) +
				strcount(str + 1, (sz - 1) / 2, niddle);
		}
		template <size_t sz>
		constexpr size_t strcount(const char(&str)[sz], char niddle) noexcept
		{
			return strcount(str, sz, niddle);
		}

		template <typename T>
		struct constexpr_array
		{
			const T * value;
			size_t size;

			constexpr constexpr_array(const T * value, size_t sz) noexcept
				:value(value), size(sz)
			{
			}

			template <size_t sz>
			constexpr constexpr_array(const T(&value)[sz]) noexcept
				: value(value), size(sz - 1)
			{
			}

			constexpr const T& operator [](size_t idx) const noexcept
			{
				return value[idx];
			}

			constexpr operator const T*() const noexcept
			{
				return value;
			}
		};

		namespace _pri_
		{
			template <typename T>
			struct cbarray
			{

				template <size_t ... counter>
				struct data:Bufferable < data<counter ...>, BufferInfo<T, false, false, true, true> >
				{
					T value[sizeof ... (counter) + 1];

					constexpr data(const T * data) noexcept
						: value{ data[counter] ... , data[sizeof ...(counter)]}
					{
					}

					template <typename T2>
					constexpr explicit data(const T2 * data) noexcept
						: value{ ((T)data[counter]) ... , (T)data[sizeof ...(counter)] }
					{
					}

					constexpr data(const data& data) noexcept
						: value{((T)data.value[counter]) ..., (T)data.value[sizeof ...(counter)] }
					{
					}

					constexpr const T* $begin() const noexcept
					{
						return value;
					}
					constexpr const T* $end() const noexcept
					{
						return value + sizeof ... (counter);
					}
					constexpr size_t $size() const noexcept
					{
						return sizeof ... (counter);
					}
				};
			};
		}

		template <typename T, size_t sz>
		using constexpr_barray = typename make_numlist_counter<sz>::template expand<_pri_::cbarray<T>::template data>;

		template <size_t sz>
		using constexpr_btext = constexpr_barray<char, sz>;
		using constexpr_text = constexpr_array<char>;

		struct strcount_t :constexpr_text
		{
			char niddle;
			size_t count;

			template <size_t sz>
			constexpr strcount_t(const char(&str)[sz], char niddle) noexcept
				:constexpr_text(str, sz - 1), niddle(niddle), count(strcount(str, niddle))
			{
			}
		};

		template <typename T, size_t sz>
		constexpr meta::constexpr_barray<T, sz - 1> literal_as(const char(&text)[sz]) noexcept
		{
			return meta::constexpr_barray<T, sz - 1>(text);
		}
	}
}

#define DEFINE_META_TEXT(varname, str) \
	constexpr ::kr::meta::constexpr_text varname##_constexpr(str); \
	template <size_t ... counter> \
	struct varname##_expand { using type = ::kr::meta::text<varname##_constexpr[counter] ...>; }; \
	using varname = ::kr::meta::make_numlist_counter<varname##_constexpr.size>::expand<varname##_expand>::type;

