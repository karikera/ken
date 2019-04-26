#pragma once

#include "assimp.h"

inline void cbs::Model::Pose::_resize(size_t size)
{
	m_transforms.resizeWithoutKeep(size);
}
inline cbs::Model::Pose::Pose()
{
}
inline cbs::Model::Pose::~Pose()
{
}
inline cbs::Model::Pose::Pose(const Pose & _copy)
{
	m_transforms = _copy.m_transforms;
}
inline cbs::Model::Pose::Pose(Pose && _move)
{
	m_transforms = std::move(_move.m_transforms);
}
inline cbs::Model::Pose& cbs::Model::Pose::operator =(const Pose& _copy)
{
	this->~Pose();
	new(this) Pose(_copy);
	return *this;
}
inline cbs::Model::Pose& cbs::Model::Pose::operator =(Pose&& _move)
{
	this->~Pose();
	new(this) Pose(std::move(_move));
	return *this;
}

inline void cbs::Model::Pose::clear()
{
	m_transforms.remove();
}
inline void cbs::Model::Pose::setNodeTransform(size_t idx, const cbs::Matrix & m)
{
	assert(idx < m_transforms.size());
	m_transforms[idx] = m;
}
inline const cbs::Matrix & cbs::Model::Pose::getNodeTransform(size_t idx) const
{
	assert(idx < m_transforms.size());
	return m_transforms[idx];
}
inline void cbs::Model::Pose::transform(const cbs::Matrix & m)
{
	Matrix tm = m.getTranspose();
	for (size_t i = 0; i < m_transforms.size(); i++)
	{
		Matrix& dest = m_transforms[i];
		dest = tm * dest;
	}
}
inline bool cbs::Model::Pose::set(AnimationStatus * status)
{
	assert(status != nullptr);
	assert(status->animation != nullptr);

	AnimationReader reader;
	reader.m_index = status->animation->m_index;
	reader.m_pose = this;
	double tps = status->animation->m_tps;
	double time = status->time;

	if (time >= status->animation->m_duration)
	{
		time = fmod(time, status->animation->m_duration);
		status->overed = true;
	}
	else if (time < 0)
	{
		time = fmod(time, status->animation->m_duration) + status->animation->m_duration;
		status->overed = true;
	}
	else
	{
		status->overed = false;
	}
	reader.m_time = tps * time;
	reader.m_duration = tps * status->animation->m_duration;

	_resize(status->animation->m_nodeCount);

	reader.getAnimation(status->animation->m_root, XMMatrixIdentity());
	status->time = time;
	return true;
}

inline cbs::Model::Pose& cbs::Model::Pose::operator *= (float weight)
{
	vec * dst = (vec*)m_transforms.data();
	vec * end = dst + m_transforms.size() * (sizeof(Matrix) / sizeof(vec));
	while (dst != end)
	{
		*dst = XMVectorScale(*dst, weight);
		dst++;
	}
	return *this;
}
inline cbs::Model::Pose& cbs::Model::Pose::operator += (const Pose & other)
{
	assert(m_transforms.size() == other.m_transforms.size());

	vec * dst = (vec*)m_transforms.data();
	vec * end = dst + m_transforms.size() * (sizeof(Matrix) / sizeof(vec));
	const vec * src = (vec*)other.m_transforms.data();

	while (dst != end)
	{
		*dst = XMVectorAdd(*dst, *src);
		dst++;
		src++;
	}
	return *this;
}
inline cbs::Model::Pose& cbs::Model::Pose::operator *= (const cbs::Matrix& m)
{
	Matrix * dst = m_transforms.data();
	Matrix * end = dst + m_transforms.size();

	while (dst != end)
	{
		*dst *= m;
	}
	return *this;
}

inline cbs::Model::Pose cbs::Model::Pose::operator * (float weight) const
{
	Pose npose = *this;
	npose *= weight;
	return npose;
}
inline const cbs::Model::Pose cbs::Model::Pose::operator + (const Pose & other) const
{
	Pose npose = *this;
	npose += other;
	return npose;
}
inline const cbs::Model::Pose cbs::Model::Pose::operator * (const cbs::Matrix& m) const
{
	Pose tmp = *this;
	return tmp *= m;
}

inline cbs::Model::Animation::Animation()
	: m_nodeCount(0), m_animation(nullptr), m_index(0), m_root(nullptr), m_tps(DEFAULT_TICK_PER_SECOND)
{
}
inline double cbs::Model::Animation::getDuration() const
{
	return m_duration;
}
inline double cbs::Model::Animation::getTPS() const
{
	return m_tps;
}

