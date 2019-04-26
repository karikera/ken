#include "stdafx.h"

#ifdef WIN32
#include "dirob.h"
#include "../pump.h"

#include <KR3/wl/windows.h>
#include <KR3/wl/handle.h>

constexpr DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

kr::DirectoryObserver::DirectoryObserver() noexcept
{
	m_dir = nullptr;
}
kr::DirectoryObserver::~DirectoryObserver() noexcept
{
	_assert(m_dir == nullptr);
}
void kr::DirectoryObserver::open(const char16 * dir) throws(Error)
{
	_assert(m_dir == nullptr);

	m_dir = (File*)CreateFileW(wide(dir),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);
	if (m_dir == ihv) throw Error();

	EventHandle * notify = EventHandle::create(false, false);
	_request(notify);
	m_event = EventDispatcher::regist(notify, [this](void*){
		FILE_NOTIFY_INFORMATION * pfni = (FILE_NOTIFY_INFORMATION*)m_buffer;
		do
		{
			Text16 filename(unwide(pfni->FileName), pfni->FileNameLength / sizeof(WCHAR));

			switch (pfni->Action)
			{
			case FILE_ACTION_ADDED: onCreate(filename); break;
			case FILE_ACTION_REMOVED: onDelete(filename); break;
			case FILE_ACTION_MODIFIED: onModified(filename); break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				m_oldfilename.clear();
				m_oldfilename << filename;
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				onRename(m_oldfilename, filename);
				break;
			}

			pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
		} while (pfni->NextEntryOffset > 0);
		_request(m_event->handle());
	});
}
void kr::DirectoryObserver::close() noexcept
{
	_assert(m_event != nullptr);
	m_event->remove();
	m_event = nullptr;
	delete m_dir;
	m_dir = nullptr;
}

void kr::DirectoryObserver::_request(EventHandle * notify) noexcept
{
	OVERLAPPED overlap = {};
	overlap.hEvent = notify;

	FILE_NOTIFY_INFORMATION * pfni = (FILE_NOTIFY_INFORMATION*)m_buffer;
	DWORD size;
	BOOL ReadDirectoryChangesW_result = ReadDirectoryChangesW(m_dir, m_buffer, sizeof(m_buffer), true, dwNotifyFilter, &size, &overlap, nullptr);
	_assert(ReadDirectoryChangesW_result != false);
}

#else

EMPTY_SOURCE

#endif