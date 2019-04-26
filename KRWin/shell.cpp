#include "stdafx.h"
#include "shell.h"

using namespace kr;
using namespace win;

DragFileList::DragFileList(HDROP drop) noexcept
	:m_drop(drop)
{
	m_count = DragQueryFile(drop, 0xFFFFFFFF, NULL, NULL);
}
DragFileList::DragFileList(WPARAM drop) noexcept
	: DragFileList((HDROP)drop)
{
}
DragFileList::~DragFileList() noexcept
{
	DragFinish(m_drop);
}

uint DragFileList::size() noexcept
{
	return m_count;
}
Text16 DragFileList::operator [](uint i) noexcept
{
	uint len = DragQueryFile(m_drop, i, wide(m_fileName), MAX_PATH);
	return Text16(m_fileName, len);
}
