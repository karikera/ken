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


void gl::Buffer::generate() noexcept
{
	glGenBuffers(1, &m_id);
}
void gl::Buffer::generate(kr::Buffer buffer, GLenum target) noexcept
{
	generate();
	glBindBuffer(target, m_id);
	glBufferData(target, buffer.size(), buffer.data(), GL_STATIC_DRAW);
}
void gl::Buffer::remove() noexcept
{
	glDeleteBuffers(1, &m_id);
}
