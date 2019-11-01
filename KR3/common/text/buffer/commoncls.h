#pragma once

#include "../container.h"

namespace kr
{
	namespace buffer
	{
		namespace _pri_
		{
			template <class Derived, typename Component, typename Info>
			class WriteToImpl : public Info
			{
				using Super = Info;
			public:
				INHERIT_COMPONENT();
				using Super::Super;

				template <typename _Derived, typename _Info>
				void writeTo(OutStream<_Derived, Component, _Info>* dest) const noexcept
				{
					return static_cast<const Derived*>(this)->$writeTo(dest);
				}
			};
			template <class Derived, typename Info>
			class WriteToImpl<Derived, AutoComponent, Info> : public Info
			{
				using Super = Info;
			public:
				INHERIT_COMPONENT();
				using Super::Super;

				template <typename _Derived, typename _Component, typename _Info>
				void writeTo(OutStream<_Derived, _Component, _Info>* dest) const noexcept
				{
					return static_cast<const Derived*>(this)->$writeTo(dest);
				}
			};
			template <class Derived, typename Component, typename Info>
			class CopyToImpl : public Info
			{
				using Super = Info;
			public:
				INHERIT_COMPONENT();
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
			template <class Derived, typename Info>
			class CopyToImpl<Derived, AutoComponent, Info> : public Info
			{
				using Super = Info;
			public:
				INHERIT_COMPONENT();
				using Super::Super;

				template <typename C>
				size_t copyTo(C* dest) const noexcept
				{
					return static_cast<const Derived*>(this)->$copyTo(dest);
				}
				size_t size() const noexcept
				{
					return static_cast<const Derived*>(this)->$size();
				}
				template <typename T> size_t sizeAs() const noexcept
				{
					return static_cast<const Derived*>(this)->template $sizeAs<T>();
				}
			};

			template <class Derived, typename Component, typename Info>
			class WriteToOnlyImpl :public WriteToImpl<Derived, Component, Info>
			{
				CLASS_HEADER(WriteToOnlyImpl, WriteToImpl<Derived, Component, Info>);
			public:
				INHERIT_COMPONENT();
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
					return (InternalComponent*)out.end() - (InternalComponent*)dest;
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

			template <class Derived, typename Info>
			class WriteToOnlyImpl<Derived, AutoComponent, Info> :
				public WriteToImpl<Derived, AutoComponent, Info>
			{
				CLASS_HEADER(WriteToOnlyImpl, WriteToImpl<Derived, AutoComponent, Info>);
			public:
				INHERIT_COMPONENT();
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
					catch (...)
					{
						out = ArrayWriter<C>(dest, dest + sizeAs<C>());
						return writeTo(&out);
					}
				}

				template <typename C> size_t sizeAs() const noexcept
				{
					io::SizeOStream<C> size;
					writeTo(&size);
					return size.size();
				}

			};

			template <class Derived, typename Component, typename Info>
			class CopyToOnlyImpl :public CopyToImpl<Derived, Component, Info>
			{
				using Super = CopyToImpl<Derived, Component, Info>;
			public:
				INHERIT_COMPONENT();
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

			template <class Derived, typename Info>
			class CopyToOnlyImpl<Derived, AutoComponent, Info> :public CopyToImpl<Derived, AutoComponent, Info>
			{
				using Super = CopyToImpl<Derived, AutoComponent, Info>;
			public:
				INHERIT_COMPONENT();
				using Super::Super;
				using Super::template copyTo;
				using Super::template sizeAs;
				using Super::szable;

				template <typename Component, class _Derived, class _Parent>
				size_t writeTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>>* os) const
				{
					size_t sz = sizeAs<Component>();
					Component* dest = os->padding(sz + (szable ? 1 : 0));
					copyTo<Component>(dest);
					os->commit(sz);
					return sz;
				}
				template <typename Component, class _Derived, class _Parent>
				size_t writeTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>>* os) const
				{
					size_t sz = sizeAs<Component>();
					TmpArray<Component> temp(sz + (szable ? 1 : 0));
					copyTo<Component>(temp.data());
					os->write(temp.data(), sz);
					return sz;
				}
			};

			template <class Derived, typename Component, typename Info>
			using CopyToWriteToImpl = CopyToImpl<Derived, Component, WriteToImpl<Derived, Component, Info> >;
		}

	}
}