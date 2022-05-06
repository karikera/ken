#include "glshader.h"
#include <KR3/http/fetch.h>

using namespace kr;

gl::Shader::Shader(GLenum type, const GLchar * shaderSrc) noexcept
{
	create(type, shaderSrc);
}
void gl::Shader::create(GLenum type, const GLchar * shaderSrc) noexcept
{
	// Create the shader object
	m_id = glCreateShader(type);
	if (m_id == 0) return;

	// Load the shader source
	glShaderSource(m_id, 1, &shaderSrc, nullptr);

	// Compile the shader
	glCompileShader(m_id);

	// Check the compile status
	GLint compiled;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;

		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			AText buffer;
			buffer.resize(infoLen-1, infoLen);
			glGetShaderInfoLog(m_id, infoLen, nullptr, buffer.data());
			dout << buffer << endl;
		}

		glDeleteShader(m_id);
		m_id = 0;
	}
}
Promise<void>* gl::Shader::load(GLenum type, Text16 filePath) noexcept
{
	return fetch::text(filePath)->then([this, type](AText script){
		create(type, script.c_str());
	});
}
void gl::Shader::remove() noexcept
{
	glDeleteShader(m_id);
}

gl::VertexShader::VertexShader(const GLchar * shaderSrc) noexcept
	:Shader(GL_VERTEX_SHADER, shaderSrc)
{
}
void gl::VertexShader::create(const GLchar * shaderSrc) noexcept
{
	Shader::create(GL_VERTEX_SHADER, shaderSrc);
}

gl::FragmentShader::FragmentShader(const GLchar * shaderSrc) noexcept
	:Shader(GL_FRAGMENT_SHADER, shaderSrc)
{
}
void gl::FragmentShader::create(const GLchar * shaderSrc) noexcept
{
	Shader::create(GL_FRAGMENT_SHADER, shaderSrc);
}

gl::Location::Location(GLint id) noexcept
{
	m_id = id;
}
gl::Location & gl::Location::operator =(GLint id) noexcept
{
	m_id = id;
	return *this;
}
GLint gl::Location::getId() noexcept
{
	return m_id;
}
gl::Location::operator GLint() noexcept
{
	return m_id;
}

void gl::AttribLocation::enable() const noexcept
{
	glCheck();
	glEnableVertexAttribArray(m_id);
	glCheck();
}
void gl::AttribLocation::disable() const noexcept
{
	glCheck();
	glDisableVertexAttribArray(m_id);
	glCheck();
}
void gl::AttribLocation::pointer(GLint size, GLenum type, GLboolean normalized, GLsizei stride, uintptr_t offset) const noexcept
{
	glCheck();
	glVertexAttribPointer(m_id, size, type, normalized, stride, (void*)offset);
	glCheck();
}

