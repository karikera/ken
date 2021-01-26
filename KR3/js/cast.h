#pragma once

#include "type.h"
#include "rawdata.h"
#include "undefined.h"

namespace kr
{
	namespace _pri_
	{
		template <bool test, typename _true, typename _false>
		struct JsIfType;

		template <typename _true, typename _false>
		struct JsIfType<true, _true, _false>
		{
			using type = _true;
		};
		template <typename _true, typename _false>
		struct JsIfType<false, _true, _false>
		{
			using type = _false;
		};

		// InnerType : types that can be contained by JsValue
		// OuterType : external usable types
		struct JsCast
		{
			static int doubleToInt(double _value) noexcept;

			template <typename To, typename From>
			struct ToOuter
			{
				static To toOuter(const From& _value) noexcept
				{
					return (To)_value;
				}
				static To toOuter(From&& _value) noexcept
				{
					return To(_value);
				}
			};
			template <typename To>
			struct ToOuter<To*, JsObjectRawData>
			{
				static To* toOuter(const JsObjectRawData& _value) noexcept;
			};
			template <typename T, typename TI>
			static T toOuter(const TI& _value) noexcept
			{
				return ToOuter<T, TI>::toOuter(_value);
			}
			template <typename T, typename TI>
			static T toOuter(TI&& _value) noexcept
			{
				return ToOuter<T, decay_t<TI> >::toOuter(move(_value));
			}

#define CASTER(from, to) \
static to toInner(from _value) noexcept { return (to)_value; }
#define CASTER_COPY(from, to) \
static to toInner(const from & _value) noexcept { return (to)_value; }
#define CASTER_MOVE(from, to) \
static to toInner(const from & _value) noexcept { return (to)_value; }\
static to toInner(from && _value) noexcept { return to(move(_value)); }

			using long_to_what_t = typename JsIfType<(sizeof(long) > sizeof(int)), double, int>::type;

			CASTER(char, int);
			CASTER(short, int);
			CASTER(int, int);
			CASTER(long long, double);
			CASTER(unsigned char, int);
			CASTER(unsigned short, int);
			CASTER(unsigned int, double);
			CASTER(unsigned long, double);
			CASTER(unsigned long long, double);
			CASTER(bool, bool);
			CASTER(double, double);
			CASTER(float, double);
			CASTER(Text16, Text16);
			CASTER(nullptr_t, nullptr_t);
			CASTER(undefined_t, undefined_t);
#undef CASTER
#undef CASTER_MOVE
#undef CASTER_COPY

			static long_to_what_t toInner(long _value) noexcept;
			static TText16 toInner(Text _value, Charset cs = Charset::Default) noexcept;
			static TText16 toInner(const AText& _value, Charset cs = Charset::Default) noexcept;
			static TText16 toInner(const TText& _value, Charset cs = Charset::Default) noexcept;
			static Text16 toInner(const AText16& _value) noexcept;
			static Text16 toInner(const TText16& _value) noexcept;
			static JsObjectRawData toInner(JsNewObject_t) noexcept;
			static JsObjectRawData toInner(JsNewArray arr) noexcept;
			static JsObjectRawData toInner(JsNewSymbol symbol) noexcept;
			static JsArrayBufferRawData toInner(JsNewArrayBuffer arr) noexcept;
			static JsTypedArrayRawData toInner(JsNewTypedArray arr) noexcept;
			static JsRawData toInner(const JsPersistent& value) noexcept;
			static JsRawData toInner(const JsRawData& value) noexcept;
			static void toInner(const void*) noexcept = delete;
			template <size_t size>
			static TText16 toInner(const char(&str)[size], Charset cs = Charset::Default) noexcept
			{
				return toInner((Text)str, cs);
			}
			template <size_t size>
			static Text16 toInner(const char16(&str)[size]) noexcept
			{
				return (Text16)str;
			}
			static JsObjectRawData toInner(JsObject* object) noexcept;

			template <typename T>
			static T defaultValue() noexcept;
		};

		template <typename T> struct OuterBridge
		{
			using type = decltype(JsCast::toInner(declval<T>()));
		};
		template <class Data, typename C> struct OuterBridge<ary::WrapImpl<Data, C> >
		{
			using type = Array<C>;
		};
		template <typename C> struct OuterBridge<View<C> >
		{
			using type = View<char16_t>;
		};

