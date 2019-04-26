#pragma once

#include <KR3/main.h>

namespace kr
{
	namespace io
	{
		template <class Derived, class Encoder>
		class OStreamEncoder;
	}
	namespace encoder
	{
		template <typename Derived, typename To, typename From>
		class Parser : public Container<To>
		{
			CLASS_HEADER(Parser, Container<To>);
		public:
			INHERIT_COMPONENT();
			
			
		};

		template <typename Derived, typename To, typename From>
		class Encoder :public Bufferable<Derived, BufferInfo<To>>
		{
			CLASS_HEADER(Encoder, Bufferable<Derived, BufferInfo<To>>);
		public:
			INHERIT_COMPONENT();
			using ToComponent = To;
			using FromComponent = From;

			using ToText = View<To>;
			using FromText = View<From>;

			Encoder(View<From> src) noexcept;
			size_t size() const noexcept;
			size_t copyTo(To * dest) const noexcept;
			class Decoder : public Bufferable<Decoder, BufferInfo<From>>
			{
			public:
				Decoder(View<To> src) noexcept;
				size_t size() const noexcept;
				size_t copyTo(From * dest) const noexcept;

			private:
				const View<To> m_data;
				const size_t m_size;
			};

			template <class _Derived> 
			static io::OStreamEncoder<_Derived, Derived> * cast(_Derived * t)
			{
				return reinterpret_cast<io::OStreamEncoder<_Derived, Derived>*>(t);
			}

		private:
			const View<From> m_data;
			const size_t m_size;
		};

		template <typename Derived, typename To, typename From>
		Encoder<Derived, To, From>::Encoder(View<From> src) noexcept
			:m_data(src), m_size(Derived::length(src))
		{
		}
		template <typename Derived, typename To, typename From>
		size_t Encoder<Derived, To, From>::size() const noexcept
		{
			return m_size;
		}
		template <typename Derived, typename To, typename From>
		size_t Encoder<Derived, To, From>::copyTo(To * dest) const noexcept
		{
			return Derived::encode(dest, m_data);
		}
		template <typename Derived, typename To, typename From>
		Encoder<Derived, To, From>::Decoder::Decoder(View<To> src) noexcept
			:m_data(src), m_size(Derived::delength(src))
		{
		}
		template <typename Derived, typename To, typename From>
		size_t Encoder<Derived, To, From>::Decoder::size() const noexcept
		{
			return m_size;
		}
		template <typename Derived, typename To, typename From>
		size_t Encoder<Derived, To, From>::Decoder::copyTo(From * dest) const noexcept
		{
			return Derived::decode(dest, m_data);
		}

		template <typename T>
		class PassEncoder
			: public Encoder<PassEncoder<T>, T, T>
		{
			using Super = Encoder<PassEncoder<T>, T, T>;
		public:
			using Super::Super;
			using typename Super::Decoder;
			using Text = typename Super::ToText;
			using Writer = typename Super::ToWriter;

			static size_t length(Text text) noexcept
			{
				return text.size();
			}
			static size_t encode(T * out, Text text) noexcept
			{
				mema::subs_copy(out, text.data(), text.size());
				return text.size();
			}
			static void encode(Writer * out, Text * text) noexcept
			{
				out->passThrough(text);
			}
			static size_t delength(Text text) noexcept
			{
				return text.size();
			}
			static size_t decode(T * out, Text text) noexcept
			{
				mema::subs_copy(out, text.data(), text.size());
				return text.size();
			}
			static void decode(Writer * out, Text * text) noexcept
			{
				out->passThrough(text);
			}
		};;

	}
	
	namespace io
	{
		template <class Base, class Encoder>
		class OStreamEncoder :
			public OutStream<OStreamEncoder<Base, Encoder>, typename Base::Component, StreamInfo<false>>
		{
			CLASS_HEADER(OStreamEncoder, OutStream<OStreamEncoder<Base, Encoder>, typename Base::Component, StreamInfo<false>>);
		public:
			INHERIT_COMPONENT();
			static_assert(IsOStream<Base>::value, "Base is not OutStream");

			OStreamEncoder() noexcept = delete;

			static OStreamEncoder * from(Base * base) noexcept
			{
				return reinterpret_cast<OStreamEncoder*>(base);
			}
			Base * base() noexcept
			{
				return reinterpret_cast<Base*>(this);
			}

			void writeImpl(const Component * _data, size_t _size)
			{
				size_t _nsize = Encoder::length(View<Component>(_data, _size));
				WriteLock<Base> lockData(_nsize);
				lockData.lock(base());
				Encoder::encode(&ArrayWriter<Component>(lockData.begin(), _nsize), &View<Component>(_data, _size));
				lockData.unlock(base());
			}
		};
	}

}
