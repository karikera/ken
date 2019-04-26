#pragma once

namespace kr
{
	namespace buffer
	{
		template <typename Component, class Derived, typename Info>
		class TransBuffer;
		template <typename Component, class Derived, typename Info>
		class TransBuffer : public Info
		{
		public:
			static constexpr bool accessable = false;

			using Info::Info;
			using Info::szable; // bool

			size_t size() const noexcept
			{
				return static_cast<const Derived*>(this)->size();
			}
			size_t copyTo(Component * dest) const noexcept
			{
				return static_cast<const Derived*>(this)->copyTo(dest);
			}

			template <typename T> size_t sizeAs() const noexcept
			{
				static_assert(is_same<T, Component>::value, "Need same type");
				return size();
			}
			
			template <class ODerived, typename _Info>
			void writeTo(OutStream<ODerived, Component, _Info> * os) const
			{
				return static_cast<const Derived*>(this)->onWriteTo(os);
			}

			template <class _Derived, class _Parent>
			void onWriteTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>> * os) const
			{
				size_t sz = size();
				Component * dest = os->padding(sz + (szable ? 1 : 0));
				sz = copyTo(dest);
				os->commit(sz);
			}
			template <class _Derived, class _Parent>
			void onWriteTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>> * os) const
			{
				size_t sz = size();
				TmpArray<Component> temp;
				copyTo(temp.padding(sz + (szable ? 1 : 0)));
				os->write(temp.data(), sz);
			}
		};
		template <class Derived, typename Info>
		struct TransBuffer<AutoComponent, Derived, Info> :public Info
		{
		public:
			static constexpr bool accessable = false;

			// XXX: has multi component (for Number)
			using Info::Info;
			using Info::szable; // bool

			template <typename T>
			size_t sizeAs() const noexcept
			{
				return static_cast<const Derived*>(this)->template sizeAs<T>();
			}
			template <typename T>
			size_t copyTo(T * dest) const noexcept
			{
				return static_cast<const Derived*>(this)->copyTo(dest);
			}

			template <typename _Component,class _Derived, typename _Info>
			size_t writeTo(OutStream<_Derived, _Component, _Info> * os) const
			{
				return static_cast<const Derived*>(this)->onWriteTo(os);
			}

		protected:
			template <typename Component, class _Derived, class _Parent>
			size_t onWriteTo(OutStream<_Derived, Component, StreamInfo<true, _Parent>> * os) const
			{
				size_t sz = sizeAs<Component>();
				Component * dest = os->padding(sz + (szable ? 1 : 0));
				sz = copyTo<Component>(dest);
				os->commit(sz);
				return sz;
			}
			template <typename Component, class _Derived, class _Parent>
			size_t onWriteTo(OutStream<_Derived, Component, StreamInfo<false, _Parent>> * os) const
			{
				size_t sz = sizeAs<Component>();
				TmpArray<Component> temp(sz + (szable ? 1 : 0));
				copyTo<Component>(temp.data());
				os->write(temp.data(), sz);
				return sz;
			}
		};
	}
}