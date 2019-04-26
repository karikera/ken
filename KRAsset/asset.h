#pragma once

#include <KR3/main.h>
#include <KRUtil/fs/file.h>

namespace kr
{
	class AssetCompressor
	{
	public:
		class Package
		{
		public:
			Package(File * file) noexcept;
			void add(File * file) noexcept;

		private:
			Must<File> m_file;
		};
		AssetCompressor() noexcept;
		void add() noexcept;

	private:

	};

}