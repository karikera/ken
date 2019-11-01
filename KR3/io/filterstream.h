#pragma once

#include <KR3/main.h>

namespace kr
{
	namespace io
	{
		template <class Derived, class Base, bool autoClose, typename NewComponent>
		class FilterIStream : public InStream<Derived, NewComponent, StreamInfo<>>
		{
			CLASS_HEADER(FilterIStream, InStream<Derived, NewComponent, StreamInfo<>>);
		public:
			static_assert(IsIStream<Base>::value, "Base is not InStream");
			INHERIT_COMPONENT();

			FilterIStream(Base* p) noexcept;
			~FilterIStream() noexcept;
			size_t $read(NewComponent* data, size_t nSize) = delete;
			void resetStream(Base * p) noexcept;
			Base* base() const noexcept;

		private:
			Base* m_pt;
		};

		template <class Derived, class Base, bool autoClose, typename NewComponent>
		class FilterOStream : public OutStream<Derived, NewComponent>
		{
		public:
			CLASS_HEADER(FilterOStream, OutStream<Derived, NewComponent>);
			static_assert(IsOStream<Base>::value, "Base is not OutStream");
			INHERIT_COMPONENT();

			FilterOStream(Base* p) noexcept;
			~FilterOStream() noexcept;
			void $write(const NewComponent* data, size_t nSize) = delete;
			void resetStream(nullptr_t) noexcept;
			void resetStream(Base * p) noexcept;
			void resetStream(typename Base::StreamableBase * p) noexcept;
			Base* base() const noexcept;
			void close() noexcept;

		private:
			Base* m_pt;
		};
	}
}

template <class Derived, class Base, bool autoClose, typename NewComponent>
inline kr::io::FilterIStream<Derived, Base, autoClose, NewComponent>::FilterIStream(Base* p) noexcept
	:m_pt(p)
{
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline kr::io::FilterIStream<Derived, Base, autoClose, NewComponent>::~FilterIStream() noexcept
{
	if(autoClose)
		delete m_pt;
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline void kr::io::FilterIStream<Derived, Base, autoClose, NewComponent>::resetStream(Base * p) noexcept
{
	m_pt = p;
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline Base* kr::io::FilterIStream<Derived, Base, autoClose, NewComponent>::base() const noexcept
{
	return m_pt;
}

template <class Derived, class Base, bool autoClose, typename NewComponent>
inline kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::FilterOStream(Base* p) noexcept
	:m_pt(p)
{
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::~FilterOStream() noexcept
{
	if(autoClose)
		delete m_pt;
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline void kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::resetStream(nullptr_t) noexcept
{
	m_pt = nullptr;
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline void kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::resetStream(Base * p) noexcept
{
	m_pt = p;
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline void kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::resetStream(typename Base::StreamableBase * p) noexcept
{
	m_pt = p->template stream<Component>();
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline Base* kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::base() const noexcept
{
	return m_pt;
}
template <class Derived, class Base, bool autoClose, typename NewComponent>
inline void kr::io::FilterOStream<Derived, Base, autoClose, NewComponent>::close() noexcept
{
	delete m_pt;
	m_pt = nullptr;
}
