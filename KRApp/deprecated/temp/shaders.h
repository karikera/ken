#pragma once

#include <GLES2/gl2.h>

#include "math/matrix.h"

GLuint loadShader(GLenum type, const char *source);
GLuint buildProgram(GLuint vertexShader, GLuint fragmentShader);

class Shader;
class Program;
class Uniform;

template <GLenum type> class ShaderT;
using VertexShader = ShaderT<GL_VERTEX_SHADER>;
using FragmentShader = ShaderT<GL_FRAGMENT_SHADER>;

enum class VertexAttributeLocation
{
	Position,
};

class Shader
{
	friend Program;
public:
	Shader(GLenum shader, const char * source) noexcept;

private:
	GLuint m_shader;
};

class Program
{
	friend Uniform;
public:
	Program(VertexShader s1, FragmentShader s2) noexcept;
	void use() noexcept;

private:
	GLuint m_program;
};

template <GLenum type> class ShaderT:public Shader
{
public:
	ShaderT(const char * source) noexcept
		: Shader(type, source)
	{
	}
};

template <typename T> class UniformT;

class Uniform
{
public:
	Uniform(Program prog, const char * name) noexcept;

protected:
	Program m_program;
	GLuint m_location;
};

template <typename T> class UniformT:public Uniform
{
public:
	using Uniform::Uniform;
	void set(const T &data) noexcept;

	UniformT& operator =(const T& data) noexcept
	{
		set(data);
		return *this;
	}

};