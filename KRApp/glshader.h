#pragma once

#include "globject.h"
#include <KR3/math/coord.h>
#include <KR3/msg/promise.h>

namespace kr
{
	namespace gl
	{
		class Shader:public Object
		{
		public:
			Shader() = default;
			Shader(GLenum type, const GLchar * shaderSrc) noexcept;
			void create(GLenum type, const GLchar * shaderSrc) noexcept;
			Promise<void>* load(GLenum type, Text16 filePath) noexcept;
			void remove() noexcept;
		};

		class VertexShader :public Shader
		{
		public:
			VertexShader() = default;
			VertexShader(const GLchar * shaderSrc) noexcept;
			void create(const GLchar * shaderSrc) noexcept;
		};

		class FragmentShader :public Shader
		{
		public:
			FragmentShader() = default;
			FragmentShader(const GLchar * shaderSrc) noexcept;
			void create(const GLchar * shaderSrc) noexcept;
		};

		class Location
		{
		public:
			Location() = default;
			Location(GLint id) noexcept;
			Location & operator =(GLint id) noexcept;
			GLint getId() noexcept;
			operator GLint() noexcept;

		protected:
			GLint m_id;
		};

		class AttribLocation:public Location
		{
		public:
			using Location::Location;
			using Location::operator =;
			using Location::operator GLint;
			void enable() const noexcept;
			void disable() const noexcept;
			void pointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, uintptr_t offset) const noexcept;
		};

		class UniformLocation :public Location
		{
		public:
			using Location::Location;
			using Location::operator =;
			using Location::operator GLint;

			void setInt(GLint value) noexcept;
			void setFloat(GLfloat value) noexcept;
			void setVector(const vec4 & vector) noexcept;
			void setVector(const vec4a & vector) noexcept;
			void setMatrix(const mat4 & matrix) noexcept;
			void setMatrix(const mat4a & matrix) noexcept;
			void setVector(const vec4 * vector, GLsizei count) noexcept;
			void setVector(const vec4a * vector, GLsizei count) noexcept;
			void setMatrix(const mat4 * matrix, GLsizei count) noexcept;
			void setMatrix(const mat4a * matrix, GLsizei count) noexcept;
			template <typename T>
			void set(const T & v) noexcept;
			template <typename T>
			void set(const T * v, GLsizei count) noexcept;
		};

		class Program :public Object
		{
		public:
			Program() = default;
			Program(Shader vs, Shader fs) noexcept;
			static Program create() noexcept;
			void create(Shader vs, Shader fs) noexcept;
			void remove() noexcept;
			void attach(Shader shader) noexcept;
			void bindAttribLocation(GLuint index, const GLchar * name) noexcept;
			void link() noexcept;
			void use() noexcept;
			GLint getInt(GLenum pname) noexcept;
			GLint getLinkState() noexcept;
			GLint getInfoLogLength() noexcept;
			void getInfoLog(GLsizei bufSize, GLsizei * length, GLchar * infoLog) noexcept;
			AText getInfoLog() noexcept;
			GLint getAttribLocation(const GLchar * name) noexcept;
			GLint getUniformLocation(const GLchar * name) noexcept;
		};
	}
}
