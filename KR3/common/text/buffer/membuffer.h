#pragma once

namespace kr
{
	template <typename C>
	class ReverseIterable
	{
	private:
		C * m_beg;
		C * m_end;

	public:
		class Iterator :public TIterator<Iterator, C>
		{
		private:
			using TIterator<Iterator, C>::m_pt;

		public:
			INHERIT_ITERATOR(TIterator<Iterator, C>);

			intptr_t operator -(const Iterator & o) const noexcept
			{
				return o.m_pt - m_pt;
			}
			Iterator& operator += (intptr_t v) noexcept
			{
				m_pt -= v;
				return *this;
			}
			Iterator& operator -= (intptr_t v) noexcept
			{
				m_pt += v;
				return *this;
			}
			Iterator& operator ++() noexcept
			{
				m_pt--;
				return *this;
			}
			Iterator& operator --() noexcept
			{
				m_pt++;
				return *this;
			}
			C& operator [](size_t idx) const noexcept
			{
				return m_pt[-(intptr_t)idx];
			}
		};

		ReverseIterable(C* beg, C* end) noexcept
		{
			m_beg = end - 1;
			m_end = beg - 1;
		}
		Iterator begin() const noexcept
		{
			return m_beg;
		}
		Iterator end() const noexcept
		{
			return m_end;
		}
	};

	namespace buffer
	{
		template <typename Derived, typename Info>
		class MemBuffer :public Info
		{
			CLASS_HEADER(MemBuffer, Info);
		public:
			INHERIT_COMPONENT();

			using Super::Super;
			using Info::accessable;
			using Info::szable;

			ComponentRef* data() noexcept
			{
				return static_cast<Derived*>(this)->begin();
			}
			ComponentRef* begin() noexcept
			{
				return static_cast<Derived*>(this)->begin();
			}
			ComponentRef* end() noexcept
			{
				return static_cast<Derived*>(this)->end();
			}
			const ComponentRef* data() const noexcept
			{
				return static_cast<const Derived*>(this)->begin();
			}
			const Component * begin() const noexcept
			{
				return static_cast<const Derived*>(this)->begin();
			}
			const Component * end() const noexcept
			{
				return static_cast<const Derived*>(this)->end();
			}
			size_t size() const noexcept
			{
				return static_cast<const Derived*>(this)->size();
			}
			bool empty() const noexcept
			{
				static_assert(&MemBuffer::empty != &Derived::empty, "This function need override");
				return static_cast<const Derived*>(this)->empty();
			}
			template <typename T> size_t sizeAs() const noexcept
			{
				static_assert(is_same<T, Component>::value, "Need same type");
				return size();
			}

			bool operator ==(const MemBuffer& other) const
			{
				size_t tsize = size();
				size_t osize = other.size();
				if (tsize != osize) return false;
				return mem::equals(begin(), other.begin(), tsize);
			}
			bool operator !=(const MemBuffer& other) const
			{
				return !(*this == other);
			}
			bool equals(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = _v.size();
				if (_len != _len2) return false;
				else return memm::equals(begin(), _v.begin(), _len);
			}
			size_t hash() const noexcept
			{
				return mem::hash(begin(), size() * sizeof(ComponentRef));
			}
			Ref beginIndex() const noexcept
			{
				return Ref(begin(), begin());
			}
			Ref endIndex() const noexcept
			{
				return Ref(end(), end());
			}
			size_t copyTo(Component * dest) const
			{
				mema::subs_copy((InternalComponent*)dest, (InternalComponent*)begin(), size());
				return size();
			}
			size_t sizeBytes() const noexcept
			{
				return size() * sizeof(InternalComponent);
			}
			bool contains_ptr(const ComponentRef * _v) const noexcept
			{
				return begin() <= _v && _v <= end();
			}
			template <class _Derived, bool a, bool b, class _Parent>
			bool contains_ptr(const Bufferable<_Derived, BufferInfo<Component, true, a, b, _Parent>>& _v) const noexcept
			{
				return contains_ptr(_v.begin());
			}
			Ref cut(const ComponentRef* end) const noexcept
			{
				_assert(contains_ptr(end));
				return Ref(begin(), end);
			}
			Ref cut(Ref _v) const noexcept
			{
				return cut(_v.begin());
			}
			Ref cut(size_t _len) const noexcept
			{
				return cut(mint((InternalComponentRef*)begin() + _len, (InternalComponentRef*)end()));
			}

