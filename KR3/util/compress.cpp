#include "stdafx.h"
#include "compress.h"

#include <KR3/fs/file.h>
#include <KR3/win/windows.h>


using namespace kr;

void kr::extractEntryTo(pcstr16 dest, KrbCompressEntry* entry) noexcept
{
	if (entry->isDirectory)
	{
		File::createDirectory(dest);
	}
	else
	{
		constexpr size_t BUF_SIZE = 8192;
		TBuffer buffer(BUF_SIZE);
		void* buffer_ptr = buffer.data();

		try
		{
			Must<File> file = File::create(dest);
			for (;;)
			{
				size_t readed = entry->read(buffer_ptr, BUF_SIZE);
				if (readed == 0) break;
				file->write(buffer_ptr, readed);
			}
			file->setModifyTime(entry->filetime);
		}
		catch (Error&)
		{
		}
	}
}

Unzipper::Unzipper(const fchar_t* path) noexcept
	:m_path(path)
{
	filter = [](Unzipper*, Text, Text16 dest){
		return dest.data();
	};
}
bool Unzipper::extractTo(Text16 path) noexcept
{
	File::createFullDirectory(path);

	m_dir.reserve(path.size() + 20);
	m_dir << path;
	if (!path16.endsWithSeperator(m_dir)) m_dir << path16.sep;
	m_dirEnd = m_dir.size();

	krb::File file(m_path);
	if (!file.param) return false;

	entry = [](KrbCompressCallback* _this, KrbCompressEntry* entry) {

		Unzipper* cb = static_cast<Unzipper*>(_this);

		Text filename(entry->filename, entry->filenameLength);
		cb->m_dir << (Utf8ToUtf16)(Text)entry->filename;
		cb->m_dir.c_str();
		pcstr16 dest = cb->filter(cb, filename, cb->m_dir);
		if (dest != nullptr)
		{
			extractEntryTo(dest, entry);
		}
		cb->m_dir.resize(cb->m_dirEnd);
	};
	return krb_load_compress(krb_make_extension_from_path(m_path), this, &file);
}
