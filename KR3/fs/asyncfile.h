#pragma once

#include <KR3/main.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/msg/cascader.h>

namespace kr
{
	class AsyncFile:public Referencable<AsyncFile>, public io::Streamable<AsyncFile>
	{
	public:
		AsyncFile() noexcept;
		AsyncFile(AText16 filename) noexcept;
		~AsyncFile() noexcept;
		void $write(const void * data, size_t size) noexcept;
		void create(AText16 filename) noexcept;
		void close() noexcept;
		
	private:
		ThreadCascader m_cascader;
		File * m_file;
	};

}
