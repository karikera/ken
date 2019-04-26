#pragma once

#include <KR3/main.h>
#include <KR3/math/geom.h>
#include <KR3/math/coord.h>

namespace kr
{
	class Collision
	{
	public:
		virtual bool setSlip(movable3 &move) noexcept = 0;
		virtual bool setStick(movable3 &move) noexcept = 0;
		virtual bool test(const movable3 &move) noexcept = 0;
	};
	class CollisionObject
	{
	public:
		void setWorld(const mat4a &mat) noexcept;

	protected:
		mat4a m_mInverse, m_mWorld;
	};
	class CollisionCube: public Collision
	{
	public:
		bool setSlip(movable3 &move) noexcept override;
		bool setStick(movable3 &move) noexcept override;
		bool test(const movable3 &move) noexcept override;
	};
	class CollisionMap
	{
	protected:
		Array<Collision*> m_list;
		dword m_width, m_height;

	public:
		CollisionMap() noexcept;
		bool test(movable3 &move) noexcept;
	};

}