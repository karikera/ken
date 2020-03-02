#pragma once

#include "filterstream.h"

namespace kr
{
	namespace io
	{
		template <typename Base, bool autoClose, size_t BUFFER_SIZE>
		class BufferedIStream
			:public FilterIStream < BufferedIStream<Base, autoClose, BUFFER_SIZE>, Base, autoClose, typename Base::Component>
		{
			CLASS_HEADER(BufferedIStream, FilterIStream<BufferedIStream<Base, autoClose, BUFFER_SIZE>, Base, autoClose, typename Base::Component>);
		public:
			INHERIT_COMPONENT();

		private:
			using m = memt<sizeof(Component)>;
			using TSZ = TempSzText<Component>;
			using Text = View<Component>;
			using AText = Array<Component>;

		public:
			using Super::read;
			using Super::base;

			template <typename Derived>
			struct BufferSet:public HasStreamTo<Derived, Component, MakeIterableIterator<BufferSet<Derived>, Text> >
			{
				INHERIT_ITERATOR(MakeIterableIterator<BufferSet<Derived>, Text>);
			private:
				BufferedIStream* m_this;
				Text m_buffer;

			public:
				BufferSet(BufferedIStream* _this) throws(...)
					:m_this(_this)
				{
					next();
				}

				KR_STREAM_TO(os) throws(...)
				{
					for (Text buf : *this)
					{
						os->write(buf);
					}
				}

				void next() throws(...)
				{
					Text buffer = m_this->getBuffer();
					const Component* found = static_cast<Derived*>(this)->findRule(buffer);
					if (found == nullptr)
					{
						m_buffer = buffer;
						static_cast<Derived*>(this)->refillRule();
					}
					else
					{
						m_buffer = buffer.cut(found);
						m_this = nullptr;
					}
				}
				bool isEnd() noexcept
				{
					return m_this == nullptr;
				}
				View<Component> value() noexcept
				{
					return m_buffer;
				}
			};

			BufferedIStream(Base* p) noexcept 
				: Super(p)
			{
				m_filled = m_read = m_buffer;
			}
			BufferedIStream(nullptr_t) noexcept 
				: BufferedIStream((Base*)nullptr)
			{
				m_filled = m_read = m_buffer;
			}
			BufferedIStream(typename Base::StreamableBase* p) noexcept
				: BufferedIStream(p->template stream<Component>())
			{
			}

			void resetStream(Base * p) noexcept
			{
				m_filled = m_read = m_buffer;
				Super::resetStream(p);
			}
			void resetStream(nullptr_t) noexcept
			{
				resetStream((Base*)nullptr);
			}
			void resetStream(typename Base::StreamableBase* p) noexcept
			{
				resetStream(p->template stream<Component>());
			}

			void refill() throws(EofException)
			{
				m_filled = m_read = m_buffer;
				m_filled += base()->read(m_buffer, BUFFER_SIZE);
			}
			void remainFill() throws(EofException)
			{
				size_t remaining = m_filled - m_read;
				if (remaining == 0)
				{
					return refill();
				}
				if (m_read == m_buffer)
				{
					m_filled += base()->read(m_filled, BUFFER_SIZE - remaining);
				}
				else
				{
					mema::copy(m_buffer, m_read, remaining);
					m_read = m_buffer;
					m_filled = m_buffer + remaining;
					m_filled += base()->read(m_filled, BUFFER_SIZE - remaining);
				}
			}
			void clearBuffer() noexcept
			{
				m_filled = m_read = m_buffer;
			}
//
//#define BUFFERSET(name, find, refill) \
//	struct name: BufferSet<Derived>{\
//		const Component* findRule(Text text) throws(...){ \
//			find;\
//		} \
//		void refillRule() throws(...){ \
//			refill;\
//		} \
//};

			InternalComponent peek() throws(EofException)
			{
				for (;;)
				{
					if (m_filled > m_read)
					{
						return *m_read;
					}

					refill();
				}
			}
			Ref peek(size_t count) throws(EofException, TooBigException)
			{
				if (count > BUFFER_SIZE) throw TooBigException();
				for (;;)
				{
					if (m_filled >= m_read + count)
					{
						return Ref(m_read, count);
					}

					remainFill();
				}
			}
			InternalComponent peekAt(size_t idx) throws(EofException, TooBigException)
			{
				return peek(idx + 1).get(idx);
			}
			bool nextIs(const Component & comp)
			{
				try
				{
					if (comp != peek()) return false;
					skip(1);
					return true;
				}
				catch (EofException&)
				{
					return false;
				}
			}
			bool nextIs(Ref comps)
			{
				try
				{
					if (comps != peek(comps.size())) return false;
					skip(comps.size());
					return true;
				}
				catch (EofException&)
				{
					return false;
				}
			}
			void must(const Component & comp) throws(InvalidSourceException)
			{
				if (!nextIs(comp)) throw InvalidSourceException();
			}
			void must(Ref comps) throws(InvalidSourceException)
			{
				TmpArray<Component> tmp((size_t)0, comps.size());
				try
				{
					tmp << read(comps.size());
					if (tmp != comps) throw InvalidSourceException();
				}
				catch (EofException&)
				{
					throw InvalidSourceException();
				}
			}

			void skip(size_t sz = 1) throws(EofException)
			{
				Component * line;
				for (;;)
				{
					line = m_read + sz;
					if (line <= m_filled) break;
					sz = line - m_filled;
					refill();
				}

				m_read = line;
			}
			size_t skipAll()
			{
				size_t size = m_filled - m_read;
				size += base()->skipAll();
				clearBuffer();
				return size;
			}

			template <typename _Derived, typename _Info>
			size_t readLine(OutStream<_Derived, Component, _Info> * dest) throws(EofException, NotEnoughSpaceException)
			{
				size_t readed = 0;
				Component * line;
				for (;;)
				{
					size_t remain = m_filled - m_read;
					line = m::find(m_read, (Component)'\n', remain);
					if (line != nullptr) break;
					if (remain != 0)
					{
						Component * last = m_filled;
						Component lastchr = last[-1];
						if (lastchr == (Component)'\r')
						{
							remain--;
							m_filled = m_buffer + 1;
							dest->write(m_read, remain);
							*m_buffer = lastchr;
						}
						else
						{
							m_filled = m_buffer;
							dest->write(m_read, remain);
						}
						readed += remain;
						m_read = m_buffer;
					}

					try
					{
						size_t res = base()->read(m_filled, m_buffer + BUFFER_SIZE - m_filled);
						m_filled += res;
					}
					catch (EofException&)
					{
						if(readed == 0) throw;
						return readed;
					}
				}

				size_t len = line - m_read;
				size_t copylen = len;
				if (line != m_read && line[-1] == '\r') copylen--;
				dest->write(m_read, copylen);
				m_read += len + 1;
				return readed + len;
			}
			TSZ readLine() throws(EofException)
			{
				TSZ tsz;
				readLine(&tsz);
				return tsz;
			}
			size_t skipLine() throws(EofException)
			{
				size_t readed = 0;
				Component * line;
				for (;;)
				{
					size_t remain = m_filled - m_read;
					line = m::find(m_read, (Component)'\n', remain);
					if (line != nullptr) break;
					if (remain != 0)
					{
						Component * last = m_filled;
						Component lastchr = last[-1];
						if (lastchr == (Component)'\r')
						{
							remain--;
							m_filled = m_buffer + 1;
							*m_buffer = lastchr;
						}
						else
						{
							m_filled = m_buffer;
						}
						readed += remain;
						m_read = m_buffer;
					}

					try
					{
						size_t res = base()->read(m_filled, m_buffer + BUFFER_SIZE - m_filled);
						m_filled += res;
					}
					catch (EofException&)
					{
						if (readed == 0) throw;
						return readed;
					}
				}

				size_t len = line - m_read;
				m_read += len + 1;
				return readed + len;
			}

			template <typename _Derived, typename _Info, typename LAMBDA>
			size_t readto_L(OutStream<_Derived, Component, _Info> * dest, LAMBDA &&lambda) throws(NotEnoughSpaceException, EofException)
			{
				size_t totalReaded = 0;
				Component * line;
				for (;;)
				{
					size_t size = m_filled - m_read;
					if (size != 0)
					{
						line = (Component*)lambda(View<Component>(m_read, size));
						if (line != nullptr) break;
						dest->write(m_read, size);
						totalReaded += size;
					}

					refill();
				}

				size_t len = line - m_read;
				dest->write(m_read, len);
				m_read = line;
				return totalReaded + len;
			}
			template <typename LAMBDA>
			TSZ readto_L(LAMBDA &&lambda) throws(EofException)
			{
				TSZ tsz;
				readto_L(&tsz, lambda);
				return tsz;
			}
			template <typename LAMBDA>
			size_t skipto_L(LAMBDA &&lambda) throws(EofException)
			{
				size_t readlen = 0;
				Component * line;
				for (;;)
				{
					size_t size = m_filled - m_read;
					if (size != 0)
					{
						line = (Component*)lambda(View<Component>(m_read, size));
						if (line != nullptr) break;
						Component * last = m_filled;
						readlen += size;
					}

					refill();
				}

				size_t copylen = line - m_read;
				m_read = line;
				return readlen + copylen;
			}

			template <typename _Derived, typename _Info>
			size_t readto(OutStream<_Derived, Component, _Info> * dest, const Component &needle) throws(EofException)
			{
				return readto_L(dest, [&](Ref text) {
					return text.find(needle);
				});
			}
			TSZ readto(const Component &chr) throws(EofException)
			{
				TSZ tsz;
				return readto(&tsz, chr);
			}
			size_t skipto(const Component &needle) throws(EofException)
			{
				return skipto_L([&](Ref text) {
					return text.find(needle);
				});
			}

			template <typename _Derived, typename _Info>
			size_t readto(OutStream<_Derived, Component, _Info> * dest, Ref needle) throws(EofException, TooBigException)
			{
				size_t totalReaded = 0;
				size_t needlesize = needle.size();
				if (needlesize >= BUFFER_SIZE / 2) throw TooBigException();
				Component * beg = m_read;
				try
				{
					Component * finded = mem::find_callback([&]{
						while (m_read == m_filled)
						{
							size_t readed = m_filled - beg;
							if (readed > needlesize)
							{
								readed -= needlesize;
								dest->write(beg, readed);
								totalReaded += readed;
								m_read = m_filled - needlesize;
							}
							remainFill();
							beg = m_buffer;
						}
						return m_read++;
					}, needle.data(), needlesize);

					m_read = finded - needlesize;
					size_t readsize = finded - beg;
					dest->write(beg, readsize);
					return totalReaded + readsize;
				}
				catch (EofException&)
				{
					if (totalReaded == 0) throw;
					dest->write(beg, m_filled - beg);
					clearBuffer();
					return totalReaded;
				}
			}
			AText readto(Ref needle) throws(EofException, TooBigException)
			{
				AText text;
				readto(&text, needle);
				return text;
			}
			size_t skipto(Ref needle) throws(EofException, TooBigException)
			{
				size_t totalReaded = 0;
				size_t needlesize = needle.size();
				if (needlesize >= BUFFER_SIZE / 2) throw TooBigException();
				Component * beg = m_read;
				try
				{
					Component * finded = mem::find_callback([&]{
						while (m_read == m_filled)
						{
							size_t readed = m_filled - beg;
							if (readed > needlesize)
							{
								totalReaded += readed - needlesize;
								m_read = m_filled - needlesize;
							}
							remainFill();
							beg = m_buffer;
						}
						return m_read++;
					}, needle.data(), needlesize);

					m_read = finded - needlesize;
					return totalReaded + finded - beg;
				}
				catch (EofException&)
				{
					if (totalReaded == 0) throw;
					clearBuffer();
					return totalReaded;
				}
			}

			template <typename _Derived, typename _Info>
			size_t readwith(OutStream<_Derived, Component, _Info> * dest, const Component &chr) throws(EofException, NotEnoughSpaceException)
			{
				size_t sz = readto(dest, chr);
				skip(1);
				return sz;
			}
			TSZ readwith(const Component &chr) throws(EofException)
			{
				TSZ tsz;
				readwith(&tsz, chr);
				return tsz;
			}
			size_t skipwith(const Component &chr) throws(EofException)
			{
				size_t sz = skipto(chr);
				skip(1);
				return sz;
			}

			template <typename _Derived, typename _Info>
			size_t readwith(OutStream<_Derived, Component, _Info> * dest, Ref needle) throws(EofException, NotEnoughSpaceException, TooBigException)
			{
				size_t sz = readto(dest, needle);
				skip(needle.size());
				return sz;
			}
			TSZ readwith(Ref needle) throws(EofException, TooBigException)
			{
				TSZ tsz;
				return readwith(&tsz, needle);
			}
			size_t skipwith(Ref needle) throws(EofException, TooBigException)
			{
				size_t sz = skipto(needle);
				skip(needle.size());
				return sz;
			}

			template <typename _Derived, typename _Info>
			size_t readto_y(OutStream<_Derived, Component, _Info> * dest, Ref chr) throws(NotEnoughSpaceException, EofException, TooBigException)
			{
				return readto_L(dest, [chr](Ref text) {
					return text.find_y(chr);
				});
			}
			TSZ readto_y(Ref chr) throws(EofException, TooBigException)
			{
				TSZ tsz;
				readto_y(&tsz, chr);
				return tsz;
			}
			size_t skipto_y(Ref chr) throws(EofException, TooBigException)
			{
				return skipto_L([chr](Ref text) {
					return text.find_y(chr);
				});
			}

			template <typename _Derived, typename _Info>
			size_t readto_n(OutStream<_Derived, Component, _Info> * dest, const Component & chr) throws(NotEnoughSpaceException, EofException)
			{
				return readto_L(dest, [&](Ref text) {
					return text.find_n(chr);
				});
			}
			TSZ readto_n(const Component &chr) throws(EofException)
			{
				TSZ tsz;
				return readto_n(&tsz, chr);
			}
			size_t skipto_n(Component chr) throws(EofException)
			{
				return skipto_L([chr](Ref text) {
					return text.find_n(chr);
				});
			}

			template <typename _Derived, typename _Info>
			size_t readto_ny(OutStream<_Derived, Component, _Info> * dest, Ref chr) throws(NotEnoughSpaceException, EofException, TooBigException)
			{
				return readto_L(dest, [chr](Ref text) {
					return text.find_ny(chr);
				});
			}
			TSZ readto_ny(Ref chr) throws(EofException, TooBigException)
			{
				TSZ tsz;
				readto_ny(&tsz, chr);
				return tsz;
			}
			size_t skipto_ny(Ref chr) throws(EofException, TooBigException)
			{
				return skipto_L([chr](Ref text) {
					return text.find_ny(chr);
				});
			}

			View<Component> getBuffer() noexcept
			{
				return View<Component>(m_read, m_filled);
			}
			size_t $read(Component * dest, size_t need) throws(EofException)
			{
				{
					Component* to = m_read + need;
					if (m_filled >= to)
					{
						mema::copy(dest, m_read, need);
						m_read = to;
						return need;
					}
				}

				size_t left = m_filled - m_read;
				mema::copy(dest, m_read, left);
				dest += left;
				size_t neednext = need - left;
				if (neednext >= BUFFER_SIZE)
				{
					m_filled = m_read = m_buffer;
					size_t readed = base()->read(dest, BUFFER_SIZE);
					return left + readed;
				}

				try
				{
					size_t readed = base()->read(m_buffer, BUFFER_SIZE);
					if (readed >= neednext)
					{
						mema::copy(dest, m_buffer, neednext);
						m_read = m_buffer + neednext;
						m_filled = m_buffer + readed;
						return need;
					}
					else
					{
						mema::copy(dest, m_buffer, readed);
						m_filled = m_read = m_buffer;
						return left + readed;
					}
				}
				catch (EofException&)
				{
					if (left == 0) throw;
					return left;
				}
			}

		private:
			Component m_buffer[BUFFER_SIZE];
			Component* m_read;
			Component* m_filled;
		};
		
