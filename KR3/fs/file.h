#pragma once

#include <KR3/main.h>
NEED_FILESYSTEM

#include <KR3/util/uts.h>

namespace kr
{
	class File: public io::StreamCastable<File>
	{
	public:
		class Mapping
		{
			friend File;
		public:
			ptr point;
			dword offset;

		private:
			void* handle;
		};

#ifdef WIN32
		static constexpr uint NAMELEN = PathMaxLen;
#else
		static constexpr uint NAMELEN = 512; // 적당히 큰 값을 사용하였다.
#endif

		File() = delete;
		template <typename CHR>
		static File* createT(const CHR * str) throws(Error);
		template <typename CHR>
		static File* openT(const CHR * str) throws(Error);
		template <typename CHR>
		static File* openWriteT(const CHR * str) throws(Error);
		template <typename CHR>
		static File* createRWT(const CHR * str) throws(Error);
		template <typename CHR>
		static File* openRWT(const CHR * str) throws(Error);

		static File* create(const char * str) throws(Error) { return createT(str); }
		static File* create(const char16 * str) throws(Error) { return createT(str); }
		static File* open(const char * str) throws(Error) { return openT(str); }
		static File* open(const char16 * str) throws(Error) { return openT(str); }
		static File* openWrite(const char * str) throws(Error) { return openWriteT(str); }
		static File* openWrite(const char16 * str) throws(Error) { return openWriteT(str); }
		static File* createRW(const char * str) throws(Error) { return createRWT(str); }
		static File* createRW(const char16 * str) throws(Error) { return createRWT(str); }
		static File* openRW(const char* str) throws(Error) { return openRWT(str); }
		static File* openRW(const char16* str) throws(Error) { return openRWT(str); }
		static void operator delete(ptr p) noexcept;

		//DefineWritablePropertySZ(File, name, GetName, CPUTF16);

		static bool exists(pcstr16 src) noexcept;
		static bool copy(pcstr16 dest, pcstr16 src) noexcept;
		static bool move(pcstr16 dest, pcstr16 src) noexcept;
		static bool copyFull(pcstr16 dest, pcstr16 src) noexcept;
		static bool toJunk(pcstr16 src) noexcept;
		static bool clearJunk() noexcept;
		static bool remove(pcstr16 str) noexcept;
		static bool isFile(pcstr16 str) noexcept;
		static bool isDirectory(pcstr16 str) noexcept;
		static bool isDirectoryModified(Text16 dir, filetime_t axis) noexcept;
		static bool createDirectory(pcstr16 str) noexcept;
		static bool createFullDirectory(Text16 str) noexcept;
		static bool removeFull(pcstr16 path) noexcept;
		static bool removeShell(pcstr16 path) noexcept;
		
		template <typename C>
		inline SizedStreamBuffer<C, io::StreamableStream<File, C>* > readAll() throws(TooBigException)
		{
			return { this->stream<C>(), sizep() };
		}

		template <typename C>
		using FileStreamBuffer = SizedStreamBuffer<C, Keep<io::StreamableStream<File, C>> >;

		template <typename C, typename CHR>
		static inline FileStreamBuffer<C> openAsArrayT(const CHR* name) throws(TooBigException, Error)
		{
			Keep<io::StreamableStream<File, C>> file = open(name)->template stream<C>();
			size_t size = file->sizep();
			return { std::move(file), size };
		}
		template <typename C>
		static inline FileStreamBuffer<C> openAsArray(const char * name) throws(TooBigException, Error)
		{
			return openAsArrayT<C,char>(name);
		}
		template <typename C>
		static inline FileStreamBuffer<C> openAsArray(const char16* name) throws(TooBigException, Error)
		{
			return openAsArrayT<C, char16>(name);
		}
		template <typename C, typename CHR>
		static inline void saveFromArrayT(const CHR * name, View<C> arr) throws(TooBigException, Error)
		{
			Must<File> file = create(name);
			return file->$write(arr.begin(), arr.bytes());
		}
		template <typename C>
		static inline void saveFromArray(const char * name, View<C> arr) throws(Error)
		{
			return saveFromArrayT(name, arr);
		}
		template <typename C>
		static inline void saveFromArray(const char16 * name, View<C> arr) throws(Error)
		{
			return saveFromArrayT(name, arr);
		}
		filesize_t getPointer() noexcept;
		void movePointerToEnd(int offset) noexcept;
		void movePointerToEnd(int64_t offset) noexcept;
		void movePointer(int Move) noexcept;
		void movePointer(int64_t Move) noexcept;
		void setPointer(int Move) noexcept;
		void setPointer(int64_t Move) noexcept;
		void toBegin() noexcept;
		void toEnd() noexcept;
		void skip(int64_t skip) noexcept;
		ptr allocAll(size_t *pSize) throws(TooBigException);
		Mapping beginMapping(filesize_t off, size_t read) throws(Error);
		void endMapping(const Mapping& map) noexcept;

		// 사용 후 파일이 닫힌다.
		void md5All(byte _dest[16]) throws(Error);

		// 지정된 크기만큼 md5 해시를 생성하고 파일 포인터를 읽기 전 위치로 돌려놓는다.
		void md5(size_t sz, byte _dest[16]) throws(Error);

