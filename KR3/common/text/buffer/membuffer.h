#pragma once

#include "../hasmethod.h"

namespace kr
{
	template <typename T>
	class ReversePointer :public MakePointerIterator<ReversePointer<T>, T>
	{
	public:
		INHERIT_ITERATOR(MakePointerIterator<ReversePointer<T>, T>);
	private:
		using Super::m_pt;

	public:

		intptr_t operator -(const ReversePointer& o) const noexcept
		{
			return o.m_pt - m_pt;
		}
		ReversePointer& operator += (intptr_t v) noexcept
		{
			m_pt -= v;
			return *this;
		}
		ReversePointer& operator -= (intptr_t v) noexcept
		{
			m_pt += v;
			return *this;
		}
		ReversePointer operator + (intptr_t v) noexcept
		{
			ReversePointer n = *this;
			n.m_pt -= v;
			return n;
		}
		ReversePointer operator - (intptr_t v) noexcept
		{
			ReversePointer n = *this;
			n.m_pt += v;
			return n;
		}
		void next() noexcept
		{
			m_pt--;
		}
		void previous() noexcept
		{
			m_pt++;
		}
		T& operator [](size_t idx) const noexcept
		{
			return m_pt[-(intptr_t)idx];
		}
	};

	template <typename C>
	class Reverse
	{
	private:
		C * m_beg;
		C * m_end;

	public:
		Reverse(C* beg, C* end) noexcept
		{
			m_beg = end - 1;
			m_end = beg - 1;
		}
		C* data() const noexcept
		{
			return m_beg;
		}
		ReversePointer<C> begin() const noexcept
		{
			return m_beg;
		}
		ReversePointer<C> end() const noexcept
		{
			return m_end;
		}
		size_t size() const noexcept
		{
			return m_beg - m_end;
		}
	};

	template <typename C>
	class FilteredBuffer :public Bufferable<FilteredBuffer<C>, BufferInfo<C, method::CopyWriteTo> >
	{
	private:
		const View<C> m_text;
		C(*const m_filter)(C);
	public:
		using InternalComponent = internal_component_t<C>;

		FilteredBuffer(View<C> text, InternalComponent(*filter)(InternalComponent)) noexcept
			:m_text(text), m_filter(filter)
		{
		}
		template <typename Derived, typename Info>
		void $writeTo(OutStream<Derived, C, Info>* dest) const noexcept
		{
			WriteLock<OutStream<Derived, C, Info> > lock(m_text.size());
			InternalComponent * ptr = lock.lock(dest);
			for (InternalComponent chr : m_text)
			{
				*ptr++ = m_filter(chr);
			}
			lock.unlock(dest);
		}
		size_t $copyTo(C* dest) const noexcept
		{
			InternalComponent* ptr = (InternalComponent*)dest;
			for (InternalComponent chr : m_text)
			{
				*ptr++ = m_filter(chr);
			}
			return m_text.size();
		}

		size_t size() const noexcept
		{
			return m_text.size();
		}
	};

	template <typename C>
	class SplitIterator :public MakeIterableIterator<SplitIterator<C>, View<C>>
	{
	private:
		using IC = internal_component_t<C>;
	public:
		const IC* m_ref;
		const IC* m_next;
		const IC* m_end;
		const IC* m_done;
		IC m_chr;

	public:
		SplitIterator() = default;
		SplitIterator(View<C> _this, IC chr) noexcept
			: m_chr(move(chr))
		{
			m_ref = _this.begin();
			m_end = _this.end();
			m_next = (IC*)_this.find_e(m_chr);
			m_done = m_end + 1;
		}
		bool isEnd() const noexcept
		{
			return m_ref == m_done;
		}
		void next() noexcept
		{
			m_ref = m_next;
			m_ref++;
			if (m_ref == m_done) return;
			m_next = (IC*)View<C>(m_ref, m_end).find_e(m_chr);
		}
		View<C> value() const noexcept
		{
			return View<C>(m_ref, m_next);
		}
	};

	template <typename C>
	class TextSplitIterator :public MakeIterableIterator<TextSplitIterator<C>, View<C>>
	{
	private:
		using IC = internal_component_t<C>;