void gl::UniformLocation::setInt(GLint value) noexcept
{
	glCheck();
	glUniform1i(m_id, value);
	glCheck();
}
void gl::UniformLocation::setFloat(GLfloat value) noexcept
{
	glCheck();
	glUniform1f(m_id, value);
	glCheck();
}
void gl::UniformLocation::setVector(const vec4 & vector) noexcept
{
	glCheck();
	glUniform4fv(m_id, 1, (GLfloat*)&vector);
	glCheck();
}
void gl::UniformLocation::setVector(const vec4a & vector) noexcept
{
	glCheck();
	glUniform4fv(m_id, 1, (GLfloat*)&vector);
	glCheck();
}
void gl::UniformLocation::setMatrix(const mat4 & matrix) noexcept
{
	glCheck();
	glUniformMatrix4fv(m_id, 1, false, (GLfloat*)&matrix);
	glCheck();
}
void gl::UniformLocation::setMatrix(const mat4a & matrix) noexcept
{
	glCheck();
	glUniformMatrix4fv(m_id, 1, false, (GLfloat*)&matrix);
	glCheck();
}
void gl::UniformLocation::setVector(const vec4 * vector, GLsizei count) noexcept
{
	glCheck();
	glUniform4fv(m_id, 1, (GLfloat*)vector);
	glCheck();
}
void gl::UniformLocation::setVector(const vec4a * vector, GLsizei count) noexcept
{
	glCheck();
	glUniform4fv(m_id, 1, (GLfloat*)vector);
	glCheck();
}
void gl::UniformLocation::setMatrix(const mat4 * matrix, GLsizei count) noexcept
{
	glCheck();
	glUniformMatrix4fv(m_id, 1, false, (GLfloat*)matrix);
	glCheck();
}
void gl::UniformLocation::setMatrix(const mat4a * matrix, GLsizei count) noexcept
{
	glCheck();
	glUniformMatrix4fv(m_id, 1, false, (GLfloat*)matrix);
	glCheck();
}
template <>
void gl::UniformLocation::set<int>(const int& v) noexcept
{
	setInt(v);
}
template <>
void gl::UniformLocation::set<float>(const float& v) noexcept
{
	setFloat(v);
}
template <>
void gl::UniformLocation::set<vec4a>(const vec4a& v) noexcept
{
	setVector(v);
}
template <>
void gl::UniformLocation::set<vec4>(const vec4& v) noexcept
{
	setVector(v);
}
template <>
void gl::UniformLocation::set<mat4>(const mat4& v) noexcept
{
	setMatrix(v);
}
template <>
void gl::UniformLocation::set<mat4a>(const mat4a& v) noexcept
{
	setMatrix(v);
}
template <>
void gl::UniformLocation::set<vec4>(const vec4* v, GLsizei count) noexcept
{
	setVector(v, count);
}
template <>
void gl::UniformLocation::set<vec4a>(const vec4a* v, GLsizei count) noexcept
{
	setVector(v, count);
}
template <>
void gl::UniformLocation::set<mat4>(const mat4* v, GLsizei count) noexcept
{
	setMatrix(v, count);
}
template <>
void gl::UniformLocation::set<mat4a>(const mat4a* v, GLsizei count) noexcept
{
	setMatrix(v, count);
}


gl::Program::Program(Shader vs, Shader fs) noexcept
{
	create(vs, fs);
}
gl::Program gl::Program::create() noexcept
{
	Program program;
	program.m_id = glCreateProgram();
	return program;
}
void gl::Program::create(Shader vs, Shader fs) noexcept
{
	m_id = glCreateProgram();
	attach(vs);
	attach(fs);
	link();
	if (!getLinkState())
	{
		AText info = getInfoLog();
		dout << info << endl;
	}
}
void gl::Program::remove() noexcept
{
	glDeleteProgram(m_id);
}
void gl::Program::attach(Shader shader) noexcept
{
	glAttachShader(m_id, shader.getId());
}
void gl::Program::bindAttribLocation(GLuint index, const GLchar * name) noexcept
{
	glBindAttribLocation(m_id, index, name);
}
void gl::Program::link() noexcept
{
	glLinkProgram(m_id);
}
void gl::Program::use() noexcept
{
	glUseProgram(m_id);
}
GLint gl::Program::getInt(GLenum pname) noexcept
{
	GLint value;
	glGetProgramiv(m_id, pname, &value);
	return value;
}
GLint gl::Program::getLinkState() noexcept
{
	return getInt(GL_LINK_STATUS);
}
GLint gl::Program::getInfoLogLength() noexcept
{
	return getInt(GL_INFO_LOG_LENGTH);
}
void gl::Program::getInfoLog(GLsizei bufSize, GLsizei * length, GLchar * infoLog) noexcept
{
	glGetProgramInfoLog(m_id, bufSize, length, infoLog);
}
AText gl::Program::getInfoLog() noexcept
{
	GLsizei sz = getInfoLogLength();
	if (sz <= 0) return AText();
	AText text;
	text.resize(sz - 1, sz);
	getInfoLog(sz, nullptr, text.data());
	return text;
}
GLint gl::Program::getAttribLocation(const GLchar * name) noexcept
{
	glCheck();
	GLint loc = glGetAttribLocation(m_id, name);
	glCheck();
	return loc;
}
GLint gl::Program::getUniformLocation(const GLchar * name) noexcept
{
	glCheck();
	GLint loc = glGetUniformLocation(m_id, name);
	glCheck();
	return loc;
}
