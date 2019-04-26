#pragma once

#include <KR3/main.h>

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace kr
{
	namespace gl
	{
		class Object
		{
		public:
			Object() = default;
			void setId(GLuint id) noexcept;
			GLuint getId() noexcept;

		protected:
			GLuint m_id;
		};

		class Buffer:public Object
		{
		public:
			Buffer() = default;
			void generate() noexcept;
			void generate(kr::Buffer buffer, GLenum target) noexcept;
			void remove() noexcept;
		};

		template <GLenum target>
		class ArrayBufferTarget
		{
		public:
			ArrayBufferTarget & operator =(Buffer buffer) noexcept
			{
				glBindBuffer(target, buffer.getId());
				return *this;
			}
			void data(kr::Buffer buffer, GLenum usage = GL_STATIC_DRAW) noexcept
			{
				glBufferData(target, buffer.size(), buffer.data(), usage);
			}
			void dataDynamic(kr::Buffer buffer) noexcept
			{
				data(buffer, GL_DYNAMIC_DRAW);
			}
			void subdata(GLsizeiptr offset, kr::Buffer buffer, GLenum usage = GL_STATIC_DRAW) noexcept
			{
#ifdef __EMSCRIPTEN__
				glBufferSubData(target, offset, buffer.size(), buffer.data());
#else
				glBufferSubData(target, offset, buffer.size(), buffer.data(), usage);
#endif
			}
			void subdataDynamic(kr::Buffer buffer) noexcept
			{
				subdata(buffer, GL_DYNAMIC_DRAW);
			}
		};
		static ArrayBufferTarget<GL_ARRAY_BUFFER> &arrayBuffer = nullref;
		static ArrayBufferTarget<GL_ELEMENT_ARRAY_BUFFER> &elementArrayBuffer = nullref;
	}
}
