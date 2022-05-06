#pragma once

#include <KR3/main.h>

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "glutil.h"

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
			bool operator ==(const Object &other) noexcept;
			bool operator !=(const Object &other) noexcept;

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
		class ArrayBufferTarget final
		{
		public:
			const ArrayBufferTarget & operator =(Buffer buffer) const noexcept
			{
				glCheck();
				glBindBuffer(target, buffer.getId());
				glCheck();
				return *this;
			}
			void data(kr::Buffer buffer, GLenum usage = GL_STATIC_DRAW) const noexcept
			{
				glCheck();
				glBufferData(target, buffer.size(), buffer.data(), usage);
				glCheck();
			}
			void data(size_t size, GLenum usage = GL_STATIC_DRAW) const noexcept
			{
				glCheck();
				glBufferData(target, size, nullptr, usage);
				glCheck();
			}
			void dataDynamic(kr::Buffer buffer) const noexcept
			{
				data(buffer, GL_DYNAMIC_DRAW);
			}
			void dataDynamic(size_t size) const noexcept
			{
				data(size, GL_DYNAMIC_DRAW);
			}
			void dataStream(kr::Buffer buffer) const noexcept
			{
				data(buffer, GL_STREAM_DRAW);
			}
			void dataStream(size_t size) const noexcept
			{
				data(size, GL_STREAM_DRAW);
			}
			void subdata(GLsizeiptr offset, kr::Buffer buffer) const noexcept
			{
				glCheck();
				glBufferSubData(target, offset, buffer.size(), buffer.data());
				glCheck();
			}
		};
		static constexpr const ArrayBufferTarget<GL_ARRAY_BUFFER> arrayBuffer = ArrayBufferTarget<GL_ARRAY_BUFFER>();
		static constexpr const ArrayBufferTarget<GL_ELEMENT_ARRAY_BUFFER> elementArrayBuffer = ArrayBufferTarget<GL_ELEMENT_ARRAY_BUFFER>();
	}
}
