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
			template <typename To, typename From>
			struct ToOuter
			{
				static To toOuter(const From& _value) noexcept
				{
					return (To)_value;
				}
				static To toOuter(From&& _value) noexcept
				{
					return To(move(_value));
				}
			};
			template <typename ToData, typename FromData>
			struct ToOuter<ary::WrapImpl<ToData, char>, ary::WrapImpl<FromData, char16> >
			{
				using XText = ary::WrapImpl<ToData, char>;
				static XText toOuter(Text16 _value) noexcept
				{
					XText text;
					text << toUtf8(_value);
					return text;
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
				return ToOuter<T, TI>::toOuter(move(_value));
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
			CASTER(unsigned int, int);
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
			using type = decltype(toInner(declval<T>()));
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

		template <class Data> struct ComputeCast<nullptr_t, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(nullptr_t) noexcept {
				return u"null";
			}
		};
		template <class Data> struct ComputeCast<undefined_t, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(undefined_t) noexcept {
				return u"undefined";
			}
		};
		template <class Data> struct ComputeCast<JsFunctionRawData, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(const JsFunctionRawData&) noexcept {
				return u"[object Function]";
			}
		};
		template <class Data> struct ComputeCast<JsObjectRawData, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(const JsObjectRawData&) noexcept {
				return u"[object Object]";
			}
		};
		template <class Data> struct ComputeCast<JsArrayBufferRawData, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(const JsArrayBufferRawData&) noexcept {
				return u"[object ArrayBuffer]";
			}
		};		
		template <class Data> struct ComputeCast<JsTypedArrayRawData, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(const JsTypedArrayRawData&) noexcept {
				return u"[object TypedArrayBuffer]";
			}
		};
		template <class Data> struct ComputeCast<JsDataViewRawData, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(const JsDataViewRawData&) noexcept {
				return u"[object DataView]";
			}
		};
		template <class Data> struct ComputeCast<bool, ary::WrapImpl<Data, char16_t> >
		{
			static Text16 cast(bool v) noexcept {
				return v ? (Text16)u"true" : (Text16)u"false";
			}
		};
		template <class Data> struct ComputeCast<int, ary::WrapImpl<Data, char16_t> >
		{
			using XText16 = ary::WrapImpl<Data, char16_t>;
			static XText16 cast(int v) noexcept {
				return XText16::concat(v);
			}
		};
		template <> struct ComputeCast<int, Text16>
		{
			static Text16 cast(int) noexcept {
				return u"[number]";
			}
		};
		template <class Data> struct ComputeCast<double, ary::WrapImpl<Data, char16_t> >
		{
			using XText16 = ary::WrapImpl<Data, char16_t>;
			static XText16 cast(double v) noexcept {
				return XText16::concat(v);
			}
		};
		template <> struct ComputeCast<double, Text16>
		{
			static Text16 cast(double) noexcept {
				return u"[number]";
			}
		};

		template <typename T> struct GetBridgeType
		{
			using type = decltype(_pri_::JsCast::toInner(declval<T>()));
		};
		template <> struct GetBridgeType<AText16>
		{
			using type = AText16;
		};
	}
}