			Ref subarr(size_t _left, size_t _count) const noexcept
			{
				return Ref((InternalComponentRef*)begin() + _left, (InternalComponentRef*)end()).cut(_count);
			}
			Ref subarr(size_t _left) const noexcept
			{
				return Ref((InternalComponentRef*)begin() + _left, (InternalComponentRef*)end());
			}
			Ref subarray(size_t _left) const noexcept
			{
				return Ref((InternalComponentRef*)begin() + _left, (InternalComponentRef*)end());
			}
			Ref subarray(size_t _left, size_t _right) const noexcept
			{
				_assert(_left <= _right);
				return Ref((InternalComponentRef*)begin() + _left, (InternalComponentRef*)begin() + _right);
			}
			Ref offsetBytes(size_t bytes) const noexcept
			{
				return Ref((InternalComponentRef*)((uint8_t*)begin() + bytes), (InternalComponentRef*)end());
			}

			bool startsWith(Ref _v) const noexcept
			{
				if (_v.size() > size())
					return false;
				return cut(_v.size()) == _v;
			}
			bool endsWith(Ref _v) const noexcept
			{
				if (_v.size() > size())
					return false;
				return subarr(size() - _v.size()) == _v;
			}
			bool startsWith_i(Ref _v) const noexcept
			{
				if (_v.size() > size())
					return false;
				return cut(_v.size()).equals_i(_v);
			}
			bool endsWith_i(Ref _v) const noexcept
			{
				if (_v.size() > size())
					return false;
				return subarr(size() - _v.size()).equals_i(_v);
			}
			bool startsWith_y(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				if (empty())
					return false;
				return memm::find(_v.data(), front(), _v.size()) != nullptr;
			}
			bool endsWith_y(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				if (empty())
					return false;
				return memm::find(_v.data(), back(), _v.size()) != nullptr;
			}
			bool startsWith(const InternalComponent &_v) const noexcept
			{
				if (empty())
					return false;
				return front() == _v;
			}
			bool endsWith(const InternalComponent &_v) const noexcept
			{
				if (empty())
					return false;
				return back() == _v;
			}
			bool startsWith_i(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				if (empty())
					return false;
				return memm::equals_i(front(), _v);
			}
			bool endsWith_i(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				if (empty())
					return false;
				return memm::equals_i(back(), _v);
			}


			InternalComponentRef& operator [](size_t i) noexcept
			{
				_assert(i < size());
				return begin()[i];
			}
			const InternalComponentRef& operator [](size_t i) const noexcept
			{
				_assert(i < size());
				return begin()[i];
			}
			InternalComponentRef& operator *()
			{
				return *begin();
			}
			const InternalComponentRef& operator *() const
			{
				return *begin();
			}
			const InternalComponentRef& get(size_t index) const noexcept
			{
				return begin()[index];
			}
			InternalComponentRef& get(size_t index) noexcept
			{
				return begin()[index];
			}
			void set(size_t index, const InternalComponent& src) noexcept
			{
				_assert(index < size());
				begin()[index] = src;
			}

