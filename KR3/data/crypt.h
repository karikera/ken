#pragma once

#include <KR3/main.h>


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
			static size_t delength(Text text) noexcept = delete;
			static void decode(Writer * out, Text * text) noexcept = delete;

			static constexpr size_t SIZE = -1;
		};;
		class Hex :public Encoder<Hex, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer * out, Text * text) noexcept;
			static size_t delength(Text text) noexcept = delete;
			static void decode(Writer *out, Text * text) noexcept = delete;

			static constexpr size_t SIZE = -1;
		};
		class Base64 :public Encoder<Base64, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer *out, Text * text) noexcept;
			static size_t delength(Text text) noexcept;
			static size_t decode(char *out, Text text) noexcept;
			static void decode(Writer *out, Text * text) noexcept;

			static constexpr size_t SIZE = -1;
		};
		class Sha1 :public Encoder<Sha1, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer *out, Text * text) noexcept;
			static void encode2(Writer *out, Text * text) noexcept;

			static constexpr size_t SIZE = 20;
		};
		class Sha256 :public Encoder<Sha256, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer *out, Text * text) noexcept;

			static constexpr size_t SIZE = 32;
		};
		class Md5 :public Encoder<Md5, char, char>
		{
		public:
			using Encoder::Encoder;
			static size_t length(Text text) noexcept;
			static size_t encode(char * out, Text text) noexcept;
			static void encode(Writer *out, Text * text) noexcept;

			static constexpr size_t SIZE = 16;
		};
	}

	namespace io
	{
		template <typename Derived>
		using HexOStream = OStreamEncoder<Derived, encoder::Hex>;
		template <typename Derived>
		using UriOStream = OStreamEncoder<Derived, encoder::Uri>;
		template <typename Derived>
		using Base64OStream = OStreamEncoder<Derived, encoder::Base64>;
		template <typename Derived>
		using Sha1OStream = OStreamEncoder<Derived, encoder::Sha1>;
	}
}