		uint32_t size32() throws(TooBigException);
		filesize_t size() noexcept;
		size_t sizep() throws(TooBigException);
		void $write(cptr buff, size_t len) throws(Error);
		size_t $read(ptr buff, size_t len) throws(EofException);

		static filetime_t getLastModifiedTime(pcstr16 filename) throws(Error);
		filetime_t getLastModifiedTime() noexcept;
		filetime_t getCreationTime() noexcept;
		bool setModifyTime(filetime_t t) noexcept;
		bool setCreationTime(filetime_t t) noexcept;
		template <typename T> void readStructure(T *value) throws(EofException)
		{
			readStructure(value, sizeof(T));
		}
		template <typename T> filesize_t find(T chr) throws(EofException)
		{
			filesize_t len=0;
			dword readed;
			T temp[1024];
			int64_t from=0;
			filesize_t out;
			pcstr find;
			do
			{
				readed=1024*sizeof(T);
				read(temp, readed);
				find=(pcstr)mem::find(temp, chr, readed);
				from-=readed;

				if(!readed)
				{
					out=~(filesize_t)0;
					goto label_failed;
				}
			}
			while(!find);

			out=(find-temp-readed)-from;
		label_failed:
			movePointer(from);
			return out;
		}

	private:
#ifdef WIN32
		static File * _createFile(pcstr str, dword Access, dword ShareMode, dword Disposition) throws(Error);
		static File * _createFile(pcstr16 str, dword Access, dword ShareMode, dword Disposition) throws(Error);
		void _movePointer(dword Method, int Move) noexcept;
		void _movePointer(dword Method, int64_t Move) noexcept;
#endif
	};

	class FindFile
	{
	public:
		FindFile() noexcept;
		~FindFile() noexcept;
		FindFile(pcstr16 file) noexcept;
		FindFile(FindFile && o) noexcept;
		FindFile& operator =(FindFile && o) noexcept;

		bool exists() noexcept;
		const char16 * getFileName() noexcept;
		bool isDirectory() noexcept;
		bool next() noexcept;
		filetime_t getLastModifiedTime() noexcept;

		class Iterator
		{
		public:
			Iterator(FindFile * ff) noexcept;
			Iterator& operator ++() noexcept;
			const char16 * operator *() noexcept;
			bool operator == (const Iterator & o) noexcept;
			bool operator != (const Iterator & o) noexcept;
		private:
			FindFile * m_ff;
		};

		Iterator begin() noexcept;
		Iterator end() noexcept;

	private:
		bool _isDotDir() noexcept;
		byte m_buffer[1024 - sizeof(void*)];

		void * m_handle;
	};

	class MappedFile:public Bufferable<MappedFile, BufferInfo<void, method::Memory, false, false>>
	{
	public:
		// Close file handle when it's destroyed
		MappedFile(File * file) throws(Error, TooBigException);
		MappedFile(const char16 * filename) throws(Error, TooBigException);
		~MappedFile() noexcept;
		size_t $size() const noexcept;
		void * $begin() noexcept;
		void * $end() noexcept;
		const void * $begin() const noexcept;
		const void * $end() const noexcept;

	private:
		Must<File> m_file;
		size_t m_size;
		File::Mapping m_map;

	};


	class DirectoryScanner
	{
	public:
		DirectoryScanner() noexcept;
		template <typename LAMBDA>
		void scan(Text16 path, LAMBDA &&lambda) noexcept;
		template <typename LAMBDA>
		void scanWithThis(Text16 path, LAMBDA &&lambda) noexcept;
		pcstr16 getSzName() noexcept;
		Text16 getPathText() noexcept;
		Text16 getRelativeText(Text16 path) noexcept;
		pcstr16 getRelativeSzName(Text16 path) noexcept;

	private:
		template <typename LAMBDA>
		void _scanOpen(Text16 path, LAMBDA &&lambda) noexcept;

		BText16<File::NAMELEN> m_path;
		pcstr16 m_dircut;
	};

	template <typename LAMBDA>
	void DirectoryScanner::scan(Text16 path, LAMBDA &&lambda) noexcept
	{
		pcstr16 olddircut = m_dircut;
		_scanOpen(path, lambda);
		m_dircut = olddircut;
	}

	template <typename LAMBDA>
	void DirectoryScanner::scanWithThis(Text16 path, LAMBDA &&lambda) noexcept
	{
		pcstr16 olddircut = m_dircut;
		_scanOpen(path, lambda);
		m_path.cut_self(m_dircut - 1);
		lambda(m_path);
		m_dircut = olddircut;
	}

	template <typename LAMBDA>
	void DirectoryScanner::_scanOpen(Text16 path, LAMBDA &&lambda) noexcept
	{
		m_path.cut_self(m_dircut);
		m_path << path;

		m_dircut = m_path.end() + 1;
		m_path << u"/*.*" << nullterm;
		Temp<FindFile> file(m_path.begin());

		if (file->exists()) do
		{
			if (file->isDirectory())
			{
				scanWithThis((Text16)file->getFileName(), lambda);
			}
			else
			{
				m_path.cut_self(m_dircut);
				m_path << (Text16)file->getFileName();
				lambda(m_path);
			}
		}
		while (file->next());
	}

}
