#pragma once

#include "container.h"

namespace kr
{
	template <bool _szable, class Parent = Empty>
	struct HasWriteToInfo :Parent
	{
		static constexpr bool szable = _szable;
	};

	template <class Derived, typename Component, typename Parent>
	class HasWriteTo : public Parent
	{
		using Super = Parent;
	public:
		using Super::Super;

		template <typename _Derived, typename _Info>
		void writeTo(OutStream<_Derived, Component, _Info>* dest) const noexcept
		{
			return static_cast<const Derived*>(this)->$writeTo(dest);
		}
	};

	template <class Derived, typename Parent>
	class HasWriteTo<Derived, AutoComponent, Parent> : public Parent
	{
		using Super = Parent;
	public:
		using Super::Super;

		template <typename _Derived, typename _Component, typename _Info>
		void writeTo(OutStream<_Derived, _Component, _Info>* dest) const noexcept
		{
			return static_cast<const Derived*>(this)->$writeTo(dest);
		}
	};

	template <bool _szable, class Parent = Empty>
	struct HasCopyToInfo :Parent
	{
		static constexpr bool szable = _szable;
	};

	template <class Derived, typename Component, typename Parent>
	class HasCopyTo : public Parent
	{
		using Super = Parent;
	public:
		using Super::Super;

		size_t copyTo(Component* dest) const noexcept
		{
			return static_cast<const Derived*>(this)->$copyTo(dest);
		}
		size_t size() const noexcept
		{
			return static_cast<const Derived*>(this)->$size();
		}
		template <typename T> size_t sizeAs() const noexcept
		{
			static_assert(is_same<T, Component>::value, "Need same type");
			return size();
		}
	};

	template <class Derived, typename Parent>
	class HasCopyTo<Derived, AutoComponent, Parent> : public Parent
	{
		using Super = Parent;
	public:
		using Super::Super;

		template <typename C>
		size_t copyTo(C* dest) const noexcept
		{
			return static_cast<const Derived*>(this)->$copyTo(dest);
		}
		template <typename T> size_t sizeAs() const noexcept
		{
			return static_cast<const Derived*>(this)->template $sizeAs<T>();
		}
	};

	template <size_t _maximum, size_t _extendsThreshold = 0, class Parent = Empty>
	struct HasOnlyCopyToInfo :Parent
	{
		static constexpr size_t maximum = _maximum;
		static constexpr size_t extendsThreshold = _extendsThreshold;
	};

	template <class Derived, typename Component, typename Parent>
	class HasOnlyCopyTo : public Parent
	{
		using Super = Parent;
	public:
		using Super::Super;

		size_t copyTo(Component* dest) const noexcept
		{
			return static_cast<const Derived*>(this)->$copyTo(dest);
		}
	};

	template <class Derived, typename Component, typename Parent>
	class CopyToByWriteTo :public HasWriteTo<Derived, Component, Parent>
	{
		CLASS_HEADER(CopyToByWriteTo, HasWriteTo<Derived, Component, Parent>);
	public:
		using Super::Super;
		using Super::writeTo;
		void copyTo(Component* dest) const noexcept
		{
			ArrayWriter<Component> out(dest, dest + 4096);
			try
			{
				writeTo(&out);
			}
			catch (...)
			{
				out = ArrayWriter<Component>(dest, dest + size());
				writeTo(&out);
			}
			return out.end() - (internal_component_t<Component>*)dest;
		}

		size_t size() const noexcept
		{
			io::SizeOStream<Component> size;
			writeTo(&size);
			return size.size();
		}
		template <typename T> size_t sizeAs() const noexcept
		{
			static_assert(is_same<T, Component>::value, "Need same type");
			return size();
		}
	};

	template <class Derived, typename Parent>
	class CopyToByWriteTo<Derived, AutoComponent, Parent> :
		public HasWriteTo<Derived, AutoComponent, Parent>
	{
		CLASS_HEADER(CopyToByWriteTo, HasWriteTo<Derived, AutoComponent, Parent>);
	public:
		using Super::Super;
		using Super::writeTo;

		template <typename C>
		size_t copyTo(C* dest) const
		{
			ArrayWriter<C> out(dest, dest + 4096);
			try
			{
				return writeTo(&out);
			}
			catch (NotEnoughSpaceException&)
			{
				out = ArrayWriter<C>(dest, dest + sizeAs<C>());
				return writeTo(&out);
			}
		}

		template <typename C> size_t sizeAs() const noexcept
		{
			io::SizeOStream<C> sstream;
			writeTo(&sstream);
			return sstream.size();
		}
	};

	template <class Derived, typename Component, typename Parent>
	class WriteToByCopyTo :public HasCopyTo<Derived, Component, Parent>
	{
		using Super = HasCopyTo<Derived, Component, Parent>;
	public:
		using Super::Super;
		using Super::copyTo;
		using Super::size;
		using Super::szable;

