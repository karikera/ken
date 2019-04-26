#pragma once

#include <KR3/main.h>
#include <KR3/mt/thread.h>
#include <KRUtil/fs/file.h>

#include "../eventdispatcher.h"

namespace kr
{
	class DirectoryObserver
	{
	public:
		DirectoryObserver() noexcept;
		~DirectoryObserver() noexcept;
		void open(const char16 * dir) throws(Error);
		void close() noexcept;
		virtual void onCreate(Text16 name) noexcept = 0;
		virtual void onDelete(Text16 name) noexcept = 0;
		virtual void onModified(Text16 name) noexcept = 0;
		virtual void onRename(Text16 oldname, Text16 newname) noexcept = 0;

	private:
		void _request(EventHandle * notify) noexcept;

		File * m_dir;
		DispatchedEvent * m_event;
		byte m_buffer[8192];
		BText16<File::NAMELEN> m_oldfilename;
	};
}
