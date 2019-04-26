#pragma once

namespace kr
{

	template <size_t size> struct intrinsic
	{
	};
	template <> struct intrinsic<1>
	{
		static byte adc(byte carry, byte v1, byte v2, void* res) noexcept;
		static byte sbb(byte carry, byte v1, byte v2, void* res) noexcept;
	};
	template <> struct intrinsic<2>
	{
		static byte adc(byte carry, word v1, word v2, void* res) noexcept;
		static byte sbb(byte carry, word v1, word v2, void* res) noexcept;
		static word rotl(word value, byte shift) noexcept;
		static word rotr(word value, byte shift) noexcept;
	};
	template <> struct intrinsic<3>
	{
		static byte adc(byte carry, dword v1, dword v2, void* res) noexcept;
		static byte sbb(byte carry, dword v1, dword v2, void* res) noexcept;
	};
	template <> struct intrinsic<4>
	{
		static byte adc(byte carry, dword v1, dword v2, void* res) noexcept;
		static byte sbb(byte carry, dword v1, dword v2, void* res) noexcept;
		static dword rotl(dword value, int shift) noexcept;
		static dword rotr(dword value, int shift) noexcept;
	};
	template <> struct intrinsic<5>
	{
		static byte adc(byte carry, qword v1, qword v2, void* res) noexcept;
		static byte sbb(byte carry, qword v1, qword v2, void* res) noexcept;
	};
	template <> struct intrinsic<6>
	{
		static byte adc(byte carry, qword v1, qword v2, void* res) noexcept;
		static byte sbb(byte carry, qword v1, qword v2, void* res) noexcept;
	};
	template <> struct intrinsic<7>
	{
		static byte adc(byte carry, qword v1, qword v2, void* res) noexcept;
		static byte sbb(byte carry, qword v1, qword v2, void* res) noexcept;
	};
	template <> struct intrinsic<8>
	{
		static byte adc(byte carry, qword v1, qword v2, void* res) noexcept;
		static byte sbb(byte carry, qword v1, qword v2, void* res) noexcept;
		static qword rotl(qword value, int shift) noexcept;
		static qword rotr(qword value, int shift) noexcept;
	};


}