		template <typename Base, bool endFlush, bool autoClose, size_t BUFFER_SIZE>
		class BufferedOStream 
			:public FilterOStream<BufferedOStream<Base, endFlush, autoClose, BUFFER_SIZE>, Base, autoClose>
		{
			CLASS_HEADER(BufferedOStream, FilterOStream<BufferedOStream<Base, endFlush, autoClose, BUFFER_SIZE>, Base, autoClose>);
		public:
			INHERIT_COMPONENT();

			using Super::base;

#pragma warning(push)
#pragma warning(disable:26495)
			BufferedOStream(Base* p) noexcept 
				: Super(p)
			{
				m_filled = m_buffer;
			}
#pragma warning(pop)
			BufferedOStream(nullptr_t) noexcept
				: BufferedOStream((Base*)nullptr)
			{
			}
			BufferedOStream(typename Base::StreamableBase * p) noexcept
				: BufferedOStream(p->template stream<Component>())
			{
			}
			~BufferedOStream() noexcept
			{
				if (endFlush) flush();
			}

			BufferedOStream & operator = (const BufferedOStream &) noexcept = delete;

			void resetStream(nullptr_t) noexcept
			{
				m_filled = m_buffer;
				Super::resetStream(nullptr);
			}
			void resetStream(Base * p) noexcept
			{
				m_filled = m_buffer;
				Super::resetStream(p);
			}
			void resetStream(typename Base::StreamableBase * p) noexcept
			{
				m_filled = m_buffer;
				Super::resetStream(p);
			}
			void $write(const Component* data, size_t sz)
			{
				const InternalComponent* src = (InternalComponent*)data;
				size_t left = m_buffer + BUFFER_SIZE - m_filled;
				if (left > sz)
				{
					mema::copy(m_filled, src, sz);
					m_filled += sz;
					return;
				}

				const InternalComponent* srcto = src + sz;
				mema::copy(m_filled, src, left);
				src += left;

				base()->write(m_buffer, BUFFER_SIZE);
				while (src + BUFFER_SIZE <= srcto)
				{
					base()->write(src, BUFFER_SIZE);
					src += BUFFER_SIZE;
				}
				left = srcto - src;
				mema::copy(m_buffer, src, left);
				m_filled = m_buffer + left;
				src = srcto;
			}
			void flush()
			{
				Base * s = base();
				if (s == nullptr) return;
				InternalComponent* ptr = (InternalComponent*)m_buffer;
				s->write((Component*)m_buffer, m_filled - ptr);
				m_filled = m_buffer;
			}
			void close() noexcept
			{
				if (endFlush) flush();
				Super::close();
			}

		private:
			InternalComponent m_buffer[BUFFER_SIZE];
			InternalComponent* m_filled;
		};

	}
}
