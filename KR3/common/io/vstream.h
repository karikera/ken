#pragma once

#include "istream.h"
#include "ostream.h"

namespace kr
{
	namespace io
	{
		template <typename C, class Parent = Empty> class VIStream;
		template <typename C, class Parent = Empty> class VOStream;


		template <typename C, class Parent>
		class VIStream : public InStream<VIStream<C, Parent>, C, StreamInfo<false, Parent>>
		{
			CLASS_HEADER(VIStream, InStream<VIStream<C, Parent>, C, StreamInfo<false, Parent>>);
		public:
			INHERIT_COMPONENT();

		private:
			void * m_stream;
			size_t(*m_read)(void * stream, C * data, size_t sz);

		public:
			VIStream() noexcept
			{
				reset();
			}
			VIStream(const VIStream&) = default;

			VIStream(FILE * file) noexcept
				: m_stream(file)
			{
				m_read = [](void * stream, C * data, size_t sz)->size_t {
					return fread(data, sizeof(InternalComponent), sz, (FILE*)stream);
				};
			}

			template <typename Derived, typename Info>
			VIStream(InStream<Derived, C, Info> * stream) noexcept
				: m_stream(stream)
			{
				m_read = [](void * stream, C * data, size_t sz)->size_t { 
					return ((InStream<Derived, C, Info> *)stream)->read(data, sz);
				};
			}

			template <typename Derived, typename Info>
			VIStream(Streamable<Derived, Info> * stream) noexcept
				: VIStream(stream->template stream<C>())
			{
			}

			size_t $read(C* data, size_t sz)
			{
				return m_read(m_stream, data, sz);
			}

			void reset() noexcept
			{
				m_read = [](void * stream, C * data, size_t sz)->size_t { throw EofException(); };
			}
		};
		template <typename C, class Parent>
		class VOStream : public OutStream<VOStream<C, Parent>, C, StreamInfo<false, Parent>>
		{
			CLASS_HEADER(VOStream, OutStream<VOStream<C, Parent>, C, StreamInfo<false, Parent>>);
		public:
			INHERIT_COMPONENT();
		private:
			void * m_stream;
			void(*m_write)(void * stream, const C * data, size_t sz);

		public:
			VOStream() noexcept
			{
				reset();
			}
			VOStream(const VOStream&) = default;

			VOStream(FILE * file) noexcept
				: m_stream(file)
			{
				m_write = [](void * stream, C * data, size_t sz)->size_t {
					return fwrite(data, sizeof(InternalComponent), sz, (FILE*)stream);
				};
			}

			template <typename Derived, typename Info>
			VOStream(OutStream<Derived, C, Info> * stream) noexcept
				: m_stream(stream)
			{
				m_write = [](void * stream, const C * data, size_t sz) { 
					((OutStream<Derived, C, Info> *)stream)->write(data, sz);
				};
			}

			template <typename Derived, typename Info>
			VOStream(Streamable<Derived, Info> * stream) noexcept
				: VOStream(stream->template stream<C>())
			{
			}



			void $write(const C* data, size_t sz)
			{
				return m_write(m_stream, data, sz);
			}

			void reset() noexcept
			{
				m_write = [](void * stream, const C * data, size_t sz) { throw EofException(); };
			}
		};

		template <typename C, class Parent>
		class VIOStream : public InStream<VIOStream<C, Parent>, C, StreamInfo<false, OutStream<VIOStream<C, Parent>, C, StreamInfo<false, Parent>>>>
		{
		private:
			void * m_stream;
			void(* m_write)(void * stream, const C * data, size_t sz);
			size_t(* m_read)(void * stream, C * data, size_t sz);

		public:
			template <class S> VIOStream(S * stream) noexcept
				: m_stream(stream)
			{
				static_assert(IsIStream<S>::value, "S is not InStream");
				m_write = [](void * stream, const C * data, size_t sz) { ((S*)stream)->write(data, sz); };
				m_read = [](void * stream, C * data, size_t sz)->size_t { return ((S*)stream)->read(data, sz); };
			}

			VIOStream() noexcept
			{
				reset();
			}
			VIOStream(const VIOStream&) = default;

			void $write(const C* data, size_t sz)
			{
				return m_write(m_stream, data, sz);
			}
			size_t $read(C* data, size_t sz)
			{
				return m_read(m_stream, data, sz);
			}

			void reset()
			{
				m_write = [](void* stream, const C* data, size_t sz) { throw EofException(); };
				m_read = [](void* stream, C* data, size_t sz)->size_t { throw EofException(); };
			}
		};
	}
}