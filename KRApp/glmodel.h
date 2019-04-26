#pragma once

#include <KR3/main.h>
#include <KRMessage/promise.h>
#include "fbx/fbxdoc.h"

#include "glutil.h"
#include "globject.h"
#include "glshader.h"
#include "gltexture.h"
#include "glrenderprog.h"

namespace kr
{
	namespace gl
	{
		using index_t = uint32_t;

		using MeshRenderProg = RenderProg<attrib::pos, attrib::uv, uniform::transform, uniform::texture>;
		using MeshVertex = MeshRenderProg::Vertex;

		struct Mesh
		{
			uint32_t ioff;
			uint32_t isize;
		};
		
		class ModelBuilder;

		class Model
		{
			friend ModelBuilder;
		public:
			Model() noexcept;
			~Model() noexcept;
			Model(Model && _move) noexcept;
			Model & operator =(Model&& model)  noexcept;
			Buffer getVertexBuffer() noexcept;
			Buffer getIndexBuffer() noexcept;
			void bind() noexcept;
			void draw() noexcept;
			void gen(const fbx::FBXDoc & doc) noexcept;
			void genSingleMesh(View<MeshVertex> vb, View<index_t> ib) noexcept;
			Promise<void>* load(Text16 url) noexcept;
			template <template <typename> class ... LOCS>
			void bind(RenderProg<LOCS ...>& renderprog) noexcept
			{
				bind();
				renderprog.template pointer<MeshVertex>();
			}

		private:
			Array<Mesh> m_mesh;
			Buffer m_vb, m_ib;
		};

		class ModelBuilder
		{
		public:
			ModelBuilder() noexcept;
			View<MeshVertex> vertexData() noexcept;
			View<index_t> indexData() noexcept;
			index_t* prepareIndex() noexcept;
			MeshVertex* prepareVertex() noexcept;
			void commitMesh() noexcept;
			void reserveMesh(size_t size) noexcept;
			Model build() noexcept;

		private:
			Array<Mesh> m_mesh;
			Array<index_t> m_ibuf;
			Array<MeshVertex> m_vbuf;
			uint32_t m_vertexMark;
			index_t m_indexMark;
		};

	}
}