		const IC* m_ref;
		const IC* m_next;
		const IC* m_end;
		const IC* m_done;
		View<C> m_chr;

	public:
		TextSplitIterator() = default;
		TextSplitIterator(View<C> _this, View<C> chr) noexcept
			: m_chr(chr)
		{
			m_ref = _this.begin();
			m_end = _this.end();
			m_next = (IC*)_this.find_e(m_chr);
			m_done = m_end + m_chr.size();
		}
		bool isEnd() const noexcept
		{
			return m_ref == m_done;
		}
		void next() noexcept
		{
			m_ref = m_next;
			m_ref += m_chr.size();
			if (m_ref == m_done) return;
			m_next = (IC*)View<C>(m_ref, m_end).find_e(m_chr);
		}
		View<C> value() const noexcept
		{
			return View<C>(m_ref, m_next);
		}
	};

	template <typename C>
	class ReverseSplitIterator :public MakeIterableIterator<ReverseSplitIterator<C>, View<C>>
	{
	private:
		using IC = internal_component_t<C>;

		const IC* m_ref;
		const IC* m_next;
		const IC* m_begin;
		const IC* m_done;
		IC m_chr;

	public:
		ReverseSplitIterator() = default;
		ReverseSplitIterator(View<C> _this, IC chr) noexcept
			: m_chr(move(chr))
		{
			m_begin = _this.begin();
			m_ref = _this.end();
			m_next = (IC*)_this.find_r(m_chr);
			if (m_next == nullptr) m_next = m_begin;
			m_done = m_begin - 1;
		}
		bool isEnd() const noexcept
		{
			return m_ref == m_done;
		}
		void next() noexcept
		{
			m_ref = m_next;
			m_ref--;
			if (m_ref == m_done) return;
			m_next = (IC*)View<C>(m_begin, m_ref).find_r(m_chr);
			if (m_next == nullptr) m_next = m_begin;
		}
		View<C> value() const noexcept
		{
			return View<C>(m_next, m_ref);
		}
	};

	template <typename C>
	class ReverseTextSplitIterator :public MakeIterableIterator<ReverseTextSplitIterator<C>, View<C>>
	{
	private:
		using IC = internal_component_t<C>;

		const IC* m_ref;
		const IC* m_next;
		const IC* m_begin;
		const IC* m_done;
		View<C> m_chr;

	public:
		ReverseTextSplitIterator() = default;
		ReverseTextSplitIterator(View<C> _this, View<C> chr) noexcept
			: m_chr(chr)
		{
			m_begin = _this.begin();
			m_ref = _this.end();
			m_next = (IC*)_this.find_r(m_chr);
			if (m_next == nullptr) m_next = m_begin;
			else m_next += m_chr.size();
			m_done = m_begin - m_chr.size();
		}
		bool isEnd() const noexcept
		{
			return m_ref == m_done;
		}
		void next() noexcept
		{
			m_ref = m_next;
			m_ref -= m_chr.size();
			if (m_ref == m_done) return;
			m_next = (IC*)View<C>(m_begin, m_ref).find_r(m_chr);
			if (m_next == nullptr) m_next = m_begin;
			else m_next += m_chr.size();
		}
		View<C> value() const noexcept
		{
			return View<C>(m_next, m_ref);
		}
	};

	template <typename C>
	class LoopIterator :public MakeIterableIterator<LoopIterator<C>, internal_component_t<C>&>
	{
	private:
		using IC = internal_component_t<C>;
		IC* m_ptr;
		IC* m_end;
		IC* m_ptr2;
		IC* m_end2;

	public:
		LoopIterator() = default;
		LoopIterator(IC* ptr, IC* end, IC* ptr2, IC* end2) noexcept
		{
			m_ptr = ptr;
			m_end = end;
			m_ptr2 = ptr2;
			m_end2 = end2;
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
		void next() noexcept
		{
			m_ptr++;
			if (m_ptr == m_end)
			{
				m_ptr = m_ptr2;
				m_end = m_end2;
				m_ptr2 = nullptr;
			}
		}
		bool isEnd() const noexcept
		{
			return m_ptr == nullptr;
		}
		IC& value() const noexcept
		{
			return *m_ptr;
		}
	};

