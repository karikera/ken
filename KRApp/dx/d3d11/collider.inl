#pragma once

#include "collider.h"
#include <algorithm>

inline cbs::Collider::Collider()
{
	m_deleted = false;
}
inline cbs::Collider::~Collider()
{
}
inline cbs::AABB cbs::Collider::getAABB()
{
	AABB cube;
	__m128 axis = getBoundingSphere().v;
	__m128 radius = _mm_shuffle_ps(axis, axis, _MM_SHUFFLE(3, 3, 3, 3));
	cube.from = _mm_sub_ps(axis, radius);
	cube.to = _mm_add_ps(axis, radius);
	return cube;
}
inline const aiScene * cbs::Collider::getBoundingModel()
{
	return nullptr;
}

inline void cbs::SphereCollider::onCollide(Collider * other) //override
{
}
inline cbs::BoundingSphere cbs::SphereCollider::getBoundingSphere() //override
{
	return m_sphere;
}

inline void cbs::WorstCollideSpace::attach(Collider * collider) //override
{
	collider->m_detached = false;
	collider->m_replaced = false;

	m_list.push_back(collider);
	m_boudingspheres.push_back((UnalignedBS&)collider->getAABB());
}
inline void cbs::WorstCollideSpace::test() //override
{
	_clear();
	auto beg = m_list.begin();
	auto end = m_list.end();
	for(auto iter = beg;iter != end;iter++)
	{
		Collider * c = *iter;
		UnalignedBS &bs1 = m_boudingspheres[iter - beg];
		vec s1(&bs1.x);
		for (auto iter2 = iter; iter2 != end; iter2++)
		{
			UnalignedBS &bs2 = m_boudingspheres[iter2 - beg];
			vec s2 = _mm_loadu_ps(&bs2.x);
			float dist;
			XMStoreFloat(&dist, XMVector3Length(_mm_sub_ps(s1, s2)));
			if (dist < bs1.radius + bs2.radius)
			{
				c->onCollide((*iter2));
			}
		}
	}
	_clear();
}
inline void cbs::WorstCollideSpace::_clear()
{
	auto beg = m_list.begin();
	auto end = m_list.end();
	auto bbeg = m_boudingspheres.begin();
	auto bend = m_boudingspheres.end();
	auto testDetach = [](Collider * c){ return c->m_detached; };
	beg = std::find_if(beg, end, testDetach);
	if (beg == end) return;
	
	auto i = beg;
	auto bi = bbeg;
	for (;;)
	{
		i++;
		bi++;
		if(i == end) break;
		Collider * c = *i;
		if (c->m_detached)
		{
			if (c->m_deleted) delete c;
			continue;
		}
		*beg++ = std::move(*i);
		*bbeg++ = std::move(*bi);
	}
	m_list.erase(beg);
	m_boudingspheres.erase(bbeg);
}
//inline cbs::ModelCollider::ModelCollider(const char * strName)
//{
//	const aiScene * scene = aiImportFile(strName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs | aiProcess_SortByPType | aiProcess_LimitBoneWeights);
//	if (scene == nullptr)
//	{
//		wchar_t temp[1024];
//		swprintf_s(temp, L"%hs 모델을 찾을 수 없습니다.\r\n", strName);
//		MessageBoxW(nullptr, temp, nullptr, MB_OK | MB_ICONERROR);
//		return;
//	}
//	_create(scene);
//	aiReleaseImport(scene);
//}
//inline cbs::ModelCollider::ModelCollider(aiScene * scene)
//{
//	_create(scene);
//}
//inline cbs::ModelCollider::ModelCollider(Model * model)
//{
//	_create(model->getAIScene());
//}
//inline void cbs::ModelCollider::_create(const aiScene * strName)
//{
//}
//inline bool cbs::isCollided(Collider * other)
//{
//}