#pragma once

#include <KR3/main.h>
#include <KR3/fs/file.h>

typedef struct _OVERLAPPED OVERLAPPED;

namespace kr
{
	class IOState
	{
	protected:
		uintptr_t m_internal;
		uintptr_t m_internalHigh;
		union {
			struct {
				uint32_t m_offset;
				uint32_t m_offsetHigh;
			};
			void* m_pointer;
		};

		void* m_event;
		IOState() noexcept;
		OVERLAPPED* _overlapped() noexcept;
		template <typename T>
		static T* _from(OVERLAPPED * overlapped) noexcept
		{
			MUST_BASE_OF(T, IOState);
			return static_cast<T*>((IOState*)overlapped);
		}
	};
	class DirectoryWatcher:public IOState
	{
	public:
		DirectoryWatcher() noexcept;
		~DirectoryWatcher() noexcept;
		void open(pcstr16 dir, bool subtree) throws(Error);
		void close() noexcept;
		virtual void onCreate(Text16 name) noexcept = 0;
		virtual void onDelete(Text16 name) noexcept = 0;
		virtual void onModified(Text16 name) noexcept = 0;
		virtual void onRename(Text16 oldname, Text16 newname) noexcept = 0;

	private:
		void _request() noexcept;

		bool m_subtree;
		File * m_dir;
		byte m_buffer[8192];
		BText16<File::NAMELEN> m_oldfilename;
	};
}
