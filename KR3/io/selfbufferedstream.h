#pragma once

#include "../main.h"

namespace kr
{
	namespace io
	{
		template <typename Base, bool autoClose> class SelfBufferedIStream :
			public InStream<SelfBufferedIStream<Base>, typename Base::Component, StreamInfo<false, Empty>>
		{
			CLASS_HEADER(SelfBufferedIStream, InStream<SelfBufferedIStream<Base>, typename Base::Component, StreamInfo<false, Empty>>);
		public:
			static_assert(IsIStream<Base>::value, "Base is not InStream");
			INHERIT_COMPONENT();

			static constexpr int BUFFER_SIZE = 8192;
			using Text = View<Component>;
			template <size_t size> using BText = BArray<Component, size>;

			SelfBufferedIStream(nullptr_t)
			{
				m_stream = nullptr;
			}
			SelfBufferedIStream(Base* stream)
			{
				m_stream = static_cast<Base*>(stream);
				m_read = m_buffer.begin();
				m_buffer.resize(0);
			}
			SelfBufferedIStream(typename Base::StreamableBase * p) noexcept
				: SelfBufferedIStream(p->template stream<Component>())
			{
			}
			~SelfBufferedIStream() noexcept
			{
				if (autoClose) delete m_stream;
			}
			SelfBufferedIStream(const SelfBufferedIStream&) = delete;
			SelfBufferedIStream& operator = (const SelfBufferedIStream&) = delete;
			SelfBufferedIStream& operator = (nullptr_t)
			{
				m_stream = nullptr;
				return *this;
			}
			SelfBufferedIStream& operator = (SelfBufferedIStream&& _move) noexcept
			{
				m_stream = _move.m_stream;
				m_read = _move.m_read - _move.m_buffer.data() + m_buffer.data();
				m_buffer.copy(_move.m_buffer.data(), _move.m_buffer.size());
				_move.m_stream = nullptr;
				_move.m_buffer.clear();
				_move.m_read = _move.m_buffer.data();
				return *this;
			}

			void setPointer(const Component * pointer) noexcept
			{
				_assert(m_buffer.begin() <= pointer && pointer <= m_buffer.end());
				m_read = pointer;
			}
			void resetIStream(nullptr_t) noexcept
			{
				resetIStream((Base*)nullptr);
			}
			void resetIStream(Base* stream) noexcept
			{
				m_stream = static_cast<Base*>(stream);
				m_read = m_buffer.begin();
				m_buffer.resize(0);
			}
			void resetIStream(typename Base::StreamableBase* stream) noexcept
			{
				resetIStream(stream->template stream<Component>());
			}
			void close() noexcept
			{
				m_read = m_buffer.end();
				delete m_stream;
				m_stream = nullptr;
			}

			void need(size_t size)
			{
				size_t txsz = m_buffer.end() - m_read;
				while (size > txsz)
				{
					size -= txsz;
					if (m_buffer.left() == 0)
					{
						if (m_read == m_buffer.begin()) throw TooBigException();
						size_t left = m_buffer.end() - m_read;
						m_buffer.copy(m_read, left);
						m_read = m_buffer.begin();
					}

					txsz = m_stream->read(&m_buffer, m_buffer.left());
				}
			}
			Component peek()
			{
				need(1);
				return m_read[0];
			}
			Component peekAt(size_t pos = 0)
			{
				need(pos + 1);
				return m_read[pos];
			}
			Text peek(size_t count)
			{
				need(count);
				return Text(m_read, count);
			}

			void forceEnqueue(Text data) noexcept
			{
				Component * dest = forceEnqueuePrepare(data);
				data.copyTo(dest);
			}
			Component * forceEnqueuePadding(size_t size) // TooBigException
			{
				size_t left = m_buffer.left();
				if (left < size)
				{
					if (m_read - m_buffer.begin() + left < size)
					{
						throw TooBigException();
					}
					m_buffer.copy(Text(m_read, left));
					m_read = m_buffer.begin();
				}
				return m_buffer.padding(size);
			}
			Component * forceEnqueuePaddingAll(size_t *size) // TooBigException
			{
				size_t left = m_buffer.left();
				if (left == 0) throw TooBigException();
				*size = left;
				return m_buffer.padding(left);
			}
			void forceEnqueueCommit(size_t size) // NotEnoughSpaceException
			{
				m_buffer.commit(size);
			}

			intptr_t request() // TooBigException, EofException
			{
				intptr_t shifted = m_buffer.begin() - m_read;
				intptr_t left = m_buffer.end() - m_read;
				m_buffer.copy(Text(m_read, left));
				size_t remaining = m_buffer.left();
				if (remaining == 0)
					throw TooBigException();
				try
				{
					size_t readed = m_stream->read(&m_buffer, remaining);
					m_read = m_buffer.begin();
					return shifted;
				}
				catch (EofException&)
				{
					m_read = m_buffer.begin();
					throw;
				}
			}
			intptr_t request(size_t need) // TooBigException, EofException
			{
				intptr_t shifted = m_buffer.begin() - m_read;
				intptr_t left = m_buffer.end() - m_read;
				m_buffer.copy(Text(m_read, left));
				size_t remaining = m_buffer.left();
				if (remaining == 0)
					throw TooBigException();
				try
				{
					if (remaining > need)
						need = need;
					size_t readed = m_stream->read(&m_buffer, remaining);
					m_read = m_buffer.begin();
					return shifted;
				}
				catch (EofException&)
				{
					m_read = m_buffer.begin();
					throw;
				}
			}
			void skip(size_t sz) noexcept
			{
				size_t left = text().size();
				if (left >= sz)
				{
					m_read += sz;
					return;
				}
				m_read += left;
				sz -= left;
				for (;;)
				{
					request();
					left = m_buffer.size();
					if (left >= sz)
					{
						m_read = m_buffer.begin() + sz;
						return;
					}
					m_read = m_buffer.begin() + left;
					sz -= left;
				}
			}
			void skipLine() // EofException
			{
				try
				{
					Component last = skipwith_y("\r\n");
					if (last == '\r')
					{
						if (peek() == '\n') m_read++;
					}
				}
				catch (EofException&)
				{
					Component * end = m_buffer.end();
					if (end == m_read) throw;
					m_read = m_buffer.end();
				}
			}
			void skipwith(InternalComponent _chr)
			{
				Text index = text().find(_chr);
				while (index == nullptr)
				{
					m_read = m_buffer.end();
					request();
					index = m_buffer.find(_chr);
				}
				m_read = index.begin() + 1;
			}
			void skipwith(Text _str)
			{
				Text index = text().find(_str);
				while (index == nullptr)
				{
					m_read = m_buffer.end() - _str.size() + 1;
					request();
					index = m_buffer.find(_str);
				}
				m_read = index.begin() + _str.size();
			}
			Component skipwith_n(InternalComponent _chr)
			{
				Text index = text().find_n(_chr);
				while (index == nullptr)
				{
					m_read = m_buffer.end();
					request();
					index = m_buffer.find_n(_chr);
				}
				m_read = index.begin() + 1;
				return *index;
			}
			Component skipwith_y(Text _str)
			{
				Text index = text().find_y(_str);
				while (index == nullptr)
				{
					m_read = m_buffer.end();
					request();
					index = m_buffer.find_y(_str);
				}
				m_read = index.begin() + 1;
				return *index;
			}
			Component skipwith_ny(Text _str)
			{
				Text index = text().find_ny(_str);
				while (index == nullptr)
				{
					m_read = m_buffer.end();
					request();
					index = m_buffer.find_ny(_str);
				}
				m_read = index.begin() + 1;
				return *index;
			}
			Component skipspace()
			{
				return skipwith_ny(Text::WHITE_SPACE);
			}
			void find_ye(Text _str, Text &index)
			{
				try
				{
					index = index.pos_y(_str);
					while (index == nullptr)
					{
						size_t off = m_buffer.end() - m_read;
						request();
						index = (m_buffer + off).find_y(_str);
					}
				}
				catch (EofException&)
				{
					index = m_buffer.endIndex();
				}
			}
			size_t pos_y(Text _str, size_t pos)
			{
				Text index = (text() + pos).find_y(_str);
				while (index == nullptr)
				{
					size_t off = m_buffer.end() - m_read;
					request();
					index = (m_buffer + off).find_y(_str);
				}
				return index - text();
			}
			size_t pos(InternalComponent _chr, size_t pos = 0)
			{
				Text index = (text() + pos).find(_chr);
				while (index == nullptr)
				{
					size_t off = m_buffer.end() - m_read;
					request();
					index = (m_buffer + off).find(_chr);
				}
				return index - text();
			}
			size_t pos(Text _str)
			{
				_assert(_str.size() != 0);
				size_t asize = _str.size() - 1;
				Text index = text().find(_str);
				while (index == nullptr)
				{
					size_t off = m_buffer.end() - m_read;
					request();
					if (off > asize) index = m_buffer + off - asize;
					else index = m_buffer;
					index = index.find(_str);
				}
				return index - text();
			}
			size_t pos_y(Text _str)
			{
				Text index = text().find_y(_str);
				while (index == nullptr)
				{
					size_t off = m_buffer.end() - m_read;
					request();
					index = (m_buffer + off).find_y(_str);
				}
				return index.begin() - m_read;
			}
			size_t pos_ye(Text _str)
			{
				try
				{
					return pos(_str);
				}
				catch (EofException&)
				{
					return text().size();
				}
			}

			using Super::read;
			Component * read_ptr(size_t sz)
			{
				need(sz);
				const Component * out = m_read;
				m_read += sz;
				return (Component*)out;
			}
			Text read(size_t sz)
			{
				need(sz);
				Text res = Text(m_read, sz);
				m_read += sz;
				return res;
			}
			size_t readImpl(Component * dest, size_t size)
			{
				Text tx = text();
				if (!tx.empty())
				{
					size_t readed = tx.read(dest, size);
					(InternalComponent*&)m_read += readed;
					try { readed += m_stream->read(dest, size - readed); }
					catch (EofException&) {}
					return readed;
				}
				else
				{
					return m_stream->read(dest, size);
				}
			}
			Text readLine() // EofException
			{
				try
				{
					size_t readlen = pos_y("\r\n");
					const Component * next = m_read;
					Text res = Text(next, readlen);
					next += readlen;
					if (*next == '\r')
					{
						next++;
						need(readlen + 1);
						if (*next == '\n')
							next++;
					}
					else
					{
						next++;
					}
					m_read = next;
					return res;
				}
				catch (EofException&)
				{
					Text res = text();
					if (res.empty()) throw;
					m_read = res.end();
					return res;
				}
			}

			Text readto(InternalComponent _chr)
			{
				size_t readlen = pos(_chr);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				return res;
			}
			Text readto(Text _str)
			{
				size_t readlen = pos(_str);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				return res;
			}
			Text readto_y(Text _str)
			{
				size_t readlen = pos_y(_str);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				return res;
			}
			Text readto_e(InternalComponent _chr)
			{
				Text res;
				try
				{
					res = readto(_chr);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
				}
				return res;
			}
			Text readto_e(Text _str)
			{
				Text res;
				try
				{
					res = readto(_str);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
				}
				return res;
			}
			Text readto_ye(Text _str)
			{
				Text res;
				try
				{
					res = readto_y(_str);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
				}
				return res;
			}

			Text readwith(InternalComponent _chr)
			{
				size_t readlen = pos(_chr);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				m_read++;
				return res;
			}
			Text readwith(Text _str)
			{
				size_t readlen = pos(_str);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				m_read += _str.size();
				return res;
			}
			Text readwith_y(Text _str)
			{
				size_t readlen = pos_y(_str);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				m_read++;
				return res;
			}
			Text readwith_y(Text _str, Component * _finded)
			{
				size_t readlen = pos_y(_str);
				Text res = Text(m_read, readlen);
				m_read += readlen;
				*_finded = *m_read;
				m_read++;
				return res;
			}
			Text readwith_e(InternalComponent _chr)
			{
				Text res;
				try
				{
					res = readwith(_chr);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
				}
				return res;
			}
			Text readwith_e(Text _str)
			{
				Text res;
				try
				{
					res = readwith(_str);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
				}
				return res;
			}
			Text readwith_ye(Text _str)
			{
				Text res;
				try
				{
					res = readwith_y(_str);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
				}
				return res;
			}
			Text readwith_ye(Text _str, int * _finded)
			{
				Text res;
				try
				{
					readwith_y(_str, _finded);
				}
				catch (EofException&)
				{
					res = text();
					m_read = res.end();
					*_finded = eof;
				}
				return res;
			}
			bool readif(InternalComponent _chr)
			{
				need(1);
				return read() == _chr;
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
			bool hasBom()
			{
				return nextIs("\xEF\xBB\xBF");
			}

			Base* operator ->() const noexcept
			{
				return m_stream;
			}
			Base* base() const noexcept
			{
				return m_stream;
			}
			Base* detachStream() noexcept
			{
				Base * stream = m_stream;
				m_stream = nullptr;
				return stream;
			}

			Text text() const noexcept
			{
				_assert(m_buffer.end() >= m_read);
				return Text(m_read, m_buffer.end());
			}
			Text text(size_t size) noexcept
			{
				need(size);

				_assert(m_buffer.end() >= m_read);
				Component * end = m_read + size;
				_assert(m_buffer.end() >= end);
				return Text(m_read, end);
			}
			Text text(size_t size, size_t offset) noexcept
			{
				size_t endpos = offset + size;
				need(endpos);

				_assert(m_buffer.end() >= m_read);
				const Component * end = m_read + endpos;
				_assert(m_buffer.end() >= end);
				return Text(m_read + offset, end);
			}
			void clearBuffer()
			{
				m_buffer.clear();
				m_read = m_buffer.begin();
			}
			
		protected:
			Base* m_stream;
			const Component * m_read;
			BText<BUFFER_SIZE> m_buffer;
		};
	}
}
