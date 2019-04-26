#pragma once

#include "common.h"

namespace cbs
{
	struct BoundingSphere
	{
		union
		{
			XMVECTOR v;
			struct {
				float x,y,z;
				float radius;
			};
		};
	};
	struct AABB
	{
		XMVECTOR from;
		XMVECTOR to;
	};

	class Collider
	{
	public:
		inline Collider();
		inline virtual ~Collider();

		// 충돌이 일어났을 경우 호출된다.
		virtual void onCollide(Collider * other) =0;

		// 바운딩 스피어를 가져온다.
		// 순수 가상함수
		virtual BoundingSphere getBoundingSphere() = 0;

		// 축으로 정렬된 바운딩 박스를 가져온다.
		// 오버라이딩하여 구현하지 않을 경우, 바운딩 스피어에서 계산해낸다.
		inline virtual AABB getAABB();

		// 충돌체크를 할 모델을 가져온다.
		// 별도의 충돌 모델을 사용하지 않을 경우 NULL을 반환
		inline virtual const aiScene * getBoundingModel();

		bool m_replaced;
		bool m_detached;
		bool m_deleted;
	};
	class SphereCollider
	{
	public:
		inline virtual void onCollide(Collider * other);
		inline virtual BoundingSphere getBoundingSphere();

	private:
		BoundingSphere m_sphere;
	};
	class ColliderSpace
	{
	public:
		// 콜라이더를 충돌 공간에 넣는다
		virtual void attach(Collider * collider) =0;

		// 전체 충돌 체크를 실행한다
		virtual void test() = 0;

	private:
	};
	class WorstCollideSpace:public ColliderSpace
	{
	public:
		inline virtual void attach(Collider * collider) override
		inline virtual void test() override

	private:
		inline void _clear();

		std::vector<Collider*> m_list;
		struct UnalignedBS
		{
			float x,y,z,radius;
		};
		std::vector<UnalignedBS> m_boudingspheres; 
	};
	//class ModelCollider :public Collider
	//{
	//public:
	//	inline ModelCollider(const char * strName);
	//	inline ModelCollider(aiScene * scene);
	//	inline ModelCollider(Model * model);
	//	inline virtual bool isCollided(Collider * other); // override
	//	virtual void onCollide(Collider * other); // override

	//private:
	//	inline void _create(const aiScene * scene);
	//};

}
