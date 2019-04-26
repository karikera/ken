#include "glmodel.h"
#include <KR3/data/map.h>

using namespace kr;
using namespace gl;
using namespace fbx;

namespace
{
	struct PropViews
	{
		View<FBXProperty> vertices;
		View<FBXProperty> indices;
		View<FBXProperty> uv;
		View<FBXProperty> uvi;
	};

	struct IndexKey
	{
		int32_t pos;
		int32_t uv;

		bool operator ==(const IndexKey & other) const noexcept
		{
			return pos == other.pos && uv == other.uv;
		}
		bool operator !=(const IndexKey & other) const noexcept
		{
			return pos == other.pos && uv == other.uv;
		}
	};

};

template <>
struct std::hash<IndexKey>
{
	size_t operator()(const IndexKey & key) const noexcept
	{
		if (sizeof(uintptr_t) >= 8)
		{
			return (uintptr_t)(((uint64_t)key.pos << 32) | key.uv);
		}
		return (key.pos << 16) ^ (key.pos >> 16) ^ key.uv;
	}
};

Model::Model() noexcept
{
	m_ib.setId(0);
	m_vb.setId(0);
}
Model::~Model() noexcept
{
	if (m_ib.getId()) m_ib.remove();
	if (m_vb.getId()) m_vb.remove();
}
Model::Model(Model&& _move) noexcept
{
	m_ib = _move.m_ib;
	_move.m_ib.setId(0);

	m_vb = _move.m_vb;
	_move.m_vb.setId(0);

	m_mesh = move(_move.m_mesh);
}
Model& Model::operator =(Model&& model)  noexcept
{
	this->~Model();
	new(this) Model(move(model));
	return *this;
}
gl::Buffer Model::getVertexBuffer() noexcept
{
	return m_vb;
}
gl::Buffer Model::getIndexBuffer() noexcept
{
	return m_ib;
}
void Model::bind() noexcept
{
	arrayBuffer = m_vb;
	elementArrayBuffer = m_ib;
}
void Model::draw() noexcept
{
	for (Mesh & mesh : m_mesh)
	{
		glDrawElements(GL_TRIANGLES, mesh.isize, GL_UNSIGNED_INT, (void*)(uintptr_t)mesh.ioff);
	}
}
void Model::gen(const FBXDoc & doc) noexcept
{
	/*
	# This is a nightmare. FBX SDK uses Maya way to compute the transformation matrix of a node - utterly simple:
	#
	#     WorldTransform = ParentWorldTransform * T * Roff * Rp * Rpre * R * Rpost * Rp-1 * Soff * Sp * S * Sp-1
	#
	# Where all those terms are 4 x 4 matrices that contain:
	#     WorldTransform: Transformation matrix of the node in global space.
	#     ParentWorldTransform: Transformation matrix of the parent node in global space.
	#     T: Translation
	#     Roff: Rotation offset
	#     Rp: Rotation pivot
	#     Rpre: Pre-rotation
	#     R: Rotation
	#     Rpost: Post-rotation
	#     Rp-1: Inverse of the rotation pivot
	#     Soff: Scaling offset
	#     Sp: Scaling pivot
	#     S: Scaling
	#     Sp-1: Inverse of the scaling pivot
	#
	# But it was still too simple, and FBX notion of compatibility is... quite specific. So we also have to
	# support 3DSMax way:
	#
	#     WorldTransform = ParentWorldTransform * T * R * S * OT * OR * OS
	#
	# Where all those terms are 4 x 4 matrices that contain:
	#     WorldTransform: Transformation matrix of the node in global space
	#     ParentWorldTransform: Transformation matrix of the parent node in global space
	#     T: Translation
	#     R: Rotation
	#     S: Scaling
	#     OT: Geometric transform translation
	#     OR: Geometric transform rotation
	#     OS: Geometric transform translation
	#
	# Notes:
	#     Geometric transformations ***are not inherited***: ParentWorldTransform does not contain the OT, OR, OS
	#     of WorldTransform's parent node.
	#
	# Taken from http://download.autodesk.com/us/fbx/20112/FBX_SDK_HELP/index.html?url=WS1a9193826455f5ff1f92379812724681e696651.htm,topicNumber=d0e7429
	*/

	TmpArray<PropViews> props;

	FBXNode & objects = doc["Objects"];
	for (auto & model : objects.getChildren("Geometry"))
	{
		try
		{
			auto & vertices = model["Vertices"];
			auto & indices = model["PolygonVertexIndex"];

			auto v = vertices.getProperties();
			auto i = indices.getProperties();

			props.push({ v, i, nullptr, nullptr });
		}
		catch (NotFoundException&)
		{
		}
	}
	for (auto & model : objects.getChildren("Model"))
	{
		try
		{
			auto & vertices = model["Vertices"];
			auto & indices = model["PolygonVertexIndex"];
			auto & uvlayer = model["LayerElementUV"];
			auto & uv = uvlayer["UV"];
			auto & uvi = uvlayer["UVIndex"];

			auto v = vertices.getProperties();
			auto i = indices.getProperties();
			auto uv_prop = uv.getProperties();
			auto uvi_prop = uvi.getProperties();

			props.push({ v, i, uv_prop, uvi_prop });

			_assert(i.size() == uvi_prop.size());
		}
		catch (NotFoundException&)
		{
		}
	}
	
	Map<IndexKey, index_t> map;
	ModelBuilder builder;
	builder.reserveMesh(props.size());

	index_t vbuf_index = 0;
	auto put = [&](const auto & read_index, const auto & read_vertex, bool * is_last){
		IndexKey key;
		read_index(&key);

		_assert(key.uv >= -1);
		if ((*is_last = (key.pos < 0)))
		{
			key.pos = -key.pos - 1;
		}
		if (key.uv == -1)
		{
			key.uv = key.pos;
		}

		auto res = map.insert({ key, 0 });
		if (res.second)
		{
			index_t index = vbuf_index++;
			res.first->second = index;

			MeshVertex * v = builder.prepareVertex();
			read_vertex(v, &key);
			return index;
		}
		else
		{
			return res.first->second;
		}
	};
	auto readVertex = [&](const auto & while_true, const auto & read_index, const auto & read_vertex) {
		while (while_true())
		{
			bool is_last;
			index_t a = put(read_index, read_vertex, &is_last);
			_assert(!is_last);
			index_t b = put(read_index, read_vertex, &is_last);
			_assert(!is_last);
			for (;;)
			{
				index_t c = put(read_index, read_vertex, &is_last);
				index_t * dest = builder.prepareIndex();
				*dest++ = a;
				*dest++ = b;
				*dest++ = c;
				if (is_last) break;
				b = c;
			}
		}
	};
	
	for (auto & prop : props)
	{
		if (prop.indices.empty()) continue;

		if (prop.indices[0].getType() == 'i')
		{
			const double * posptr = prop.vertices[0].as<View<double>>().data();
			View<int32_t> idxview = prop.indices[0].as<View<int32_t>>();
			View<int32_t> pos_indices = prop.indices[0].as<View<int32_t>>();

			if (prop.uv == nullptr || prop.uvi == nullptr)
			{
				readVertex(
					[&] { return !pos_indices.empty(); },
					[&](IndexKey * ib) {
						ib->pos = pos_indices.read();
						ib->uv = -1;
					},
					[&](MeshVertex * v, IndexKey * ib) {
						const double * read;
						read = posptr + ib->pos * 3;
						v->uv.x = v->pos.x = (float)(*read++);
						v->uv.y = v->pos.y = (float)(*read++);
						v->pos.z = (float)(*read);
					}
				);
			}
			else
			{
				const double * uvptr = prop.uv[0].as<View<double>>().data();
				View<int32_t> uv_indices = prop.uvi[0].as<View<int32_t>>();

				readVertex(
					[&]{ return !pos_indices.empty(); },
					[&](IndexKey * ib) { 
						ib->pos = pos_indices.read(); 
						ib->uv = uv_indices.read();
					},
					[&](MeshVertex * v, IndexKey * ib) {
						const double * read;
						read = posptr + ib->pos * 3;
						v->pos.x = (float)(*read++);
						v->pos.y = (float)(*read++);
						v->pos.z = (float)(*read);

						read = uvptr + ib->pos * 2;
						v->uv.x = (float)(*read++);
						v->uv.y = (float)(*read);
					}
				);
			}
		}
		else
		{
			auto pos_indices = prop.indices;
			auto uv_indices = prop.uvi;
			const FBXProperty * posptr = prop.vertices.data();
			const FBXProperty * uvptr = prop.uv.data();

			readVertex(
				[&]{ return !pos_indices.empty(); },
				[&](IndexKey * ib) { 
					ib->pos = (*pos_indices++).as<int32_t>(); 
					ib->uv = (*uv_indices++).as<int32_t>();
				},
				[&](MeshVertex * v, IndexKey * ib) {
					const FBXProperty * read;
					read = posptr + ib->pos * 3;
					v->pos.x = (float)(read++)->as<double>();
					v->pos.y = (float)(read++)->as<double>();
					v->pos.z = (float)(read)->as<double>();

					read = uvptr + ib->pos * 2;
					v->uv.x = (float)(read++)->as<double>();
					v->uv.y = (float)(read)->as<double>();
				}
			);
		}

		builder.commitMesh();
	}

	*this = builder.build();
}
void Model::genSingleMesh(View<MeshVertex> vb, View<index_t> ib) noexcept
{
	if (m_vb.getId()) m_vb.remove();
	m_vb.generate(vb.cast<void>(), GL_ARRAY_BUFFER);
	if (m_ib.getId()) m_ib.remove();
	m_ib.generate(ib.cast<void>(), GL_ELEMENT_ARRAY_BUFFER);
	m_mesh = nullptr;
	Mesh * mesh = m_mesh.prepare(1);
	mesh->ioff = 0;
	mesh->isize = intact<uint32_t>(ib.size());
}
Promise<void>* Model::load(Text16 url) noexcept
{
	return FBXDoc::load(url)->then([this](FBXDoc &doc){ gen(doc); });
}

