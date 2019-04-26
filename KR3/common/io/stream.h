#pragma once

#include "../container.h"
#include "istream.h"
#include "ostream.h"


namespace kr
{
	namespace io
	{
		template <class Derived, typename _Info = StreamInfo<>>
		class Streamable:public _Info
		{
		public:
			template <typename C>
			using Stream = StreamableStream<Derived, C>;

			template <typename C>
			Stream<C> * stream() noexcept
			{
				return static_cast<Stream<C>*>(this);
			}
		};

		template <class Base, typename C>
		class StreamableStream :
			public InOutStream<StreamableStream<Base, C>, C, StreamInfo<Base::accessable, Base>>
		{
			CLASS_HEADER(StreamableStream, InOutStream<StreamableStream<Base, C>, C, StreamInfo<Base::accessable, Base>>);
		public:
			INHERIT_COMPONENT();
			using StreamableBase = Base;

			static_assert(sizeof(InternalComponent) == 1, "size unmatch");

			void writeImpl(const C * data, size_t size)
			{
				return Base::writeImpl((void*)data, size);
			}
			size_t readImpl(C * data, size_t size)
			{
				return Base::readImpl((void*)data, size);
			}

		public:
			using Super::Super;
		};


	}
}
