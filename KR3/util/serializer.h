#pragma once

#include <KR3/main.h>
#include <KR3/math/coord.h>
#include <vector>

namespace kr
{
	template <typename OS> class Serializer;
	template <typename IS> class Deserializer;

	namespace _pri_
	{
		template <typename T, bool is_class> struct BaseSerialize;
		template <typename T> struct BaseSerialize<T, true>
		{
			template <typename S>
			inline static void serialize(T& dest, S & serializer)
			{
				dest.serialize(serializer);
			}
		};
		template <typename T> struct BaseSerialize<T, false>
		{
			template <typename S>
			inline static void serialize(T& dest, S & serializer)
			{
				serializer.raw(dest);
			}
		};
	}
	template <typename T> 
	struct Serialize : _pri_::BaseSerialize<T, std::is_class<T>::value>
	{
	};
	template <typename C>
	struct Serialize<Array<C>>
	{
		template <typename S>
		inline static void serialize(Array<C>& dest, S & s)
		{
			s.serializeSize(&dest);
			for (C & c : dest)
				s & c;
		}
	};
	template <typename C>
	struct Serialize<View<C>>
	{
		template <typename S>
		inline static void serialize(View<C>& dest, S & s)
		{
			s.reference(dest);
		}
	};
	template <typename C>
	struct Serialize<std::vector<C>>
	{
		template <typename S>
		inline static void serialize(std::vector<C>& dest, S & s)
		{
			s.serializeSize(&dest);
			for (C & c: dest)
				s & c;
		}
	};
	template <typename C, size_t count, bool aligned, typename order>
	struct Serialize<math::vector<C, count, aligned, order>>
	{
		template <typename S>
		inline static void serialize(math::vector<C, count, aligned, order>& dest, S & s)
		{
			s.raw(dest);
		}
	};
	template <typename C>
	struct Serialize<std::basic_string<C>>
	{
		template <typename S>
		inline static void serialize(std::basic_string<C>& dest, S & s)
		{
			s.serializeSize(&dest);
			for (C & c : dest)
				s & c;
		}
	};
	template <typename C, size_t size>
	struct Serialize<C[size]>
	{
		template <typename S>
		inline static void serialize(C(&dest)[size], S & serializer)
		{
			for (C & c : dest)
			{
				serializer & dest;
			}
		}
	};

	class SerializedSizer
	{
	private:
		size_t m_size;

	public:
		static constexpr bool read = true;
		static constexpr bool write = false;
		inline SerializedSizer() noexcept
		{
			m_size = 0;
		}
		template <typename T>
		inline void raw(T &) noexcept
		{
			m_size += sizeof(T);
		}
		template <typename T>
		inline void serializeSize(T * buffer)
		{
			size_t sz = buffer->size();
			if(sz == 0) m_size ++;
			else m_size += math::plog2(sz) / 7 + 1;
		}
		template <typename T>
		inline SerializedSizer& operator &(T & t)
		{
			Serialize<T>::serialize(t, *this);
			return *this;
		}
		template <typename T>
		inline SerializedSizer& operator <<(T & t)
		{
			Serialize<T>::serialize(t, *this);
			return *this;
		}
		template <typename T>
		inline SerializedSizer& operator >>(T & t)
		{
			Serialize<T>::serialize(t, *this);
			return *this;
		}
		size_t size() const noexcept
		{
			return m_size;
		}
		template <typename T>
		static size_t getSize(const T & value)
		{
			SerializedSizer sizer;
			sizer & const_cast<T&>(value);
			return sizer.m_size;
		}
	};

