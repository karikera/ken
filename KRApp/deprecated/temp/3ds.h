#pragma once

#include <stdio.h>
#include <GLES2/gl2.h>
#include <inttypes.h>

#include "buffer.h"

class File;

using vindex_t = unsigned short;

class Model3ds
{
public:
	Model3ds() noexcept;
	~Model3ds() noexcept;

	void setIndexBuffer(const Buffer & buffer) noexcept;
	void setVertexBuffer(const Buffer & buffer) noexcept;
	void draw() noexcept;
	void drawLine() noexcept;

	GLuint m_ib;
	GLuint m_vb;
	int m_ibcount;
};

class Loader3ds
{
public:
	static constexpr size_t BUFFER_SIZE = 2048;

	Loader3ds() noexcept;
	~Loader3ds() noexcept;
	int load(Model3ds * dest, const char * aFilename) noexcept;

private:
	class Chunk;
	struct Range
	{
		uint32_t offset;
		uint32_t range;
	};
	
	void _processNextChunk(Chunk * parent); // int
	void _processNextObjectChunk(Chunk * parent); // int
	void _processNextMaterialChunk(Chunk * parent) noexcept;
	void _processNextKeyFrameChunk(Chunk * parent) noexcept;

	void _fillVertexBuffer(Chunk * chunk); // int
	void _fillIndexBuffer(Chunk * chunk); // int
	void _fillTexCoordBuffer(Chunk * chunk) noexcept;
	void _sortIndicesByMaterial(Chunk * chunk) noexcept;
	
	void _computeNormals() noexcept;
	
	uint32_t _readColorChunk() noexcept;
	uint32_t _readPercentChunk(float * dest) noexcept;
	uint32_t _readString(char * dest, size_t capacity) noexcept;

	void _read(Chunk * chunk, void * dest, size_t size) noexcept;
	template <typename T> T _read(Chunk * chunk) noexcept;
	void _readAll(Chunk * chunk, void * dest, size_t limit); // int
	void _skip(Chunk * chunk, size_t size) noexcept;
	void _skipAll(Chunk * chunk) noexcept;
	
	File * m_file;
	char m_buffer[BUFFER_SIZE];

	Buffer m_vb;
	Buffer m_ib;
	Buffer m_tb;
	BufferT<Range> m_vr;
	BufferT<Range> m_ir;
};

template <typename T>
inline T Loader3ds::_read(Chunk * chunk) noexcept
{
	T value;
	_read(chunk,&value, sizeof(value));
	return value;
}
