#pragma once

#include <type_traits>

#include "type.h"
#include "rawdata.h"
#include "value.h"
#include "string.h"
#include "function.h"
#include "object.h"
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
			template <typename T,typename TI>
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
			static Text16 toInner(const AText16& _value) noexcept;
			static JsRawData toInner(JsNewObject_t) noexcept;
			static JsRawData toInner(JsNewArray arr) noexcept;
			static JsRawData toInner(JsNewTypedArray arr) noexcept;
			static JsRawData toInner(const JsPersistent& value) noexcept;

			template <typename T>
			static T defaultValue() noexcept
			{
				using type = decltype(toInner(*(T*)0));
				return toOuter<T>(defaultValue<type>());
			}
			template <>
			static void defaultValue<void>() noexcept
			{
			};
#define DEFAULT(type, v)	template <> static type defaultValue<type>() noexcept { return v; }
			DEFAULT(int, 0);
			DEFAULT(double, NAN);
			DEFAULT(bool, false);
			DEFAULT(nullptr_t, nullptr);
			DEFAULT(undefined_t, undefined);
			DEFAULT(Text16, u"");
#undef DEFAULT
		};

		enum CastType
		{
			CastTypeSame,
			CastTypeCast,
			CastTypeDefault
		};
		template <CastType type, typename S, typename D> struct ComputeCastImpl;
		template <typename S, typename D> struct ComputeCastImpl<CastTypeCast, S, D>
		{
			static D cast(const S &v) noexcept
			{
				return (D)v;
			}
		};
		template <typename S, typename D> struct ComputeCastImpl<CastTypeSame, S, D>
		{
			static const D & cast(const S &v) noexcept
			{
				return v;
			}
		};
		template <typename S, typename D> struct ComputeCastImpl<CastTypeDefault, S, D>
		{
			static D cast(const S &) noexcept
			{
				return JsCast::defaultValue<D>();
			}
		};
		template <typename T>
		struct IsTypedRawData :meta::bool_false{};
		template <JsType type>
		struct IsTypedRawData<JsTypedRawData<type>> :meta::bool_true {};

		template <typename S, typename D> struct ComputeCast: public ComputeCastImpl<
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
		template <typename S> struct ComputeCast<S, bool>
		{
			static bool cast(const S &v) noexcept { return v != 0; }
		};
		template <> struct ComputeCast<nullptr_t, bool>
		{
			static bool cast(nullptr_t) noexcept { return false; }
		};
		template <> struct ComputeCast<undefined_t, bool>
		{
			static bool cast(undefined_t) noexcept { return false; }
		};
		template <> struct ComputeCast<Text16, bool>
		{
			static bool cast(const Text16 &v) noexcept { return !v.empty(); }
		};
		template <> struct ComputeCast<Text16, int>
		{
			static int cast(const Text16&v) noexcept { return v.to_int(); }
		};
		template <> struct ComputeCast<Text16, double>
		{
			static double cast(const Text16&v) noexcept{ return v.to_float(); }
		};
		template <> struct ComputeCast<bool, Text16>
		{
			static Text16 cast(bool v) noexcept {
				return v ? (Text16)u"true" : (Text16)u"false";
			}
		};
		template <> struct ComputeCast<int, Text16>
		{
			static Text16 cast(int) noexcept {
				return u"[number]";
			}
		};
		template <> struct ComputeCast<double, Text16>
		{
			static Text16 cast(double) noexcept {
				return u"[number]";
			}
		};
		template <> struct ComputeCast<nullptr_t, Text16>
		{
			static Text16 cast(nullptr_t) noexcept {
				return u"null";
			}
		};
		template <> struct ComputeCast<undefined_t, Text16>
		{
			static Text16 cast(undefined_t) noexcept {
				return u"undefined";
			}
		};
		template <> struct ComputeCast<JsFunctionRawData, Text16>
		{
			static Text16 cast(const JsFunctionRawData&) noexcept {
				return u"[object Function]";
			}
		};
		template <> struct ComputeCast<JsObjectRawData, Text16>
		{
			static Text16 cast(const JsObjectRawData&) noexcept {
				return u"[object Object]";
			}
		};
		template <> struct ComputeCast<JsArrayBufferRawData, Text16>
		{
			static Text16 cast(const JsArrayBufferRawData&) noexcept {
				return u"[object ArrayBuffer]";
			}
		};
		template <> struct ComputeCast<JsTypedArrayRawData, Text16>
		{
			static Text16 cast(const JsArrayBufferRawData&) noexcept {
				return u"[object TypedArrayBuffer]";
			}
		};
		template <> struct ComputeCast<JsDataViewRawData, Text16>
		{
			static Text16 cast(const JsDataViewRawData&) noexcept {
				return u"[object DataView]";
			}
		};
		template <> struct ComputeCast<int, AText16>
		{
			static AText16 cast(int v) noexcept {
				return AText16::concat(v);
			}
		};
		template <> struct ComputeCast<double, AText16>
		{
			static AText16 cast(double v) noexcept {
				return AText16::concat(v);
			}
		};
		template <> struct ComputeCast<bool, AText16> :public ComputeCast<bool, Text16> {};
		template <> struct ComputeCast<undefined_t, AText16> :public ComputeCast<undefined_t, Text16> {};
		template <> struct ComputeCast<nullptr_t, AText16> :public ComputeCast<nullptr_t, Text16> {};
		template <> struct ComputeCast<JsFunctionRawData, AText16> :public ComputeCast<JsFunctionRawData, Text16>{};
		template <> struct ComputeCast<JsObjectRawData, AText16> :public ComputeCast<JsObjectRawData, Text16> {};
		template <> struct ComputeCast<JsArrayBufferRawData, AText16> :public ComputeCast<JsArrayBufferRawData, Text16> {};
		template <> struct ComputeCast<JsTypedArrayRawData, AText16> :public ComputeCast<JsTypedArrayRawData, Text16> {};
		template <> struct ComputeCast<JsDataViewRawData, AText16> :public ComputeCast<JsDataViewRawData, Text16> {};

		template <typename T> struct GetBridgeType
		{
			using type = decltype(_pri_::JsCast::toInner(*(T*)0));
		};
		template <> struct GetBridgeType<AText16>
		{
			using type = AText16;
		};
	}
}

template <typename T>
void kr::JsValue::_ctor(const T & value) noexcept
{
	m_data = JsRawData(_pri_::JsCast::toInner(value));
}
template <typename T>
void kr::JsValue::_ctor(T && value) noexcept
{
	m_data = JsRawData(move(_pri_::JsCast::toInner(value)));
}
template <typename T>
kr::JsValue::JsValue(const T& value) noexcept
{
	_ctor(value);
}
template <typename T>
kr::JsValue::JsValue(T&& value) noexcept
{
	_ctor(move(value));
}
template <typename T>
T kr::JsValue::as() const noexcept
{
	return m_data.get<T>();
}

template <typename T> T kr::JsValue::cast() const noexcept
{
	using Inner = typename _pri_::GetBridgeType<T>::type;
	KRJS_TYPE_CONSTLIZE(
		return _pri_::JsCast::toOuter<T>(_pri_::ComputeCast<type, Inner>::cast(as<type>()));
	);
	return _pri_::JsCast::defaultValue<T>();
}
