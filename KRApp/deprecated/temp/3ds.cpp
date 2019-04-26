#include "3ds.h"
#include "const.h"
#include "fileio.h"

#include <cstdlib>
#include <iomanip>
#include <string.h>
#include <GLES2\gl2.h>

using std::cout;
using std::cerr;
using std::endl;

inline void glTestError()
{
	int err;
	for (;;)
	{
		err = glGetError();
		if (err == GL_NO_ERROR) break;
		cerr << err << endl;
	}
}

class Loader3ds::Chunk
{
public:
	Chunk(File * file) noexcept;
	~Chunk() noexcept;
	uint32_t left() const noexcept;

	uint16_t mID;
	uint32_t mLength;
	uint32_t mBytesRead;
};

Model3ds::Model3ds() noexcept
{
	glGenBuffers(1, &m_ib);
	glGenBuffers(1, &m_vb);
}
Model3ds::~Model3ds() noexcept
{
	glDeleteBuffers(1, &m_ib);
	glDeleteBuffers(1, &m_vb);
}
void Model3ds::setIndexBuffer(const Buffer & buffer) noexcept
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

	size_t size = buffer.size();
	char * temp = new char[size];
	buffer.copyTo(temp, size);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, temp, GL_STATIC_DRAW);
	delete[] temp;

	m_ibcount = size / sizeof(vindex_t);
}
void Model3ds::setVertexBuffer(const Buffer & buffer) noexcept
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vb);
	
	size_t size = buffer.size();
	char * temp = new char[size];
	buffer.copyTo(temp, size);
	glBufferData(GL_ARRAY_BUFFER, size, temp, GL_STATIC_DRAW);
	delete[] temp;
}
void Model3ds::draw() noexcept
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glBindBuffer(GL_ARRAY_BUFFER, m_vb);

	GLsizei stride = sizeof(float)*3;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glDrawElements(GL_TRIANGLES, m_ibcount, GL_UNSIGNED_SHORT, 0);
}
void Model3ds::drawLine() noexcept
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glBindBuffer(GL_ARRAY_BUFFER, m_vb);

	GLsizei stride = sizeof(float)*3;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	for (size_t i = 0; i < m_ibcount; i += 3)
	{
		glDrawElements(GL_LINE_STRIP, 3, GL_UNSIGNED_SHORT, (void*)(i*sizeof(vindex_t)));
	//	glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_SHORT, (void*)(i*sizeof(vindex_t)));
	}
}


Loader3ds::Loader3ds() noexcept
{
}
Loader3ds::~Loader3ds() noexcept
{
}

