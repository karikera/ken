#include "stdafx.h"
#include "asset.h"

#include <KRUtil/fs/file.h>
#include <KRCompress/lzma.h>

kr::AssetCompressor::AssetCompressor() noexcept
{
}

void kr::AssetCompressor::add() noexcept
{
}

kr::AssetCompressor::Package::Package(File * file) noexcept
	:m_file(file)
{
}

void kr::AssetCompressor::Package::add(File * file) noexcept
{
//	m_file->write();
}
