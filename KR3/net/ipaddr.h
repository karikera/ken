#pragma once

#include <KR3/main.h>

namespace kr
{
	struct Ipv4Address:public Bufferable<Ipv4Address, BufferInfo<AutoComponent>>
	{
		union
		{
			struct { byte a, b, c, d;  } ;
			byte arr[4];
			dword value;
		};
		

		Ipv4Address() = default;
		Ipv4Address(nullptr_t) noexcept;
		Ipv4Address(byte a, byte b, byte c, byte d) noexcept;
		explicit Ipv4Address(kr::Text str) noexcept;

		bool operator ==(const Ipv4Address& v) const noexcept;
		bool operator !=(const Ipv4Address& v) const noexcept;

		size_t $size() const noexcept;
		template <typename CHR>
		size_t $sizeAs() const noexcept
		{
			return $size();
		}
		template <typename CHR>
		size_t $copyTo(CHR * dest) const noexcept;

		static const Ipv4Address loopBack;
	};

}

template<>
struct std::hash<kr::Ipv4Address>
{
	size_t operator ()(const kr::Ipv4Address& ip) const noexcept;
};