		template <class _Derived, class _Parent>
		void writeTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>>* os) const
		{
			size_t sz = size();
			Component* dest = os->padding(sz + (szable ? 1 : 0));
			copyTo(dest);
			os->commit(sz);
		}
		template <class _Derived, class _Parent>
		void writeTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>>* os) const
		{
			size_t sz = size();
			TmpArray<Component> temp;
			copyTo(temp.padding(sz + (szable ? 1 : 0)));
			os->write(temp.data(), sz);
		}
	};

	template <class Derived, typename Parent>
	class WriteToByCopyTo<Derived, AutoComponent, Parent> :public HasCopyTo<Derived, AutoComponent, Parent>
	{
		using Super = HasCopyTo<Derived, AutoComponent, Parent>;
	public:
		using Super::Super;
		using Super::szable;

		template <typename Component, class _Derived, class _Parent>
		size_t writeTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>>* os) const
		{
			size_t sz = this->template sizeAs<Component>();
			Component* dest = os->padding(sz + (szable ? 1 : 0));
			this->template copyTo<Component>(dest);
			os->commit(sz);
			return sz;
		}
		template <typename Component, class _Derived, class _Parent>
		size_t writeTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>>* os) const
		{
			size_t sz = this->template sizeAs<Component>();
			TmpArray<Component> temp;
			temp.resize(sz + (szable ? 1 : 0));
			this->template copyTo<Component>(temp.data());
			os->write(temp.data(), sz);
			return sz;
		}
	};

	template <class Derived, typename Component, typename Parent>
	class WriteToByOnlyCopyTo :public HasOnlyCopyTo<Derived, Component, Parent>
	{
		CLASS_HEADER(WriteToByOnlyCopyTo, HasOnlyCopyTo<Derived, Component, Parent>);
	public:
		using Super::Super;
		using Super::copyTo;
		using Super::szable;
		using Super::maximum;
		using Super::extendsThreshold;

		template <class _Derived, class _Parent>
		size_t writeTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>>* os) const
		{
			size_t size = maximum;
			for (;;) {
				Component* dest = os->padding(size + (szable ? 1 : 0));
				size_t sz = copyTo(dest);
				if (extendsThreshold != 0 && sz >= size + extendsThreshold - maximum) {
					size <<= 1;
					continue;
				}
				os->commit(sz);
				return sz;
			}
		}
		template <class _Derived, class _Parent>
		size_t writeTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>>* os) const
		{
			TmpArray<Component> temp;
			size_t size = maximum;
			for (;;) {
				temp.resize(size + (szable ? 1 : 0));
				size_t sz = copyTo(temp.data());
				if (extendsThreshold != 0 && sz >= size + extendsThreshold - maximum) {
					size <<= 1;
					continue;
				}
				os->write(temp.data(), sz);
				return sz;
			}
		}
	};

	template <class Derived, typename Parent>
	class WriteToByOnlyCopyTo<Derived, AutoComponent, Parent> :public HasOnlyCopyTo<Derived, AutoComponent, Parent>
	{
		CLASS_HEADER(WriteToByOnlyCopyTo, HasOnlyCopyTo<Derived, AutoComponent, Parent>);
	public:
		using Super::Super;
		using Super::szable;
		using Super::maximum;
		using Super::extendsThreshold;

		template <typename Component, class _Derived, class _Parent>
		size_t writeTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>>* os) const
		{
			size_t size = maximum;
			for (;;) {
				Component* dest = os->padding(size + (szable ? 1 : 0));
				size_t sz = this->template copyTo<Component>(dest);
				if (extendsThreshold != 0 && sz >= size+extendsThreshold-maximum) {
					size <<= 1;
					continue;
				}
				os->commit(sz);
				return sz;
			}
		}
		template <typename Component, class _Derived, class _Parent>
		size_t writeTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>>* os) const
		{
			TmpArray<Component> temp;
			size_t size = maximum;
			for (;;) {
				temp.resize(size + (szable ? 1 : 0));
				size_t sz = this->template copyTo<Component>(temp.data());
				if (extendsThreshold != 0 && sz >= size + extendsThreshold - maximum) {
					size <<= 1;
					continue;
				}
				os->write(temp.data(), sz);
				return sz;
			}
		}
	};

	template <typename Derived, typename C, typename Parent>
	class HasStreamTo :public Parent
	{
		using Super = Parent;
	public:
		using Super::Super;

		template <class _Derived, typename Info>
		void streamTo(OutStream<_Derived, C, Info>* os) throws(...)
		{
			return static_cast<Derived*>(this)->$streamTo(os);
		}
		template <class _Derived, typename Info>
		void streamTo(OutStream<_Derived, AutoComponent, Info>* os) throws(...)
		{
			return static_cast<Derived*>(this)->$streamTo(os->template cast<C>());
		}
	};

	template <typename Derived, typename Parent>
	class HasStreamTo<Derived, AutoComponent, Parent> :public Parent
	{
		using Super = Parent;
	public:
		template <class _Derived, typename C, typename Info>
		void streamTo(OutStream<_Derived, C, Info>* os) const throws(...)
		{
			return static_cast<Derived>(this)->$streamTo(os);
		}
	};

#define KR_STREAM_TO(os) template <typename _Derived, typename _Info> \
	void $streamTo(kr::OutStream<_Derived, Component, _Info>* os)

}