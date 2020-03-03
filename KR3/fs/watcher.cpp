#include "stdafx.h"

#ifdef WIN32
#include "watcher.h"
#include <KR3/msg/pump.h>

#include <KR3/win/windows.h>
#include <KR3/win/handle.h>

using namespace kr;

constexpr DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

static_assert(sizeof(OVERLAPPED) == sizeof(IOState), "must be same size");

IOState::IOState() noexcept
{
	memset(this, 0, sizeof(IOState));
};
OVERLAPPED* IOState::_overlapped() noexcept
{
	return (OVERLAPPED*)this;
}

#pragma warning(push)
#pragma warning(disable:26495)
DirectoryWatcher::DirectoryWatcher() noexcept
{
	m_dir = nullptr;
}
#pragma warning(pop)
DirectoryWatcher::~DirectoryWatcher() noexcept
{
	_assert(m_dir == nullptr);
}
void DirectoryWatcher::open(const char16 * dir, bool subtree) throws(Error)
{
	_assert(m_dir == nullptr);

	m_subtree = subtree;
	File * dirfile = (File*)CreateFileW(wide(dir),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);
	if (dirfile == ihv) throw Error();
	m_dir = dirfile;
	_request();

}
void DirectoryWatcher::close() noexcept
{
	CancelIoEx(m_dir, _overlapped());
	SleepEx(0, true); // resolve the cancel io state
	delete m_dir;
	m_dir = nullptr;
}

void DirectoryWatcher::_request() noexcept
{
	DWORD size;
	BOOL ReadDirectoryChangesW_result = ReadDirectoryChangesW(m_dir, m_buffer, sizeof(m_buffer), m_subtree, dwNotifyFilter, &size, _overlapped(),
		[](DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
			auto * _this = _from<DirectoryWatcher>(lpOverlapped);
			if (dwErrorCode == ERROR_OPERATION_ABORTED) return;
			_assert(dwErrorCode == ERROR_SUCCESS);
			if (_this->m_dir == nullptr) return;
			if (dwErrorCode == ERROR_SUCCESS)
			{
				FILE_NOTIFY_INFORMATION* pfni = (FILE_NOTIFY_INFORMATION*)_this->m_buffer;
				do
				{
					Text16 filename(unwide(pfni->FileName), pfni->FileNameLength / sizeof(WCHAR));

					switch (pfni->Action)
					{
					case FILE_ACTION_ADDED: _this->onCreated(filename); break;
					case FILE_ACTION_REMOVED: _this->onDeleted(filename); break;
					case FILE_ACTION_MODIFIED: _this->onModified(filename); break;
					case FILE_ACTION_RENAMED_OLD_NAME:
						_this->m_oldfilename.clear();
						_this->m_oldfilename << filename;
						break;
					case FILE_ACTION_RENAMED_NEW_NAME:
						_this->onRenamed(filename, _this->m_oldfilename);
						break;
					}

					pfni = (FILE_NOTIFY_INFORMATION*)((PBYTE)pfni + pfni->NextEntryOffset);
				} while (pfni->NextEntryOffset > 0);
			}
			_this->_request();
	});
	_assert(ReadDirectoryChangesW_result != false);
}

#else

EMPTY_SOURCE

#endif