#include "stdafx.h"

#ifndef NO_USE_FILESYSTEM

#include "file.h"

using namespace kr;

#ifdef WIN32

#include <KR3/win/windows.h>
#include <KR3/win/handle.h>
#include <shellapi.h>

static_assert(File::NAMELEN == MAX_PATH, "MAX_PATH not matching");

#else

#error Need implement

#endif


#include <KR3/io/selfbufferedstream.h>
#include <KR3/data/crypt/md5.h>
#include <KR3/util/path.h>

#pragma warning(disable:4800)

namespace
{
}

template<typename CHR> File* File::createT(const CHR * str) throws(Error)
{
	return _createFile(str, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
}
template<typename CHR> File* File::openT(const CHR * str) throws(Error)
{
	return _createFile(str, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
}
template<typename CHR> File* File::openWriteT(const CHR * str) throws(Error)
{
	return _createFile(str, GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);
}
template<typename CHR> File* File::createRWT(const CHR * str) throws(Error)
{
	return _createFile(str, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
}
template<typename CHR> File* File::openRWT(const CHR* str) throws(Error)
{
	return _createFile(str, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);
}

void File::operator delete(ptr p) noexcept
{
	CloseHandle((HANDLE)p);
}

bool File::exists(pcstr16 src) noexcept
{
	return GetFileAttributesW(wide(src)) != -1;
}
bool File::copy(pcstr16 dest, pcstr16 src) noexcept
{
	return CopyFileW(wide(src), wide(dest), false);
}
bool File::move(pcstr16 dest, pcstr16 src) noexcept
{
	return MoveFileW(wide(src), wide(dest));
}
bool File::copyFull(pcstr16 dest, pcstr16 src) noexcept
{
	SHFILEOPSTRUCT fos = { 0 };
	fos.wFunc = FO_COPY;
	fos.pFrom = wide(src);
	fos.pTo = wide(dest);
	fos.fFlags = FOF_NO_UI;
	return SHFileOperationW(&fos) == 0;
}
bool File::toJunk(pcstr16 src) noexcept
{
	createDirectory(u"junk");

	uint maxnum = 0;
	for (pcstr16 filename : FindFile(u"junk/*"))
	{
		Text16 filenametx = (Text16)filename;
		if (!filenametx.numberonly())
			continue;
		uint now = filenametx.to_uint();
		if (maxnum < now)
			maxnum = now;
	}
	
	return move(TSZ16() << u"junk/" << (maxnum + 1), src);
}
bool File::clearJunk() noexcept
{
	if (!exists(u"junk"))
		return true;
	return removeFullDirectory(u"junk");
}
bool File::remove(pcstr16 str) noexcept
{
	return DeleteFileW(wide(str));
}
bool File::isFile(pcstr16 str) noexcept
{
  DWORD dwAttrib = GetFileAttributesW(wide(str));
  return ((dwAttrib != INVALID_FILE_ATTRIBUTES) &&  !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
bool File::isDirectory(pcstr16 str) noexcept
{
	DWORD dwAttrib = GetFileAttributesW(wide(str));
	return ((dwAttrib != INVALID_FILE_ATTRIBUTES) &&  (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
inline bool isDirectoryModifiedLoop(Path & path, filetime_t axis)
{
	FindFile find(path.getsz(u"*"));
	if (!find.exists())
		return false;

	for (pcstr16 filename : find)
	{
		if (find.isDirectory())
		{
			char16 * save = path.enter((Text16)filename);
			if (isDirectoryModifiedLoop(path, axis))
				return true;
			path.leave(save);
		}
		if (find.getLastModifiedTime() > axis)
			return true;
	}
	return false;
}
bool File::isDirectoryModified(Text16 dir, filetime_t axis) noexcept
{
	Path path;
	path.enter(dir);
	if (!File::isDirectory(path.getCurrentDirectorySz()))
		return false;
	return isDirectoryModifiedLoop(path, axis);
}
bool File::createDirectory(pcstr16 str) noexcept
{
	return CreateDirectoryW(wide(str), nullptr);
}
bool File::createFullDirectory(Text16 str) noexcept
{
	TText16 temp((size_t)0, 1024);
	Text16 nstr = str;
#ifdef WIN32
	size_t sz = nstr.size();
	if (sz >= 1)
	{
		if (sz >= 3)
		{
			if (nstr[1] == ':' && path16.isSeperator(nstr[2]))
			{
				temp << nstr.cut(3);
				nstr += 3;
			}
		}
		else
		{
			if (path16.isSeperator(*nstr))
			{
				nstr++;
			}
		}
	}

#else
	if (!nstr.empty())
	{
		if (path16.isSeperator(*nstr))
		{
			nstr++;
		}
	}
#endif

	for (;;)
	{
		Text16 dir = nstr.readwith_L(path16.isSeperator);
		if (dir == nullptr) break;
		_assert(!dir.empty());
		temp << dir;
		
		switch(*dir)
		{
		case '.':
			if (dir.size() == 1 || (dir.size() == 2 && dir[1] == '.'))
			{
				temp << path16.sep;
				continue;
			}
			break;
		}
		if (!createDirectory(temp.c_str()))
		{
			int err = GetLastError();
			if (err != ERROR_ALREADY_EXISTS) return false;
		}
		temp << path16.sep;
	}
	if (nstr.empty()) return true;
	temp << nstr;
	
	switch (*nstr)
	{
	case '.':
		if (nstr.size() == 1 || (nstr.size() == 2 && nstr[1] == '.'))
		{
			return true;
		}
		break;
	}
	createDirectory(temp.c_str());
	return true;
}
bool File::removeFullDirectory(pcstr16 path) noexcept
{
	SHFILEOPSTRUCTW fos = { 0 };
	fos.wFunc = FO_DELETE;
	fos.pFrom = wide(path);
	fos.fFlags = FOF_NO_UI;
	return SHFileOperationW(&fos) == 0;
}
bool File::removeShell(pcstr16 path) noexcept
{
	SHFILEOPSTRUCTW fos = { 0 };
	fos.wFunc = FO_DELETE;
	fos.pFrom = wide(path);
	fos.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR; // FOF_NO_UI;
	return SHFileOperationW(&fos) == 0;
}
void File::md5All(byte _dest[16]) throws(Error)
{
	io::FIStream<char> fis(this);
	md5_context ctx;
	md5_starts(&ctx);
	try
	{
		for (;;)
		{
			fis.request();
			Text text = fis.text();
			md5_update(&ctx, (uint8_t*)text.begin(), intact<uint32_t>(text.size()));
			fis.clearBuffer();
		}
	}
	catch (EofException &)
	{
	}
	md5_finish(&ctx, _dest);
}
void File::md5(size_t sz, byte _dest[16]) throws(Error)
{
	io::FIStream<char, false> fis(this);
	intptr_t minusSize = -(intptr_t)sz;
	
	md5_context ctx;
	md5_starts(&ctx);
	try
	{
		while (sz != 0)
		{
			fis.request(sz);
			Text text = fis.text();
			if (text.size() > sz)
				text.cut_self(sz);
			sz -= text.size();
			
			md5_update(&ctx, (uint8_t*)text.begin(), intact<uint32_t>(text.size()));
			fis.clearBuffer();
		}
	}
	catch (EofException &)
	{
	}
	md5_finish(&ctx, _dest);
	movePointer(minusSize);
}
uint32_t File::size32() throws(TooBigException)
{
	_assert(this != nullptr);
	filesize_t sz = size();
	if(sz >= 0x7fffffff) throw TooBigException();
	return (dword)sz;
}
filesize_t File::size() noexcept
{
	_assert(this != nullptr);
	qword base;
	(dword&)base = GetFileSize(this,(LPDWORD)&base+1);
	return base;
}
size_t File::sizep() throws(TooBigException)
{
	if (sizeof(size_t) == sizeof(filesize_t))
	{
		return (size_t)size();
	}
	else
	{
		return size32();
	}
}
void File::$write(cptr buff, size_t len) throws(Error)
{
	_assert(this != nullptr);
	_assert(len <= 0xffffffff);
	DWORD writed;
	BOOL res = WriteFile(this, buff, (dword)len, &writed, nullptr);
	if (res == FALSE)
		throw Error();
	_assert(len == writed);
}
size_t File::$read(ptr buff, size_t len) throws(EofException)
{
	_assert(this != nullptr);
	if (len == 0) return 0;
	_assert(len <= 0xffffffff);
	BOOL res = ReadFile(this, buff, (dword)len, (LPDWORD)&len, nullptr);
	_assert(res == TRUE);
	if (len == 0) throw EofException();
	return len;
}
filesize_t File::getPointer() noexcept
{
	_assert(this != nullptr);
	qword value=0;
	(dword&)value = SetFilePointer(this, 0, (PLONG)&value+1, FILE_CURRENT);
	_assert((dword&)value != INVALID_SET_FILE_POINTER);
	return value;
}
void File::movePointerToEnd(int offset) noexcept
{
	_assert(this != nullptr);
	return _movePointer(FILE_END, offset);
}
void File::movePointerToEnd(int64_t offset) noexcept
{
	_assert(this != nullptr);
	return _movePointer(FILE_END, offset);
}
void File::movePointer(int Move) noexcept
{
	_assert(this != nullptr);
	return _movePointer(FILE_CURRENT, Move);
}
void File::movePointer(int64_t Move) noexcept
{
	_assert(this != nullptr);
	return _movePointer(FILE_CURRENT, Move);
}
void File::setPointer(int Move) noexcept
{
	_assert(this != nullptr);
	return _movePointer(FILE_BEGIN, Move);
}
void File::setPointer(int64_t Move) noexcept
{
	_assert(this != nullptr);
	return _movePointer(FILE_BEGIN, Move);
}
void File::toBegin() noexcept
{
	_assert(this != nullptr);
	_movePointer(FILE_BEGIN,0);
}
void File::toEnd() noexcept
{
	_assert(this != nullptr);
	_movePointer(FILE_END,0);
}
void File::skip(int64_t skip) noexcept
{
	_assert(this != nullptr);
	_movePointer(FILE_CURRENT,skip);
}
ptr File::allocAll(size_t *pSize) throws(TooBigException)
{
	_assert(this != nullptr);
	filesize_t qwSize=size();
	if (sizeof(size_t) != sizeof(filesize_t))
	{
		if (hidword(qwSize) != 0)
			throw TooBigException();
	}
	size_t upSize=*pSize=(size_t)qwSize;
	if(upSize == 0) return nullptr;
	ptr p=_new byte[upSize];
	$read(p, upSize);
	return p;
}
File::Mapping File::beginMapping(filesize_t off,size_t read) throws(Error)
{
	_assert(this != nullptr);
	Mapping map;
	map.offset = off % getAllocationGranularity();
	filesize_t noff = off - map.offset;
	read += map.offset;

	map.handle = (Handle<>*)CreateFileMapping(this, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if(map.handle == nullptr) throw Error();
	map.point=MapViewOfFile(map.handle,FILE_MAP_READ,hidword(noff),lodword(noff),read);
	_assert(map.handle != nullptr);
	if(map.point==nullptr)
	{
		CloseHandle(map.handle);
		throw Error();
	}
	map.point = (byte*)map.point + map.offset;
	return map;
}
void File::endMapping(const Mapping& map) noexcept
{
	_assert(this != nullptr);
	UnmapViewOfFile((byte*)map.point - map.offset);
	CloseHandle(map.handle);
}
filetime_t File::getLastModifiedTime(pcstr16 filename) throws(Error)
{
	WIN32_FILE_ATTRIBUTE_DATA  fileInfo;
	if (!GetFileAttributesExW(wide(filename), GetFileExInfoStandard, &fileInfo))
	{
		throw Error();
	}
	return (filetime_t &)fileInfo.ftLastWriteTime;
}
filetime_t File::getLastModifiedTime() noexcept
{
	_assert(this != nullptr);
	FILETIME modify;
	BOOL GetFileTimeResult = GetFileTime(this, nullptr, nullptr, &modify);
	_assert(GetFileTimeResult);
	return (filetime_t&)modify;
}
filetime_t File::getCreationTime() noexcept
{
	_assert(this != nullptr);
	FILETIME ft;
	BOOL GetFileTimeResult = GetFileTime(this, &ft, nullptr, nullptr);
	_assert(GetFileTimeResult);
	return (filetime_t&)ft;
}
bool File::setModifyTime(filetime_t t) noexcept
{
	_assert(this != nullptr);
	return SetFileTime(this, nullptr, nullptr, (FILETIME*)&t);
}
bool File::setCreationTime(filetime_t t) noexcept
{
	_assert(this != nullptr);
	return SetFileTime(this, (FILETIME*)&t, nullptr, nullptr);
}

#ifdef WIN32
File * File::_createFile(pcstr str, dword Access, dword ShareMode, dword Disposition) throws(Error)
{
	File * file = (File*)CreateFileA(str, Access, ShareMode, nullptr, Disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == ihv) throw Error();
	return file;
}
File * File::_createFile(pcstr16 str, dword Access, dword ShareMode, dword Disposition) throws(Error)
{
	File * file = (File*)CreateFileW(wide(str), Access, ShareMode, nullptr, Disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == ihv) throw Error();
	return file;
}
void File::_movePointer(dword Method, int Move) noexcept
{
	_assert(this != nullptr);
	dword res = SetFilePointer(this, Move, nullptr, Method);
	_assert(res != INVALID_SET_FILE_POINTER);
}
void File::_movePointer(dword Method, int64_t Move) noexcept
{
	_assert(this != nullptr);
	dword res = SetFilePointerEx(this, (LARGE_INTEGER&)Move, nullptr, Method);
	_assert(res != INVALID_SET_FILE_POINTER);
}
#endif

#pragma warning(push)
#pragma warning(disable:25495)
FindFile::FindFile() noexcept
{
	m_handle = nullptr;
}
#pragma warning(pop)
FindFile::~FindFile() noexcept
{
	if(m_handle != nullptr) FindClose(m_handle);
}
FindFile::FindFile(pcstr16 file) noexcept
{
	m_handle = FindFirstFileW(wide(file), (WIN32_FIND_DATAW*)m_buffer);
	if (m_handle == ihv)
	{
		m_handle = nullptr;
		return;
	}
	if (m_handle == nullptr) return;
	
	while (_isDotDir())
	{
		if (!next())
		{
			FindClose(m_handle);
			m_handle = nullptr;
			break;
		}
	}
}

#pragma warning(push)
#pragma warning(disable:25495)
FindFile::FindFile(FindFile && o) noexcept
{
	m_handle = o.m_handle;
	o.m_handle = nullptr;
}
#pragma warning(pop)
FindFile& FindFile::operator =(FindFile && o) noexcept
{
	m_handle = o.m_handle;
	o.m_handle = nullptr;
	return *this;
}
bool FindFile::exists() noexcept
{
	return m_handle != nullptr;
}
const char16 * FindFile::getFileName() noexcept
{
	return (pcstr16)((WIN32_FIND_DATAW*)m_buffer)->cFileName;
}
bool FindFile::isDirectory() noexcept
{
	return (((WIN32_FIND_DATAW*)m_buffer)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
bool FindFile::next() noexcept
{
	for (;;)
	{
#pragma warning(push)
#pragma warning(disable:4800)
		bool res = FindNextFileW(m_handle, (WIN32_FIND_DATAW*)m_buffer);
#pragma warning(pop)
		if (!res) return false;
		if (_isDotDir()) continue;
		return true;
	}
}
filetime_t FindFile::getLastModifiedTime() noexcept
{
	return (filetime_t&)((WIN32_FIND_DATAW*)m_buffer)->ftLastWriteTime;
}
FindFile::Iterator::Iterator(FindFile * ff) noexcept
	:m_ff(ff)
{
}
FindFile::Iterator& FindFile::Iterator::operator ++() noexcept
{
	if (!m_ff->next())
	{
		m_ff = nullptr;
	}
	return *this;
}
const char16 * FindFile::Iterator::operator *() noexcept
{
	return m_ff->getFileName();
}
FindFile::Iterator FindFile::begin() noexcept
{
	if (m_handle == nullptr) return nullptr;
	return this;
}
FindFile::Iterator FindFile::end() noexcept
{
	return nullptr;
}
bool kr::FindFile::_isDotDir() noexcept
{
	WIN32_FIND_DATAW* fd = (WIN32_FIND_DATAW*)m_buffer;
	if (!isDirectory())
		return false;
	if (fd->cFileName[0] != L'.')
		return false;
	
	switch (fd->cFileName[1])
	{
	case L'.':
		if (fd->cFileName[2] == L'\0')
			return true;
		break;
	case L'\0':
		return true;
	}
	return false;
}
bool FindFile::Iterator::operator == (const Iterator & o) noexcept
{
	return m_ff == o.m_ff;
}
bool FindFile::Iterator::operator != (const Iterator & o) noexcept
{
	return m_ff != o.m_ff;
}

MappedFile::MappedFile(File * file)
	:m_file(file)
{
	m_size = m_file->size32();
	m_map = m_file->beginMapping(0, m_size);
}
MappedFile::MappedFile(const char16 * filename)
	: m_file(File::open(filename))
{
	m_size = m_file->size32();
	m_map = m_file->beginMapping(0, m_size);
}
MappedFile::~MappedFile() noexcept
{
	m_file->endMapping(m_map);
}
size_t MappedFile::$size() const noexcept
{
	return m_size;
}
void * MappedFile::$begin() noexcept
{
	return m_map.point;
}
void * MappedFile::$end() noexcept
{
	return (byte*)m_map.point + m_size;
}
const void * MappedFile::$begin() const noexcept
{
	return m_map.point;
}
const void * MappedFile::$end() const noexcept
{
	return (byte*)m_map.point + m_size;
}

DirectoryScanner::DirectoryScanner() noexcept
{
	m_dircut = m_path.begin();
}
pcstr16 kr::DirectoryScanner::getSzName() noexcept
{
	m_path << nullterm;
	return m_path.begin();
}
Text16 kr::DirectoryScanner::getPathText() noexcept
{
	return m_path.cut(m_dircut);
}
Text16 kr::DirectoryScanner::getRelativeText(Text16 path) noexcept
{
	m_path.cut_self(m_dircut);
	m_path << path;
	return m_path;
}
pcstr16 kr::DirectoryScanner::getRelativeSzName(Text16 path) noexcept
{
	m_path.cut_self(m_dircut);
	m_path << path << nullterm;
	return m_path.begin();
}

template File* File::openT<char>(const char * str);
template File* File::openWriteT<char>(const char * str);
template File* File::createRWT<char>(const char * str);
template File* File::openRWT<char>(const char* str);
template File* File::createT<char>(const char * str);
template File* File::openT<char16>(const char16 * str);
template File* File::openWriteT<char16>(const char16 * str);
template File* File::createRWT<char16>(const char16 * str);
template File* File::openRWT<char16>(const char16* str);
template File* File::createT<char16>(const char16 * str);

#endif
