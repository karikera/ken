#include "shaders.h"
#include <iostream>

using namespace std;

GLuint loadShader(GLenum type, const char *source)
{
	//create a shader
	GLuint shader = glCreateShader(type);
	if (shader == 0)
	{
		cerr << "Error creating shader" << endl;
		return 0;
	}

	//load the shader source to the shader object and compile it
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	//check if the shader compiled successfully
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		cerr << "Shader compilation error" << endl;
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}
GLuint buildProgram(GLuint vertexShader, GLuint fragmentShader)
{
	//create a GL program and link it
	GLuint programObject = glCreateProgram();
	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	glBindAttribLocation(programObject, (GLuint)VertexAttributeLocation::Position, "vPosition");
	glLinkProgram(programObject);

	//check if the program linked successfully
	GLint linked;
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		cerr << "Program link error" << endl;
		glDeleteProgram(programObject);
		return 0;
	}
	return programObject;
}

Shader::Shader(GLenum shader, const char * source) noexcept
{
	m_shader = loadShader(shader, source);
}

Program::Program(VertexShader s1, FragmentShader s2) noexcept
{
	m_program = buildProgram(s1.m_shader, s2.m_shader);
}
void Program::use() noexcept
{
	glUseProgram(m_program);
}

Uniform::Uniform(Program prog, const char * name) noexcept
	:m_program(prog)
{
	m_location = glGetUniformLocation(m_program.m_program, name);
	if(m_location == -1) cout << "cannot found " << name << " uniform"<< endl;
}
template<> void UniformT<float>::set(const float &data) noexcept
{
	glUniform1f(m_location, data);
}
template<> void UniformT<vec2>::set(const vec2 &data) noexcept
{
	glUniform2fv(m_location, 1, (GLfloat*)&data);
}
template<> void UniformT<vec3>::set(const vec3 &data) noexcept
{
	glUniform3fv(m_location, 1, (GLfloat*)&data);
}
template<> void UniformT<vec4>::set(const vec4 &data) noexcept
{
	glUniform4fv(m_location, 1, (GLfloat*)&data);
}
template<> void UniformT<mat3>::set(const mat3 &data) noexcept
{
	glUniformMatrix3fv(m_location, 1, false, (GLfloat*)&data);
}
template<> void UniformT<mat4>::set(const mat4 &data) noexcept
{
	glUniformMatrix4fv(m_location, 1, false, (GLfloat*)&data);
}
