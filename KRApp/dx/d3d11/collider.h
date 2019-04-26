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

		// �浹�� �Ͼ�� ��� ȣ��ȴ�.
		virtual void onCollide(Collider * other) =0;

		// �ٿ�� ���Ǿ �����´�.
		// ���� �����Լ�
		virtual BoundingSphere getBoundingSphere() = 0;

		// ������ ���ĵ� �ٿ�� �ڽ��� �����´�.
		// �������̵��Ͽ� �������� ���� ���, �ٿ�� ���Ǿ�� ����س���.
		inline virtual AABB getAABB();

		// �浹üũ�� �� ���� �����´�.
		// ������ �浹 ���� ������� ���� ��� NULL�� ��ȯ
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
		// �ݶ��̴��� �浹 ������ �ִ´�
		virtual void attach(Collider * collider) =0;

		// ��ü �浹 üũ�� �����Ѵ�
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
