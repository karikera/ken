#pragma once

#include "reference.h"
#include "../meta/attacher.h"
#include "../meta/value.h"
#include "../meta/if.h"
#include "memlib.h"

#include <wchar.h>

namespace kr
{
	template <typename T> struct internal_component
	{
		using type = T;
	};
	template <> struct internal_component<void>
	{
		using type = byte;
	};
	template <typename T>
	using internal_component_t = typename internal_component<T>::type;

#define KR_DEFINE_MMEM() using memm = memt<sizeof(InternalComponent)>

	template <typename C, bool rdonly, class Parent> 
	class Container: public Parent
	{
	public:
		static constexpr bool readonly = rdonly;
		static constexpr bool is_string =
			is_same<C, char>::value ||
			is_same<C, wchar_t>::value ||
			is_same<C, char16>::value ||
			is_same<C, char32>::value;
		using Component = C;
		using ComponentRef = meta::if_t<rdonly, const C, C>;
		using InternalComponent = internal_component_t<C>;
		using InternalComponentRef = meta::if_t<rdonly, const InternalComponent, InternalComponent>;

		using Alc = Array<C>;
		using Ref = View<C>;
		using WRef = WView<C>;
		using SelfRef = meta::if_t<rdonly, View<C>, WView<C>>;
		using Wri = ArrayWriter<C>;

		using Parent::Parent;
	};
	template <bool rdonly, class Parent> 
	class Container<AutoComponent, rdonly, Parent> : public Parent
	{
	public:
		static constexpr bool readonly = rdonly;
		static constexpr bool is_string = true;
		using Component = AutoComponent;
		using InternalComponent = AutoComponent;
		using ComponentRef = meta::if_t<rdonly, const AutoComponent, AutoComponent>;
		using InternalComponentRef = ComponentRef;
		using memm = void;

		using Alc = void;
		using Ref = void;
		using WRef = void;
		using SelfRef = void;
		using Wri = void;

		using Parent::Parent;
	};


	template <bool _accessable, class Parent>
	class StreamInfo :public Parent
	{
	public:
		static constexpr bool accessable = _accessable;
		using StreamableBase = Undefined;

		using Parent::Parent;
	};
}

#define INHERIT_COMPONENT() \
	static_assert(kr::IsContainer<Super>::value, "Super is not Container"); \
	using typename Super::Component;\
	using typename Super::ComponentRef;\
	using typename Super::InternalComponent;\
	using typename Super::InternalComponentRef;\
	using typename Super::Alc;\
	using typename Super::Ref;\
	using typename Super::Wri;\
	using typename Super::WRef;\
	using typename Super::SelfRef;
