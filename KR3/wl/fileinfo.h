#pragma once

#ifndef WIN32
#error is not windows system
#endif

#include <KR3/main.h>

namespace kr
{
	struct ProgramVersion:public Bufferable<ProgramVersion, BufferInfo<AutoComponent>>
	{
		union
		{
			struct { word b, a, d, c; };
			struct { word arr[4]; };
			struct { dword ab, cd; };
			qword abcd;
		};

		ProgramVersion() = default;
		ProgramVersion(qword abcd) noexcept;
		ProgramVersion(dword ab, dword cd) noexcept;
		ProgramVersion(word a, word b, word c, word d) noexcept;
		static ProgramVersion fromFile(pcstr16 filename) noexcept;

		template <typename C>
		void $copyTo(ArrayWriter<C> * dest) const noexcept
		{
			*dest << a << (C)' ' << (C)',' << b << (C)' ' << (C)',' << c << (C)' ' << (C)',' << d;
		}
		template <typename C>
		size_t $copyTo(C * dest) const noexcept
		{
			ArrayWriter<C> stream(dest, 0xffff);
			copyTo(&stream);
			return stream.end() - dest;
		}
		template <typename C>
		size_t $sizeAs() const noexcept
		{
			return math::cipher(a, 10) + 2
				+ math::cipher(b, 10) + 2
				+ math::cipher(c, 10) + 2
				+ math::cipher(d, 10);
		}
	};
}