int Loader3ds::load(Model3ds * dest, const char * aFilename) noexcept
{
	try
	{
		{
			File file(aFilename);
			m_file = &file;

			{
				Chunk chunk(m_file);
				if (chunk.mID != PRIMARY)
					throw (int)1107;

				_processNextChunk(&chunk);
			}

			dest->setIndexBuffer(m_ib);
			dest->setVertexBuffer(m_vb);

			m_file = nullptr;
		}

		m_vb.clear();
		m_ib.clear();
		m_tb.clear();
		m_vr.clear();
		m_ir.clear();

		return 0;
	}
	catch (int err)
	{
		return err;
	}
}
void Loader3ds::_processNextChunk(Chunk * parent) // int
{
	while (parent->mBytesRead < parent->mLength)
	{
		// Read next chunk
		Chunk chunk(m_file);
		
		switch (chunk.mID)
		{
		case VERSION:
			if (_read<uint16_t>(&chunk) > VERSION_OF_3DS) throw (int)1107;
			break;

		case EDITMATERIAL:
			_processNextMaterialChunk(&chunk);
			break;

		case EDIT3DS:
			{
				Chunk chunk2(m_file);
				if (chunk2.mID != MESHVERSION) throw (int)1107;
				if(_read<uint16_t>(&chunk2) > VERSION_OF_3DS) throw (int)1107;
				_skipAll(&chunk2);
				chunk.mBytesRead += chunk2.mBytesRead;
			}
			_processNextChunk(&chunk);
			break;

		case EDITOBJECT:
			chunk.mBytesRead += _readString((char*)m_buffer, BUFFER_SIZE);
			_processNextObjectChunk(&chunk);
			break;

		case EDITKEYFRAME:
			_processNextKeyFrameChunk(&chunk);
			break;
		}
		
		_skipAll(&chunk);
		parent->mBytesRead += chunk.mBytesRead;
	}
}
void Loader3ds::_processNextObjectChunk(Chunk * parent) // int
{
	while (parent->mBytesRead < parent->mLength)
	{
		Chunk chunk(m_file);		
		switch (chunk.mID)
		{
		case OBJTRIMESH:
			_processNextObjectChunk(&chunk);
			_computeNormals();
			break;

		case TRIVERT:
			_fillVertexBuffer(&chunk);
			break;

		case TRIFACE:
			_fillIndexBuffer(&chunk);
			break;

		case TRIFACEMAT:
			_sortIndicesByMaterial(&chunk);
			break;

		case TRIUV:
			_fillTexCoordBuffer(&chunk);
			break;
		}
		
		_skipAll(&chunk);
		parent->mBytesRead += chunk.mBytesRead;
	}
}
void Loader3ds::_processNextMaterialChunk(Chunk * parent) noexcept
{
	while (parent->mBytesRead < parent->mLength)
	{
		Chunk chunk(m_file);

		switch (chunk.mID)
		{
		case MATNAME:
			_skipAll(&chunk);
			break;

		case MATLUMINANCE:
			chunk.mBytesRead += _readColorChunk();
			break;

		case MATDIFFUSE:
			chunk.mBytesRead += _readColorChunk();
			break;

		case MATSPECULAR:
			chunk.mBytesRead += _readColorChunk();
			break;
		
		case MATSHININESS:
			chunk.mBytesRead += _readColorChunk();
			break;
		
		case MATMAP:
			// texture map chunk, proceed
			_processNextMaterialChunk(&chunk);
			break;

		case MATMAPFILE:
			_skipAll(&chunk);
			// mBuffer now contains the filename of the next texture; load it if you wish
			break;
		}
		_skipAll(&chunk);
		parent->mBytesRead += chunk.mBytesRead;
	}
}
void Loader3ds::_processNextKeyFrameChunk(Chunk * parent) noexcept
{
	uint16_t lCurrentID, lCurrentParentID;
	
	while (parent->mBytesRead < parent->mLength)
	{
		Chunk chunk(m_file);
		switch (chunk.mID)
		{
		case KFMESH:
			_processNextKeyFrameChunk(&chunk);
			break;

		case KFHEIRARCHY:
			lCurrentID = _read<uint16_t>(&chunk);
			// lCurrentID now contains the ID of the current object being described
			//  Save this if you want to support an object hierarchy
			break;

		case KFNAME:
			chunk.mBytesRead += m_file->getString((char *)m_buffer, BUFFER_SIZE);
			// mBuffer now contains the name of the object whose KF info will
			//   be described

			_skip(&chunk, 4); // useless, ignore
			lCurrentParentID = _read<uint16_t>(&chunk);
			// lCurrentParentID now contains the ID of the parent of the current object 
			// being described
			break;
		}
		_skipAll(&chunk);
		parent->mBytesRead += chunk.mBytesRead;
	}
}