		template <typename T> struct InnerBridge
		{
			using type = decltype(JsCast::toInner(declval<T>()));
		};
		template <class Data, typename C> struct InnerBridge<ary::WrapImpl<Data, C> >
		{
			using type = View<C>;
		};

#define DEFDECL(type) template <> type JsCast::defaultValue<type>() noexcept
		DEFDECL(int);
		DEFDECL(double);
		DEFDECL(bool);
		DEFDECL(nullptr_t);
		DEFDECL(undefined_t);
		DEFDECL(Text16);
		DEFDECL(JsRawData);
		DEFDECL(AText16);
		DEFDECL(AText);
		DEFDECL(Text);
		DEFDECL(void);
#undef DEFDECL

		template <typename T>
		T JsCast::defaultValue() noexcept
		{
			using type = typename InnerBridge<T>::type;
			return toOuter<T>(defaultValue<type>());
		}

		enum CastType
		{
			CastTypeSame,
			CastTypeCast,
			CastTypeDefault
		};
		template <CastType type, typename S, typename D> struct ComputeCastImpl;
		template <typename S, typename D> struct ComputeCastImpl<CastTypeCast, S, D>
		{
			static D cast(const S& v) noexcept
			{
				return (D)v;
			}
		};
		template <typename S, typename D> struct ComputeCastImpl<CastTypeSame, S, D>
		{
			static const D& cast(const S& v) noexcept
			{
				return v;
			}
		};
		template <typename S, typename D> struct ComputeCastImpl<CastTypeDefault, S, D>
		{
			static D cast(const S&) noexcept
			{
				return JsCast::defaultValue<D>();
			}
		};
		template <typename T>
		struct IsTypedRawData :meta::bool_false {};
		template <JsType type>
		struct IsTypedRawData<JsTypedRawData<type>> :meta::bool_true {};