ModelBuilder::ModelBuilder() noexcept
{
	m_indexMark = 0;
	m_vertexMark = 0;
}
View<MeshVertex> ModelBuilder::vertexData() noexcept
{
	return m_vbuf;
}
View<index_t> ModelBuilder::indexData() noexcept
{
	return m_ibuf;
}
index_t* ModelBuilder::prepareIndex() noexcept
{
	return m_ibuf.prepare(3);
}
MeshVertex* ModelBuilder::prepareVertex() noexcept
{
	return m_vbuf.prepare(1);
}
void ModelBuilder::commitMesh() noexcept
{
	index_t now_total = intact<index_t>(m_ibuf.size());
	m_mesh.push({ m_indexMark, now_total - m_indexMark });
	m_indexMark = now_total;
}
void ModelBuilder::reserveMesh(size_t size) noexcept
{
	m_mesh.reserve(size);
}
Model ModelBuilder::build() noexcept
{
	Model model;
	model.m_vb.generate(m_vbuf.cast<void>(), GL_ARRAY_BUFFER);
	model.m_ib.generate(m_ibuf.cast<void>(), GL_ELEMENT_ARRAY_BUFFER);
	model.m_mesh = move(m_mesh);
	m_vbuf = nullptr;
	m_ibuf = nullptr;
	return model;
}