	namespace buffer
	{
		template <typename Derived, typename Component, template <typename, typename> class Method, bool szable, bool readonly, typename Parent>
		class Memory<Derived, BufferInfo<Component, Method, szable, readonly, Parent> > 
			:public WriteToByCopyTo<Derived, Component, BufferInfo<Component, Method, szable, readonly, Parent> >
		{
			CLASS_HEADER(WriteToByCopyTo<Derived, Component, BufferInfo<Component, Method, szable, readonly, Parent> >);
			using Memory_Info = BufferInfo<Component, Method, szable, readonly, Parent>;
		public:
			INHERIT_COMPONENT();

			using Super::Super;

			ComponentRef* data() noexcept
			{
				return static_cast<Derived*>(this)->$begin();
			}
			InternalComponentRef* begin() noexcept
			{
				return static_cast<Derived*>(this)->$begin();
			}
			InternalComponentRef* end() noexcept
			{
				return static_cast<Derived*>(this)->$end();
			}
			const Component * data() const noexcept
			{
				return static_cast<const Derived*>(this)->$begin();
			}
			const InternalComponent* begin() const noexcept
			{
				return static_cast<const Derived*>(this)->$begin();
			}
			const InternalComponent* end() const noexcept
			{
				return static_cast<const Derived*>(this)->$end();
			}
			size_t size() const noexcept
			{
				return static_cast<const Derived*>(this)->$size();
			}
			bool empty() const noexcept
			{
				return static_cast<const Derived*>(this)->emptyImpl();
			}
			template <typename T> size_t sizeAs() const noexcept
			{
				static_assert(is_same<T, Component>::value, "Need same type");
				return size();
			}

			bool equals(Ref other) const noexcept
			{
				size_t tsize = size();
				size_t osize = other.size();
				if (tsize != osize) return false;
				return mem::equals(begin(), other.begin(), tsize);
			}
			size_t hash() const noexcept
			{
				return mem::hash(begin(), size() * sizeof(InternalComponent));
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
				mema::assign_copy((InternalComponent*)dest, begin(), size());
				return size();
			}
			size_t bytes() const noexcept
			{
				return size() * sizeof(InternalComponent);
			}
			bool contains_ptr(const Component* _v) const noexcept
			{
				return begin() <= _v && _v <= end();
			}
			bool contains_ptr_or_null(const Component* _v) const noexcept
			{
				return _v == nullptr || (begin() <= _v && _v <= end());
			}
			template <class _Derived, bool a, bool b, class _Parent>
			bool contains_ptr(const Memory<_Derived, BufferInfo<Component, method::Memory, a, b, _Parent>>& _v) const noexcept
			{
				return contains_ptr(_v.begin());
			}
			Ref cut(const Component* end) const noexcept
			{
				_assert(contains_ptr(end));
				return Ref(begin(), end);
			}
			Ref cut(size_t _len) const noexcept
			{
				return cut(begin() + _len);
			}

			Ref subarr(const Component* _ptr) const noexcept
			{
				_assert(contains_ptr_or_null(_ptr));
				return Ref(_ptr, end());
			}
			Ref subarray(const Component* _ptr) const noexcept
			{
				_assert(contains_ptr_or_null(_ptr));
				return Ref(_ptr, end());
			}
			Ref subarr(size_t _left) const noexcept
			{
				_assert(_left <= size());
				return Ref(begin() + _left, end());
			}
			Ref subarray(size_t _left) const noexcept
			{
				_assert(_left <= size());
				return Ref(begin() + _left, end());
			}
			Ref subarr(size_t _left, size_t _count) const noexcept
			{
				_assert(_left <= size());
				_assert(_left + _count <= size());
				return Ref(begin() + _left, end()).cut(_count);
			}
			Ref subarray(size_t _left, size_t _right) const noexcept
			{
				_assert(_left <= _right);
				_assert(_right <= size());
				return Ref(begin() + _left, begin() + _right);
			}
			Ref offsetBytes(size_t bytes) const noexcept
			{
				return Ref((InternalComponent*)((uint8_t*)begin() + bytes), end());
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
				_assert(!empty());
				return *begin();
			}
			const InternalComponentRef& operator *() const
			{
				_assert(!empty());
				return *begin();
			}
			const InternalComponentRef& get(size_t index) const noexcept
			{
				_assert(index < size());
				return begin()[index];
			}
			InternalComponentRef& get(size_t index) noexcept
			{
				_assert(index < size());
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
			ComponentRef* find_n(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_n(begin(), _v, size());
			}
			ComponentRef* find_ne(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_ne(begin(), _v, size());
			}
			ComponentRef* find_ny(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_ny(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find_nr(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_nr(begin(), _v, size());
			}
			ComponentRef* find_nry(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_nry(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find_nye(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_nye(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find(const InternalComponent &needle) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find(begin(), needle, size());
			}
			ComponentRef* find(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = _v.size();
				if (_len < _len2)
					return nullptr;
				return (ComponentRef*)memm::find(begin(), _v.begin(), _len, _len2);
			}
			ComponentRef* find_e(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_e(begin(), _v, size());
			}
			ComponentRef* find_e(Ref _v) const noexcept
			{
				const Component* finded = find(_v);
				if (finded == nullptr) return end();
				return (ComponentRef*)finded;
			}
			ComponentRef* find_y(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_y(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find_ye(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_ye(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find_r(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_r(begin(), _v, size());
			}
			ComponentRef* find_r(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = _v.size();
				if (_len < _len2)
					return nullptr;
				return (ComponentRef*)memm::find_r(begin(), _v.begin(), _len, _len2);
			}
			ComponentRef* find_ry(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_ry(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find_rye(Ref _v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_rye(begin(), _v.begin(), size(), _v.size());
			}
			ComponentRef* find_re(const InternalComponent &_v) const noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)memm::find_re(begin(), _v, size());
			}
			ComponentRef* find_re(Ref _v) const noexcept
			{
				const Component* finded = find_r(_v);
				if (finded == nullptr) return begin() - 1;
				return (ComponentRef*)finded;
			}
			template <typename LAMBDA>
			ComponentRef* find_L(LAMBDA && lambda) const noexcept
			{
				InternalComponentRef * e = end();
				InternalComponentRef * p = begin();
				for (; p != e; p++)
				{
					if (lambda(*p)) return (ComponentRef*)p;
				}
				return nullptr;
			}

			ComponentRef* end_find_r(const InternalComponent &needle) noexcept
			{
				KR_DEFINE_MMEM();
				const Component* _end = memm::find_r(begin(), needle, size());
				if (_end == nullptr)
				{
					Ref out;
					out.setEnd(nullptr);
					return (ComponentRef*)out;
				}
				return (ComponentRef*)(_end + 1);
			}
			ComponentRef* end_find_re(const InternalComponent &needle) noexcept
			{
				KR_DEFINE_MMEM();
				return (ComponentRef*)(memm::find_re(begin(), needle, size()) + 1);
			}
			ComponentRef* end_find_r(Ref needle) noexcept
			{
				KR_DEFINE_MMEM();
				size_t _len = size();
				size_t _len2 = needle.size();
				if (_len < _len2) return nullptr;
				const Component* _end = memm::find_r(begin(), needle.begin(), _len, _len2);
				if (_end == nullptr) return nullptr;
				return (ComponentRef*)(_end + needle.size());
			}
			ComponentRef* end_find_re(Ref needle) noexcept
			{
				const Component* finded = end_find_r(needle);
				if (finded == nullptr) return (ComponentRef*)begin();
				return (ComponentRef*)finded;
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

			FilteredBuffer<Component> filter(InternalComponent(*filter)(InternalComponent)) const noexcept
			{
				return FilteredBuffer<Component>(*this, filter);
			}
			template <class Converter, class _Derived, typename _Info>
			void replace(OutStream<_Derived, typename Converter::Component, _Info> * _out, Ref _tar, View<typename Converter::Component> _to) const // NotEnoughSpaceException
			{
				Ref reader = *static_cast<const Bufferable<Derived, Memory_Info>*>(this);

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
				Ref reader = *static_cast<const Bufferable<Derived, Memory_Info>*>(this);

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

			SplitIterator<Component> splitIterable(const InternalComponent &chr) const noexcept
			{
				return SplitIterator<Component>((Ref)*this, chr);
			}
			TextSplitIterator<Component> splitIterable(Ref chr) const noexcept
			{
				return TextSplitIterator<Component> ((Ref)*this, chr);
			}
			ReverseSplitIterator<Component> reverseSplitIterable(const InternalComponent &chr) const noexcept
			{
				return ReverseSplitIterator<Component> ((Ref)*this, chr );
			}
			ReverseTextSplitIterator<Component> reverseSplitIterable(Ref chr) const noexcept
			{
				return ReverseTextSplitIterator<Component> ((Ref)*this, chr);
			}

			LoopIterator<Component> loopIterable(size_t offset) noexcept
			{
				InternalComponent * _begin = begin();
				InternalComponent * startAt = _begin + offset;
				return { startAt, end(), _begin, startAt };
			}
			LoopIterator<Component> loopIterable(size_t offset, size_t length) noexcept
			{
				InternalComponent * _begin = begin();
				InternalComponent * startAt = _begin + offset;
				InternalComponent * endAt = startAt + length;
				InternalComponent * _end = end();
				size_t sz = size();

				if (endAt > _end)
				{
					return { startAt, _end, _begin, endAt - sz };
				}
				else
				{
					return { startAt, _end, nullptr, _end };
				}
			}
			
			size_t innerMaxSize() noexcept
			{
				const Component* iter = begin();
				const Component* iend = end();
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
				Array<Ref> arr;
				arr.resize(scolen);
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
				Array<Array<C2>> arr;
				arr.resize(scolen);
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
				Array2D<Array<C2>> arr;
				arr.resize(ylen + 1);

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

			Reverse<InternalComponentRef> reverse() noexcept
			{
				return Reverse<InternalComponentRef>(begin(), end());
			}
			Reverse<const InternalComponentRef> reverse() const noexcept
			{
				return Reverse<const InternalComponentRef>(begin(), end());
			}

			Ref operator +(intptr_t n) const noexcept
			{
				return Ref(begin() + n, end());
			}
			Ref operator -(intptr_t n) const noexcept
			{
				return Ref(begin() - n, end());
			}
			intptr_t operator -(const Component* ptr) const noexcept
			{
				return data() - ptr;
			}
			friend intptr_t operator -(const Component* ptr, const Memory& ori) noexcept
			{
				return ptr - ori.data();
			}
			template <typename _Derived, bool _szable, bool _readonly, typename _Parent> 
			intptr_t operator -(const Memory<_Derived, BufferInfo<Component, method::Memory, _szable, _readonly, _Parent>>& ptr) const noexcept
			{
				return data() - ptr.data();
			}
			intptr_t operator -(const Memory& ptr) const noexcept
			{
				return data() - ptr.data();
			}
		};

		template <typename Derived, typename Info>
		class WMemory :public Memory<Derived, Info>
		{
			CLASS_HEADER(Memory<Derived, Info>);
		public:
			INHERIT_COMPONENT();

			using Super::Super;
			using Super::data;
			using Super::begin;
			using Super::end;
			using Super::size;
			using Super::bytes;
			using Super::cut;
			using Super::subarr;
			using Super::subarray;
			using Super::contains_ptr;

			WRef cut(const Component* end) noexcept
			{
				_assert(contains_ptr(end));
				return WRef(begin(), (Component*)end);
			}
			WRef cut(Ref _v) noexcept
			{
				return cut(_v.begin());
			}
			WRef cut(size_t _len) noexcept
			{
				return cut(begin() + _len);
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
					mema::assign_fill(begin(), chr, size());
				}
			}
			void zero() noexcept
			{
				memset(data(), 0, bytes());
			}

			void subfill(const InternalComponent& chr, size_t sz, size_t offset) noexcept
			{
				_assert(offset + sz <= size());
				mema::assign_fill(begin() + offset, chr, sz);
			}
			void subcopy(const Component* arr, size_t sz, size_t offset = 0) noexcept
			{
				_assert(offset + sz <= size());
				mema::assign_copy(begin() + offset, (InternalComponent*)arr, sz);
			}
			void subcopy(View<Component> arr, size_t offset = 0) noexcept
			{
				subcopy(arr.begin(), arr.size(), offset);
			}
		};
	}
}
