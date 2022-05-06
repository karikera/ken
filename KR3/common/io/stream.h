#pragma once

#include "../container.h"
#include "istream.h"
#include "ostream.h"


namespace kr
{
	namespace io
	{
		template <class Derived, typename _Info = StreamInfo<>>
		class StreamCastable:public _Info
		{
		public:
			template <typename C>
			using SStream = StreamableStream<Derived, C>;

			void write(const void* data, size_t size)
			{
				return static_cast<Derived*>(this)->$write(data, size);
			}
			size_t read(void* data, size_t size)
			{
				return static_cast<Derived*>(this)->$read(data, size);
			}

			template <typename C>
			SStream<C>* stream() noexcept
			{
				return static_cast<SStream<C>*>(this);
			}
		};

		template <class Base, typename C>
		class StreamableStream :
			public InOutStream<StreamableStream<Base, C>, C, StreamInfo<Base::accessable, Base>>
		{
			CLASS_HEADER(InOutStream<StreamableStream<Base, C>, C, StreamInfo<Base::accessable, Base>>);
		public:
			INHERIT_COMPONENT();
			using StreamableBase = Base;

			static_assert(sizeof(InternalComponent) == 1, "size unmatch");

			void $write(const C * data, size_t size)
			{
				return Base::$write((void*)data, size);
			}
			size_t $read(C * data, size_t size)
			{
				return Base::$read((void*)data, size);
			}

		public:
			using Super::Super;
		};


	}
}
