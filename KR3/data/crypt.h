#pragma once

#include <KR3/main.h>

#ifndef NO_USE_FILESYSTEM
#include <KR3/fs/file.h>

#endif

namespace kr
{
	namespace encoder
	{

		class HtmlEntity :public Encoder<HtmlEntity, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer * out, Text * text) noexcept;
			static size_t delength(Text text) noexcept;
			static size_t decode(char *out, Text text) noexcept;
			static void decode(Writer * out, Text * text) noexcept;

			static constexpr size_t SIZE = -1;
		};;
		class Uri :public Encoder<Uri, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer * out, Text * text) noexcept;
			static size_t delength(Text text) noexcept;
			static size_t decode(char* out, Text text) noexcept;
			static void decode(Writer * out, Text * text) noexcept;

			static constexpr size_t SIZE = -1;
		};;
		class Hex :public Encoder<Hex, char, void>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Buffer text) noexcept;
			static size_t encode(char * out, Buffer text) noexcept;
			static void encode(Writer * out, Buffer* text) noexcept;
			static size_t delength(Text text) noexcept;
			static size_t decode(void* out, Text text) noexcept;
			static void decode(BufferWriter* out, Text* text) noexcept;

			static constexpr size_t SIZE = -1;
		};
		class Base64 :public Encoder<Base64, char, void>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Buffer text) noexcept;
			static size_t encode(char * out, Buffer text) noexcept;
			static void encode(Writer *out, Buffer* text) noexcept;
			static size_t delength(Text text) noexcept;
			static size_t decode(void *out, Text text) noexcept;
			static void decode(BufferWriter *out, Text * text) noexcept;

			static constexpr size_t SIZE = -1;
		};
		template <typename Algorithm>
		class Hasher :public OutStream<Hasher<Algorithm>, AutoComponent, StreamInfo<false, Bufferable<Hasher<Algorithm>, BufferInfo<AutoComponent, method::CopyTo, false, true, Algorithm> > > >
		{
		public:
			using Algorithm::update;
			using Algorithm::finish;
			using Algorithm::SIZE;

			Hasher() = default;

			template <typename _Derived, typename _C, bool _szable, bool _readonly, typename _Parent>
			Hasher(const buffer::Memory<_Derived, BufferInfo<_C, method::Memory, _szable, _readonly, _Parent>>& data) noexcept
			{
				update(data.template cast<void>());
			}
			template <typename C>
			void $write(const C* data, size_t _sz) noexcept
			{
				update(Buffer(data, _sz * sizeof(internal_component_t<C>)));
			}
			template <typename C>
			size_t $sizeAs() const noexcept
			{
				return (SIZE + sizeof(internal_component_t<C>) - 1) / sizeof(internal_component_t<C>);
			}
			template <typename C>
			size_t $copyTo(C* dest) const noexcept
			{
				finish(dest);
				return $sizeAs<C>();
			}

			static TBuffer hash(File * file) noexcept
			{
				Hasher hasher;
				hasher << file->stream<void>()->readAll();
				delete file;
				return (TBuffer)hasher;
			}
		};

		class Sha1Context
		{
		public:
			static constexpr size_t SIZE = 20;
			Sha1Context() noexcept;
			void reset() noexcept;
			void update(Buffer input) noexcept;
			void finish(void* out) const noexcept;

		private:
			byte m_context[90];
		};
		class Sha256Context
		{
		public:
			static constexpr size_t SIZE = 32;
			Sha256Context() noexcept;
			void reset() noexcept;
			void update(Buffer input) noexcept;
			void finish(void* out) const noexcept;

		private:
			byte m_context[112];
		};
		class Md5Context
		{
		public:
			static constexpr size_t SIZE = 16;
			Md5Context() noexcept;
			void reset() noexcept;
			void update(Buffer input) noexcept;
			void finish(void* out) const noexcept;

		private:
			byte m_context[88];
		};

		using Sha1 = Hasher<Sha1Context>;
		using Sha256 = Hasher<Sha256Context>;
		using Md5 = Hasher<Md5Context>;
	}

	namespace io
	{
		template <typename Derived>
		using HexOStream = OStreamEncoder<Derived, encoder::Hex>;
		template <typename Derived>
		using UriOStream = OStreamEncoder<Derived, encoder::Uri>;
		template <typename Derived>
		using Base64OStream = OStreamEncoder<Derived, encoder::Base64>;
	}
}

extern template class kr::encoder::Encoder<kr::encoder::Uri, char, char>;
extern template class kr::encoder::Encoder<kr::encoder::HtmlEntity, char, char>;
extern template class kr::encoder::Encoder<kr::encoder::Hex, char, void>;
extern template class kr::encoder::Encoder<kr::encoder::Base64, char, void>;

extern template class kr::encoder::Hasher<kr::encoder::Sha1Context>;
extern template class kr::encoder::Hasher<kr::encoder::Sha256Context>;
extern template class kr::encoder::Hasher<kr::encoder::Md5Context>;