void Loader3ds::_fillIndexBuffer(Chunk * chunk)  // int
{
	uint16_t lNumFaces = _read<uint16_t>(chunk);
	
	size_t offset = m_ib.size();
	if (offset > 0x7fffffff)
	{
		cerr << "Too big data" << endl;
		throw (int)ENOTSUP;
	}
	m_ir.append({(uint32_t)offset, lNumFaces });

	while (lNumFaces != 0)
	{
		m_ib.append(m_file, sizeof(vindex_t)*3);
		chunk->mBytesRead += sizeof(vindex_t)*3;
	
		uint16_t flags = _read<uint16_t>(chunk);
		lNumFaces--;
	}
}
void Loader3ds::_sortIndicesByMaterial(Chunk * chunk) noexcept
{
	chunk->mBytesRead += m_file->getString((char *) m_buffer, BUFFER_SIZE);
	// mBuffer contains the name of the material that is associated
	//  with the following triangles (set of 3 indices which index into the vertex list
	//  of the current object chunk)
	
	uint16_t lNumFaces = _read<uint16_t>(chunk);

	//chunk->mBytesRead += fread(mBuffer, 1, chunk->mLength - chunk->mBytesRead, mFile);
	// mBuffer now contains a list of triangles that use the material specified above
}
void Loader3ds::_fillTexCoordBuffer(Chunk * chunk) noexcept
{
	uint32_t lNumTexCoords = _read<uint32_t>(chunk);
	uint32_t left = chunk->left();
	m_tb.append(m_file,left);
	chunk->mBytesRead += left;

	// mBuffer now contains a list of UV coordinates (2 floats)
}
void Loader3ds::_fillVertexBuffer(Chunk * chunk) // int
{
	uint16_t lNumVertices = _read<uint16_t>(chunk);

	size_t offset = m_vb.size();
	if (offset > 0x7fffffff)
	{
		cerr << "Too big data" << endl;
		throw (int)ENOTSUP;
	}
	m_vr.append({ (uint32_t)offset, lNumVertices });
	
	size_t vsize = lNumVertices * (sizeof(float) * 3);
	m_vb.append(m_file, vsize);
	chunk->mBytesRead += vsize;
}
void Loader3ds::_computeNormals() noexcept
{
/*
Compute your normals here. Quick way of doing it (no smoothing groups, though):

  for (each vertex in vertex list)
	{
	for (each triangle in triangle list)
	  {
	  if (vertex is in triangle)
	    {
		compute normal of triangle
		add to total_normal
		}
	  }
	normalize(total_normal)
	save_to_normal_list(total_normal)
	total_normal = 0
	}
	
This seemed to work well for me, though it's my first time having to compute
normals myself. One might think that normalizing the normal right after it's 
computed and added to total_normal would be best, but I've found this not to be
the case. If this is done, then small triangles have just as much influence on the 
final normal as larger triangles. My way, the model comes out looking much more 
smooth, especially if there's small flaws in the model. 
*/
}

uint32_t Loader3ds::_readColorChunk() noexcept
{
	Chunk chunk(m_file);
	_skipAll(&chunk);
	
	// TODO: color chunk

	return chunk.mBytesRead;
}
uint32_t Loader3ds::_readPercentChunk(float * dest) noexcept
{
	Chunk chunk(m_file);
	int16_t value = _read<int16_t>(&chunk);
	_skipAll(&chunk);
	*dest = (float)value / 100.0f;
	return chunk.mBytesRead;
}
uint32_t Loader3ds::_readString(char * dest, size_t capacity) noexcept
{
	return m_file->getString(dest, capacity);
}
void Loader3ds::_read(Chunk * chunk, void * dest, size_t size) noexcept
{
	uint32_t left = chunk->left();
	if (size > left)
	{
		cerr << "chunk size overflow" << endl;
		size = left;
	}
	m_file->read(dest, size);
	chunk->mBytesRead += size;
}
void Loader3ds::_readAll(Chunk * chunk, void * dest, size_t limit) // int
{
	uint32_t left = chunk->left();
	if (limit < left)
	{
		cerr << "temp memory size overflow" << endl;
		left = limit;
	}
	m_file->read(dest,left);
	chunk->mBytesRead = chunk->mLength;
}
void Loader3ds::_skip(Chunk * chunk, size_t size) noexcept
{
	m_file->skip(size);
	chunk->mBytesRead += size;
}
void Loader3ds::_skipAll(Chunk * chunk) noexcept
{
	uint32_t leftsize = chunk->left();
	if (leftsize == 0) return;
	m_file->skip(leftsize);
	chunk->mBytesRead = chunk->mLength;
}

Loader3ds::Chunk::Chunk(File * m_file) noexcept
{
	m_file->read(&mID);
	m_file->read(&mLength);
	mBytesRead = sizeof(mID) + sizeof(mLength);
}
Loader3ds::Chunk::~Chunk() noexcept
{
}
uint32_t Loader3ds::Chunk::left() const noexcept
{
	return mLength - mBytesRead;
}