		template <typename S, typename D> struct ComputeCast : public ComputeCastImpl<
			std::is_same<S, D>::value ? CastTypeSame :
			(
				IsTypedRawData<D>::value ||
				std::is_same<D, JsObjectRawData>::value ||
				std::is_same<D, undefined_t>::value ||
				std::is_same<D, nullptr_t>::value ||
				IsTypedRawData<S>::value ||
				std::is_same<S, undefined_t>::value ||
				std::is_same<S, nullptr_t>::value
				) ? CastTypeDefault : CastTypeCast, S, D>
		{
		};
		template <> struct ComputeCast<double, int>
		{
			static int cast(double v) noexcept { return JsCast::doubleToInt(v); }
		};
		template <> struct ComputeCast<double, unsigned int>
		{
			static unsigned int cast(double v) noexcept { return JsCast::doubleToInt(v); }
		};
		template <> struct ComputeCast<int, bool>
		{
			static bool cast(int v) noexcept { return v != 0; }
		};
		template <> struct ComputeCast<double, bool>
		{
			static bool cast(double v) noexcept { return v != 0; }
		};
		template <> struct ComputeCast<nullptr_t, bool>
		{
			static bool cast(nullptr_t) noexcept { return false; }
		};
		template <> struct ComputeCast<undefined_t, bool>
		{
			static bool cast(undefined_t) noexcept { return false; }
		};
		template <JsType jstype> struct ComputeCast<JsTypedRawData<jstype>, bool>
		{
			static bool cast(const JsTypedRawData<jstype>&) noexcept {
				return true;
			}
		};
		template <> struct ComputeCast<Text16, bool>
		{
			static bool cast(const Text16& v) noexcept { return !v.empty(); }
		};
		template <> struct ComputeCast<Text16, int>
		{
			static int cast(const Text16& v) noexcept { return v.to_int(); }
		};
		template <> struct ComputeCast<Text16, double>
		{
			static double cast(const Text16& v) noexcept { return v.to_float(); }
		};
		template <> struct ComputeCast<AText16, bool> :ComputeCast<Text16, bool> {};
		template <> struct ComputeCast<AText16, int> :ComputeCast<Text16, int> {};
		template <> struct ComputeCast<AText16, double> :ComputeCast<Text16, double> {};

#define RETURN_STR(...) static const C str[] = {__VA_ARGS__, (C)'\0'}; return View<C>(str, countof(str)-1);
		template <class Data, typename C> struct ComputeCast<nullptr_t, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(nullptr_t) noexcept {
				RETURN_STR('n', 'u', 'l', 'l');
			}
		};
		template <class Data, typename C> struct ComputeCast<undefined_t, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(undefined_t) noexcept {
				RETURN_STR('u', 'n', 'd', 'e', 'f', 'i', 'n', 'e', 'd');
			}
		};
		template <class Data, typename C> struct ComputeCast<JsFunctionRawData, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(const JsFunctionRawData&) noexcept {
				RETURN_STR('[', 'o', 'b', 'j', 'e', 'c', 't', ' ', 'F', 'u', 'n', 'c', 'u', 't', 'i', 'o', 'n', ']');
			}
		};
		template <class Data, typename C> struct ComputeCast<JsObjectRawData, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(const JsObjectRawData&) noexcept {
				RETURN_STR('[', 'o', 'b', 'j', 'e', 'c', 't', ' ', 'O', 'b', 'j', 'e', 'c', 't', ']');
			}
		};
		template <class Data, typename C> struct ComputeCast<JsArrayBufferRawData, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(const JsArrayBufferRawData&) noexcept {
				RETURN_STR('[', 'o', 'b', 'j', 'e', 'c', 't', ' ',
					'A', 'r', 'r', 'a', 'y', 'B', 'u', 'f', 'f', 'e', 'r', ']');
			}
		};
		template <class Data, typename C> struct ComputeCast<JsTypedArrayRawData, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(const JsTypedArrayRawData&) noexcept {
				RETURN_STR('[', 'o', 'b', 'j', 'e', 'c', 't', ' ',
					'T', 'y', 'p', 'e', 'd', 'A', 'r', 'r', 'a', 'y', 'B', 'u', 'f', 'f', 'e', 'r', ']');
			}
		};
		template <class Data, typename C> struct ComputeCast<JsDataViewRawData, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(const JsDataViewRawData&) noexcept {
				RETURN_STR('[', 'o', 'b', 'j', 'e', 'c', 't', ' ',
					'D', 'a', 't', 'a', 'V', 'i', 'e', 'w', ']');
			}
		};
		template <class Data, typename C> struct ComputeCast<bool, ary::WrapImpl<Data, C> >
		{
			static View<C> cast(bool v) noexcept {
				if (v)
				{
					RETURN_STR('t', 'r', 'u', 'e');
				}
				else
				{
					RETURN_STR('f', 'a', 'l', 's', 'e');
				}
			}
		};
		template <class Data, typename C> struct ComputeCast<int, ary::WrapImpl<Data, C> >
		{
			using XText16 = ary::WrapImpl<Data, C>;
			static XText16 cast(int v) noexcept {
				return XText16::concat(v);
			}
		};
		template <typename C> View<C> __number_str() noexcept
		{
			RETURN_STR('[', 'n', 'u', 'm', 'b', 'e', 'r', ']');
		}
		template <typename C> struct ComputeCast<int, View<C>>
		{
			static View<C> cast(int) noexcept {
				return __number_str<C>();
			}
		};
		template <class Data, typename C> struct ComputeCast<double, ary::WrapImpl<Data, C> >
		{
			using XText16 = ary::WrapImpl<Data, C>;
			static XText16 cast(double v) noexcept {
				return XText16::concat(v);
			}
		};
		template <typename C> struct ComputeCast<double, View<C>>
		{
			static View<C> cast(double) noexcept {
				return __number_str<C>();
			}
		};
		template <class Data> struct ComputeCast<Text16, ary::WrapImpl<Data, char>>
		{
			using XText = ary::WrapImpl<Data, char>;
			static XText cast(Text16 text) noexcept
			{
				XText out;
				out << toUtf8(text);
				return move(out);
			}
		};
		template <class Data> struct ComputeCast<Text16, ary::WrapImpl<Data, char32_t>>
		{
			using XText = ary::WrapImpl<Data, char>;
			static XText cast(Text16 text) noexcept
			{
				XText out;
				out << toUtf32(text);
				return move(out);
			}
		};
		template <typename C> struct ComputeCast<Text16, View<C>>
		{
			static View<C> cast(Text16 text) noexcept = delete;
		};

		template <> struct ComputeCast<Text16, Text16>
		{
			static Text16 cast(Text16 text) noexcept
			{
				return text;
			}
		};

		template <typename T>
		struct CheckDirectReturn
		{
			static constexpr bool possible = false;
			
			ATTR_NORETURN static T cast(const JsValue*) noexcept {
				unreachable();
			}
		};

		struct CheckDirectReturnPossible
		{
			static constexpr bool possible = true;
			static const JsValue& cast(const JsValue* jsval) noexcept {
				return *jsval;
			}
		};

		template <>
		struct CheckDirectReturn<const JsValue&> :CheckDirectReturnPossible
		{
		};
		template <>
		struct CheckDirectReturn<JsValue&>:CheckDirectReturnPossible
		{
		};
		template <>
		struct CheckDirectReturn<const JsValue>:CheckDirectReturnPossible
		{
		};
		template <>
		struct CheckDirectReturn<JsValue> :CheckDirectReturnPossible
		{
		};
	}
}