inline cbs::Model::NodeExtra::NodeExtra(size_t id)
	:m_id(id), m_nodeanim(nullptr)
{
}
inline cbs::Model::NodeExtra::~NodeExtra()
{
}
inline size_t cbs::Model::NodeExtra::getId()
{
	return m_id;
}
inline bool cbs::Model::NodeExtra::hasAnimation()
{
	return m_nodeanim != nullptr;
}
inline void cbs::Model::NodeExtra::setAnimationCount(size_t count)
{
	m_nodeanim = _new aiNodeAnim*[count];
	memset(m_nodeanim, 0, sizeof(aiNodeAnim*) * count);
}
inline void cbs::Model::NodeExtra::setAnimation(size_t id, aiNodeAnim * anim)
{
	m_nodeanim[id] = anim;
}
inline aiNodeAnim * cbs::Model::NodeExtra::getAnimation(size_t id)
{
	if (m_nodeanim == nullptr) return nullptr;
	return m_nodeanim[id];
}

template <typename LAMBDA>
inline void cbs::Model::_callEachNode(aiNode * node, LAMBDA &lambda)
{
	lambda(node);
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		_callEachNode(node->mChildren[i], lambda);
	}
}

inline cbs::Model::Model()
{
	m_scene = nullptr;
	m_nodeCount = 0;
}
inline cbs::Model::Model(D3D11Device * device, const char * strName)
{
	_create(device, strName, DEFAULT_BASIC_LAYOUT, DEFAULT_SKINNED_LAYOUT);
}
inline cbs::Model::Model(D3D11Device * device, const char * strName, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl)
{
	_create(device, strName, basic_vl, skinned_vl);
}
#pragma warning(push)
#pragma warning(disable:4996)
CBS_DEPRECATED inline cbs::Model::Model(const char * strName)
{
	_create(g_d3d11dev, strName, DEFAULT_BASIC_LAYOUT, DEFAULT_SKINNED_LAYOUT);
}
CBS_DEPRECATED inline cbs::Model::Model(const char * strName, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl)
{
	_create(g_d3d11dev, strName, basic_vl, skinned_vl);
}
#pragma warning(pop)

inline cbs::Model::Model(Model&& _move)
{
	m_meshes = std::move(_move.m_meshes);
	m_materials = std::move(_move.m_materials);
	m_animations = std::move(_move.m_animations);
	m_vb = std::move(_move.m_vb);
	m_ib = std::move(_move.m_ib);
	m_scene = _move.m_scene;
	m_nodeCount = _move.m_nodeCount;
	_move.m_scene = nullptr;
	_move.m_nodeCount = 0;
}
inline cbs::Model & cbs::Model::operator =(Model && _move)
{
	this->~Model();
	new(this) cbs::Model(std::move(_move));
	return *this;
}
inline void cbs::Model::setAnimationTPS(double tps)
{
	size_t cnt =  getAnimationCount();
	for (size_t i = 0; i < cnt; i++)
	{
		m_animations[i].setTPS(tps);
	}
}
inline cbs::Model::Animation * cbs::Model::getAnimation(size_t animation) const
{
	assert(animation < getAnimationCount());
	Animation * anim = &m_animations[animation];
	if (anim->m_animation == nullptr) return nullptr;
	return anim;
}
inline const aiScene * cbs::Model::getAIScene() const
{
	return m_scene;
}

inline cbs::Model::operator bool()
{
	return m_scene != nullptr;
}
inline bool cbs::Model::operator !()
{
	return m_scene == nullptr;
}

#pragma warning(push)
#pragma warning(disable:4996)
inline cbs::ModelRenderer::ModelRenderer()
{
	m_mrContext = g_d3d11dev->m_context;
}
#pragma warning(pop)

inline cbs::ModelRenderer::ModelRenderer(D3D11Device * device)
{
	m_mrContext = device->m_context;
}

#pragma warning(push)
#pragma warning(disable:4996)
inline void cbs::ModelRenderer::setSkinnedWorlds(const Matrix4x3 * world, size_t m4x3count)
{
	setBoneWorlds(world, m4x3count);
}
inline void cbs::ModelRenderer::setBoneWorlds(const Matrix4x3 * world, size_t m4x3count)
{
	assert(!"Need implement");
}
#pragma warning(pop)
