#include "globject.h"

using namespace kr;

void gl::Object::setId(GLuint id) noexcept
{
	m_id = id;
}
GLuint gl::Object::getId() noexcept
{
	return m_id;
}
bool gl::Object::operator ==(const Object& other) noexcept
{
	return m_id == other.m_id;
}
bool gl::Object::operator !=(const Object& other) noexcept
{
	return m_id != other.m_id;
}


void gl::Buffer::generate() noexcept
{
	glCheck();
	glGenBuffers(1, &m_id);
	glCheck();
}
void gl::Buffer::generate(kr::Buffer buffer, GLenum target) noexcept
{
	generate();
	glBindBuffer(target, m_id);
	glCheck();
	glBufferData(target, buffer.size(), buffer.data(), GL_STATIC_DRAW);
	glCheck();
}
void gl::Buffer::remove() noexcept
{
	glCheck();
	glDeleteBuffers(1, &m_id);
	glCheck();
}