	template <class OS> class Serializer
	{
		using Component = typename OS::Component;
		static_assert(sizeof(typename OS::InternalComponent) == 1, "sizeof Component != 1");
	private:
		OS * const m_os;

	public:
		static constexpr bool read = false;
		static constexpr bool write = true;

		inline Serializer(OS * os) noexcept
			:m_os(os)
		{
		}
		OS * stream() const noexcept
		{
			return m_os;
		}
		template <typename T>
		inline void raw(T & t)
		{
			m_os->write((Component*)&t, sizeof(T));
		}
		inline void leb128(dword value)
		{
			m_os->writeLeb128(value);
		}
		inline void leb128(qword value)
		{
			m_os->writeLeb128(value);
		}
		inline void kr_leb128(dword value)
		{
			byte out;
			while (value >= 0x80)
			{
				out = (byte)(value & 0x7f);
				m_os->write((Component*)&out, 1);
				value >>= 7;
			}
			out = (byte)(value | 0x80);
			m_os->write((Component*)&out, 1);
		}
		inline void kr_leb128(qword value)
		{
			byte out;
			while (value >= 0x80)
			{
				out = (byte)(value) & 0x7f;
				m_os->write((Component*)&out, 1);
				value >>= 7;
			}
			out = (byte)(value) | 0x80;
			m_os->write((Component*)&out, 1);
		}
		template <typename C>
		inline void reference(const C* ref, size_t sz)
		{
			copy(ref, sz);
		}
		template <typename C>
		inline void copy(const C* p, size_t sz)
		{
			sz *= sizeof(C);
			if (m_os->write((Component*)p, sz) != sz)
				throw NotEnoughSpaceException();
		}
		template <typename C>
		inline void reference(View<C> &ref)
		{
			size_t sz = ref.size();
			kr_leb128(sz);
			reference(ref, sz);
		}
		template <typename C>
		inline void reference(View<C> &ref, size_t sz)
		{
			copy(ref.data(), sz);
		}
		template <typename T>
		inline void serializeSize(T * buffer)
		{
			kr_leb128(buffer->size());
		}
		template <typename T>
		inline Serializer& operator &(const T & t)
		{
			Serialize<T>::serialize((T&)t, *this);
			return *this;
		}
		template <typename T>
		inline Serializer& operator <<(const T & t)
		{
			Serialize<T>::serialize((T&)t, *this);
			return *this;
		}
	};
	template <class IS> class Deserializer
	{
		using Component = typename IS::Component;
		static_assert(sizeof(typename IS::InternalComponent) == 1, "sizeof Component != 1");
	private:
		IS * const m_is;

	public:
		static constexpr bool read = true;
		static constexpr bool write = false;

		inline Deserializer(IS * is) noexcept
			: m_is(is)
		{
		}
		IS * stream() const noexcept
		{
			return m_is;
		}
		template <typename T>
		inline void raw(T & t)
		{
			m_is->read((Component*)&t, sizeof(T));
		}
		inline void leb128(dword &value)
		{
			value = m_is->readLeb128();
		}
		inline void leb128(qword &value)
		{
			value = m_is->readLeb128_64();
		}
		inline void kr_leb128(dword & value)
		{
			value = m_is->readLeb128_kr();
		}
		inline void kr_leb128(qword & value)
		{
			value = m_is->readLeb128_kr64();
		}
		template <typename C>
		inline void reference(const C* &ref, size_t sz)
		{
			ref = (C*)m_is->begin();
			m_is->addBegin(sz * sizeof(C));
		}
		template <typename C>
		inline void copy(C* p, size_t sz)
		{
			sz *= sizeof(C);
			if (m_is->read((Component*)p, sz) != sz)
				throw EofException();
		}
		template <typename C>
		inline void reference(View<C> &ref)
		{
			size_t sz;
			kr_leb128(sz);
			reference(ref, sz);
		}
		template <typename C>
		inline void reference(View<C> &ref, size_t sz)
		{
			const C * beg;
			reference(beg, sz);
			ref = View<C>(beg, sz);
		}
		template <typename T>
		inline void serializeSize(T * buffer)
		{
			size_t sz;
			kr_leb128(sz);
			buffer->resize(sz);
		}
		template <typename T>
		inline Deserializer& operator &(T & t)
		{
			Serialize<T>::serialize(t, *this);
			return *this;
		}
		template <typename T>
		inline Deserializer& operator >>(T & t)
		{
			Serialize<T>::serialize(t, *this);
			return *this;
		}
	};
}