			bool contains(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::contains(data(), _v, size());
			}
			bool contains(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = _v.size();
				if (_len < _len2) return false;
				return memm::find(data(), _v.data(), _len, _len2) != nullptr;
			}
			Ref find_n(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_n(begin(), _v, size()), end());
			}
			Ref find_ne(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_ne(begin(), _v, size()), end());
			}
			Ref find_ny(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_ny(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find_nr(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_nr(begin(), _v, size()), end());
			}
			Ref find_nry(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_nry(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find_nye(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_nye(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find(const InternalComponent &needle) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find(begin(), needle, size()), end());
			}
			Ref find(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = _v.size();
				if (_len < _len2)
					return nullptr;
				return Ref(memm::find(begin(), _v.begin(), _len, _len2), end());
			}
			Ref find_e(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_e(begin(), _v, size()), end());
			}
			Ref find_e(Ref _v) const noexcept
			{
				Ref finded = find(_v);
				if (finded == nullptr)
					return endIndex();
				return finded;
			}
			Ref find_y(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_y(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find_ye(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_ye(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find_r(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_r(begin(), _v, size()), end());
			}
			Ref find_r(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = _v.size();
				if (_len < _len2)
					return nullptr;
				return Ref(memm::find_r(begin(), _v.begin(), _len, _len2), end());
			}
			Ref find_ry(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_ry(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find_rye(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_rye(begin(), _v.begin(), size(), _v.size()), end());
			}
			Ref find_re(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(memm::find_re(begin(), _v, size()), end());
			}
			Ref find_re(Ref _v) const noexcept
			{
				Ref finded = find_r(_v);
				if (finded == nullptr)
					return Ref(begin()-1, end());
				return finded;
			}
			template <typename LAMBDA>
			Ref find_L(const LAMBDA & lambda) const noexcept
			{
				InternalComponentRef * e = end();
				InternalComponentRef * p = begin();
				for (; p != e; p++)
				{
					if (lambda(*p)) return Ref(p, end());
				}
				return Ref(nullptr, end());
			}

			Ref end_find_r(const InternalComponent &needle) noexcept
			{
				KR_DEFINE_MMEM();
				const Component * _end = memm::find_r(begin(), needle, size());
				if (_end == nullptr)
				{
					Ref out;
					out.setEnd(nullptr);
					return out;
				}
				return Ref(begin(), _end + 1);
			}
			Ref end_find_re(const InternalComponent &needle) noexcept
			{
				KR_DEFINE_MMEM();
				return Ref(begin(), memm::find_re(begin(), needle, size()) + 1);
			}
			Ref end_find_r(Ref needle) noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = needle.size();
				if (_len < _len2)
				{
					Ref out;
					out.setEnd(nullptr);
					return out;
				}
				const Component * _end = memm::find_r(begin(), needle.begin(), _len, _len2);
				if (_end == nullptr)
				{
					Ref out;
					out.setEnd(nullptr);
					return out;
				}
				return Ref(begin(), _end + needle.size());
			}
			Ref end_find_re(Ref needle) noexcept
			{
				Ref finded = end_find_r(needle);
				if (finded.end() == nullptr)
					return Ref(begin(), begin());
				return finded;
			}

			size_t pos(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos(begin(), _v, size());
			}
			size_t pos(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t len2 = _v.size();
				if (size() < len2) return -1;
				return memm::pos(data(), _v.data(), size(), len2);
			}
			size_t pos_y(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_y(data(), _v.data(), size(), _v.size());
			}
			size_t pos_r(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_r(data(), _v, size());
			}
			size_t pos_ry(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_ry(data(), _v.data(), size(), _v.size());
			}
			size_t pos_re(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_re(data(), _v, size());
			}
			size_t pos_n(const InternalComponent & _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_n(data(), _v, size());
			}
			size_t pos_nr(const InternalComponent & _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_nr(data(), _v, size());
			}
			size_t pos_nry(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::pos_nry(data(), _v.data(), size(), _v.size());
			}
			size_t count(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::count(data(), _v, size());
			}
			size_t count_y(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return memm::count_y(data(), _v.data(), size(), _v.size());
			}

			const InternalComponentRef& front() const noexcept
			{
				return data()[0];
			}
			InternalComponentRef& front() noexcept
			{
				return data()[0];
			}
			const InternalComponentRef& back() const noexcept
			{
				return end()[-1];
			}
			InternalComponentRef& back() noexcept
			{
				return end()[-1];
			}

			template <class ODerived, typename _Info>
			size_t writeTo(OutStream<ODerived, Component, _Info> * os) const
			{
				size_t sz = size();
				os->write(data(), sz);
				return sz;
			}

			template <typename NewComponent>
			View<NewComponent> cast() const noexcept
			{
				return View<NewComponent>((NewComponent*)begin(), (NewComponent*)end());
			}

			template <class Converter, class _Derived, typename _Info>
			void replace(OutStream<_Derived, typename Converter::Component, _Info> * _out, Ref _tar, View<typename Converter::Component> _to) const // NotEnoughSpaceException
			{
				Ref reader = *static_cast<const Bufferable<Derived, Info>*>(this);

				for (;;)
				{
					Ref finded = reader.readwith(_tar);
					if (finded == nullptr)
					{
						_out->print(Converter(reader));
						return;
					}
					_out->print(Converter(finded));
					_out->write(_to);
				}
			}
			template <class Converter, class _Derived, typename _Info>
			void replace(OutStream<_Derived, typename Converter::Component, _Info> * _out, const InternalComponent &_tar, const internal_component_t<typename Converter::Component> &_to) const // NotEnoughSpaceException
			{
				Ref reader = *static_cast<const Bufferable<Derived, Info>*>(this);

				for (;;)
				{
					Ref finded = reader.readwith(_tar);
					if (finded == nullptr)
					{
						_out->print(Converter(reader));
						return;
					}
					_out->print(Converter(finded));
					_out->write(_to);
				}
			}
			template <class _Derived, typename _Info>
			void replace(OutStream<_Derived, Component, _Info> * _out, Ref _tar, Ref _to) const // NotEnoughSpaceException
			{
				replace<Ref>(_out, _tar, _to);
			}
			template <class _Derived, typename _Info>
			void replace(OutStream<_Derived, Component, _Info> * _out, const InternalComponent &_tar, const InternalComponent &_to) const // NotEnoughSpaceException
			{
				replace<Ref>(_out, _tar, _to);
			}

			class SplitIterator
			{
			private:
			public:
				const Component * m_ref;
				const Component * m_next;
				const Component * m_end;
				const Component * m_done;
				Component m_chr;

			public:
				SplitIterator() = default;
				SplitIterator(const This * _this, const meta::types<Component> & params) noexcept
					: m_chr(params.template get<0>())
				{
					m_ref = _this->begin();
					m_end = _this->end();
					m_next = _this->find_e(m_chr).begin();
					m_done = m_end + 1;
				}
				bool isEnd() const noexcept
				{
					return m_ref == m_done;
				}
				SplitIterator& operator ++() noexcept
				{
					m_ref = m_next;
					m_ref++;
					if (m_ref == m_done) return *this;
					m_next = Ref(m_ref, m_end).find_e(m_chr).begin();
					return *this;
				}
				SplitIterator operator ++(int) noexcept
				{
					SplitIterator old = *this;
					++*this;
					return old;
				}
				Ref operator *() const noexcept
				{
					return Ref(m_ref, m_next);
				}
			};
			
			class TextSplitIterator
			{
			private:
				const Component * m_ref;
				const Component * m_next;
				const Component * m_end;
				const Component * m_done;
				Ref m_chr;

			public:
				TextSplitIterator() = default;
				TextSplitIterator(const This * _this, const meta::types<Ref> & params) noexcept
					: m_chr(params.template get<0>())
				{
					m_ref = _this->begin();
					m_end = _this->end();
					m_next = _this->find_e(m_chr).begin();
					m_done = m_end + m_chr.size();
				}
				bool isEnd() const noexcept
				{
					return m_ref == m_done;
				}
				TextSplitIterator& operator ++() noexcept
				{
					m_ref = m_next;
					m_ref += m_chr.size();
					if (m_ref == m_done) return *this;
					m_next = Ref(m_ref, m_end).find_e(m_chr).begin();
					return *this;
				}
				TextSplitIterator operator ++(int) noexcept
				{
					TextSplitIterator old = *this;
					++*this;
					return old;
				}
				Ref operator *() const noexcept
				{
					return Ref(m_ref, m_next);
				}
			};
			
			class ReverseSplitIterator
			{
			private:
				const Component * m_ref;
				const Component * m_next;
				const Component * m_begin;
				const Component * m_done;
				Component m_chr;

			public:
				ReverseSplitIterator() = default;
				ReverseSplitIterator(const This * _this, const meta::types<Component> & params) noexcept 
					: m_chr(params.template get<0>())
				{
					m_begin = _this->begin();
					m_ref = _this->end();
					m_next = _this->find_r(m_chr).begin();
					if (m_next == nullptr) m_next = m_begin;
					m_done = m_begin - 1;
				}
				bool isEnd() const noexcept
				{
					return m_ref == m_done;
				}
				ReverseSplitIterator& operator ++() noexcept
				{
					m_ref = m_next;
					m_ref--;
					if (m_ref == m_done) return *this;
					m_next = Ref(m_begin, m_ref).find_r(m_chr).begin();
					if (m_next == nullptr) m_next = m_begin;
					return *this;
				}
				ReverseSplitIterator operator ++(int) noexcept
				{
					SplitIterator old = *this;
					++*this;
					return old;
				}
				Ref operator *() const noexcept
				{
					return Ref(m_next, m_ref);
				}
			};

			class ReverseTextSplitIterator
			{
			private:
				const Component * m_ref;
				const Component * m_next;
				const Component * m_begin;
				const Component * m_done;
				Ref m_chr;

			public:
				ReverseTextSplitIterator() = default;
				ReverseTextSplitIterator(const This * _this, const meta::types<Ref> & params) noexcept
					: m_chr(params.template get<0>())
				{
					m_begin = _this->begin();
					m_ref = _this->end();
					m_next = _this->find_r(m_chr).begin();
					if (m_next == nullptr) m_next = m_begin;
					else m_next += m_chr.size();
					m_done = m_begin - m_chr.size();
				}
				bool isEnd() const noexcept
				{
					return m_ref == m_done;
				}
				ReverseTextSplitIterator& operator ++() noexcept
				{
					m_ref = m_next;
					m_ref -= m_chr.size();
					if (m_ref == m_done) return *this;
					m_next = Ref(m_begin, m_ref).find_r(m_chr).begin();
					if (m_next == nullptr) m_next = m_begin;
					else m_next += m_chr.size();
					return *this;
				}
				ReverseTextSplitIterator operator ++(int) noexcept
				{
					TextSplitIterator old = *this;
					++*this;
					return old;
				}
				Ref operator *() const noexcept
				{
					return Ref(m_next, m_ref);
				}
			};

			class LoopIterator
			{
			private:
				InternalComponent * m_ptr;
				InternalComponent * m_end;
				InternalComponent * m_ptr2;
				InternalComponent * m_end2;

			public:
				LoopIterator() = default;
				LoopIterator(const This * _this, const meta::types<InternalComponent*, InternalComponent *, InternalComponent *, InternalComponent *> & params) noexcept
				{
					m_ptr = params.template get<0>();
					m_end = params.template get<1>();
					m_ptr2 = params.template get<2>();
					m_end2 = params.template get<3>();
					if (m_ptr == m_end)
					{
						m_ptr = m_ptr2;
						m_end = m_end2;
						m_ptr2 = nullptr;
						if (m_ptr == m_end)
						{
							m_ptr = m_ptr2;
						}
					}
				}
				bool isEnd() const noexcept
				{
					return m_ptr == nullptr;
				}
				LoopIterator& operator ++() noexcept
				{
					m_ptr++;
					if (m_ptr == m_end)
					{
						m_ptr = m_ptr2;
						m_end = m_end2;
						m_ptr2 = nullptr;
						return *this;
					}
					return *this;
				}
				LoopIterator operator ++(int) noexcept
				{
					LoopIterator old = *this;
					++*this;
					return old;
				}
				InternalComponent & operator *() const noexcept
				{
					return *m_ptr;
				}
			};
			using SplitIterable = Iterable<This, SplitIterator, InternalComponent>;
			using TextSplitIterable = Iterable<This, TextSplitIterator, Ref>;
			using ReverseSplitIterable = Iterable<This, ReverseSplitIterator, InternalComponent>;
			using ReverseTextSplitIterable = Iterable<This, ReverseTextSplitIterator, Ref>;
			using LoopIterable = Iterable<This, LoopIterator, InternalComponent*, InternalComponent*, InternalComponent*, InternalComponent*>;

			SplitIterable splitIterable(const InternalComponent &chr) const noexcept
			{
				return SplitIterable(this, chr);
			}
			TextSplitIterable splitIterable(Ref chr) const noexcept
			{
				return TextSplitIterable(this, chr);
			}
			ReverseSplitIterable reverseSplitIterable(const InternalComponent &chr) const noexcept
			{
				return ReverseSplitIterable(this, chr);
			}
			ReverseTextSplitIterable reverseSplitIterable(Ref chr) const noexcept
			{
				return ReverseTextSplitIterable(this, chr);
			}
			LoopIterable loopIterable(size_t offset) noexcept
			{
				InternalComponent * _begin = begin();
				InternalComponent * startAt = _begin + offset;
				return LoopIterable(this, startAt, end(), _begin, startAt);
			}
			LoopIterable loopIterable(size_t offset, size_t length) noexcept
			{
				InternalComponent * _begin = begin();
				InternalComponent * startAt = _begin + offset;
				InternalComponent * endAt = startAt + length;
				InternalComponent * _end = end();
				size_t sz = size();

				if (endAt > _end)
				{
					return LoopIterable(this, startAt, _end, _begin, endAt - sz);
				}
				else
				{
					return LoopIterable(this, startAt, _end, nullptr, _end);
				}
			}
			
			size_t innerMaxSize() noexcept
			{
				const Component * iter = begin();
				const Component * iend = end();
				if (iter == iend) return 0;

				size_t sz = iter->size();
				iter++;
				while (iter != iend)
				{
					size_t nsz = iter->size();
					if (nsz > sz) sz = nsz;
					iter++;
				}
				return sz;
			}
			Array<Ref> splitView(const InternalComponent& chr)
			{
				if (size() == 0) return nullptr;
				size_t scolen = count(chr) + 1;
				Array<Ref> arr(scolen);
				Ref * scores = arr.begin();
				Ref read(begin(), end());
				Ref readi;
				while ((readi = read.readwith(chr)) != nullptr)
				{
					*scores++ = readi;
				}
				*scores = read;
				return arr;
			}
			template <class Converter> Array<Array<typename Converter::Component>> split(const InternalComponent &chr)
			{
				using C2 = typename Converter::Component;
				if (size() == 0) return nullptr;
				size_t scolen = count(chr) + 1;
				Array<Array<C2>> arr(scolen);
				Array<C2>* scores = arr.begin();
				Ref read(begin(), end());
				Ref readi;
				while ((readi = read.readwith(chr)) != nullptr)
				{
					(scores++)->copy(Converter(readi));
				}
				scores->copy(Converter(read));
				return arr;
			}
			template <class Converter> Array2D<Array<typename Converter::Component>> split(const InternalComponent &chr, const InternalComponent &chrin)
			{
				using C2 = typename Converter::Component;
				if (size() == 0) return nullptr;
				size_t ylen = count(chr);
				Array2D<Array<C2>> arr(ylen + 1);

				Array<Array<C2>>* inarray = arr.begin();
				Ref read(begin(), end());

				for (size_t y = 0; y<ylen; y++)
				{
					*inarray++ = read.readwith(chr).template split<Converter>(chrin);
				}

				*inarray = read.template split<Converter>(chrin);
				return arr;
			}
			Array<Alc> split(const InternalComponent &chr)
			{
				return split<Ref>(chr);
			}
			Array2D<Alc> split(const InternalComponent &chr, const InternalComponent &chrin)
			{
				return split<Ref>(chr, chrin);
			}

			ReverseIterable<InternalComponentRef> reverse() noexcept
			{
				return ReverseIterable<InternalComponentRef>(begin(), end());
			}
			ReverseIterable<const InternalComponentRef> reverse() const noexcept
			{
				return ReverseIterable<const InternalComponentRef>(begin(), end());
			}

			Ref operator +(intptr_t n) const noexcept
			{
				return Ref(begin() + n, end());
			}
			Ref operator -(intptr_t n) const noexcept
			{
				return Ref(begin() - n, end());
			}
			intptr_t operator -(const Component * ptr) const noexcept
			{
				return data() - ptr;
			}
			friend intptr_t operator -(const Component * ptr, const MemBuffer& ori) noexcept
			{
				return ptr - ori.data();
			}
			template <typename _Derived, bool _szable, bool _readonly, typename _Parent> 
			intptr_t operator -(const MemBuffer<_Derived, BufferInfo<Component, true, _szable, _readonly, _Parent>>& ptr) const noexcept
			{
				return data() - ptr.data();
			}
			intptr_t operator -(const MemBuffer& ptr) const noexcept
			{
				return data() - ptr.data();
			}
		};

		template <typename Derived, typename Info>
		class WMemBuffer :public MemBuffer<Derived, Info>
		{
			CLASS_HEADER(WMemBuffer, MemBuffer<Derived, Info>);
		public:
			INHERIT_COMPONENT();

			using Super::Super;
			using Super::data;
			using Super::begin;
			using Super::end;
			using Super::size;
			using Super::sizeBytes;
			using Super::cut;
			using Super::subarr;
			using Super::subarray;
			using Super::contains_ptr;

			WRef cut(const ComponentRef* end) noexcept
			{
				_assert(contains_ptr(end));
				return WRef(begin(), const_cast<ComponentRef*>(end));
			}
			WRef cut(Ref _v) noexcept
			{
				return cut(_v.begin());
			}
			WRef cut(size_t _len) noexcept
			{
				return cut(mint(begin() + _len, end()));
			}
			WRef subarr(size_t _left, size_t _count) noexcept
			{
				return WRef(begin() + _left, end()).cut(_count);
			}
			WRef subarr(size_t _left) noexcept
			{
				return WRef(begin() + _left, end());
			}
			WRef subarray(size_t _left) noexcept
			{
				return WRef(begin() + _left, end());
			}
			WRef subarray(size_t _left, size_t _right) noexcept
			{
				_assert(_left <= _right);
				return WRef(begin() + _left, begin() + _right);
			}

			void change(const InternalComponent &fchr, const InternalComponent &tchr) noexcept
			{
				KR_DEFINE_MMEM();
				static_assert(std::is_trivially_default_constructible<Component>::value, "Need to use non class component");
				memm::change(begin(), fchr, tchr, size());
			}
			void fill(const InternalComponent &chr) noexcept
			{
				KR_DEFINE_MMEM();
				if (std::is_trivially_default_constructible<InternalComponent>::value)
				{
					memm::set(begin(), chr, size());
				}
				else
				{
					mema::subs_fill(begin(), chr, size());
				}
			}
			void zero() noexcept
			{
				memset(data(), 0, sizeBytes());
			}

			void subfill(const InternalComponent &chr, size_t sz, size_t offset) noexcept
			{
				_assert(offset + sz <= size());
				mema::subs_fill(begin() + offset, chr, sz);
			}
			void subcopy(const InternalComponent* arr, size_t sz, size_t offset = 0) noexcept
			{
				_assert(offset + sz <= size());
				mema::subs_copy(begin() + offset, arr, sz);
			}
			void subcopy(View<InternalComponent> arr, size_t offset = 0) noexcept
			{
				subcopy(arr.begin(), arr.size(), offset);
			}
		};
	}
}
