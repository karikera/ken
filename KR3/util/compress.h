#pragma once

#include "resloader.h"
#include <KRThird/ken-res-loader/include/ken-back/compress.h>

namespace kr
{
	void extractEntryTo(pcstr16 dest, KrbCompressEntry* entry) noexcept;

	class Unzipper: private KrbCompressCallback
	{
	public:
		Unzipper(const fchar_t* path) noexcept;
		bool extractTo(Text16 path) noexcept;

		bool (*filter)(Unzipper* unzipper, Text filename);

	private:
		const fchar_t* const m_path;

		// for extractTo
		AText16 m_dir;
		size_t m_dirEnd;
	};
}