#include "stdafx.h"
#include "collision.h"

using namespace kr;

inline bool stickX(vec3 &stick, const movable3 &move, float intersect) noexcept
{
	if(move.pos.x < intersect) return false;
	if(move.pos.x + move.speed.x > intersect) return false;

	stick=move.stick_x(0);

	return math::between(0.f, stick.y, 1.f) && math::between(0.f, stick.z, 1.f);
}
inline bool stickY(vec3 &stick, const movable3 &move, float intersect) noexcept
{
	if(move.pos.y < 0) return false;
	if(move.pos.y + move.speed.y > intersect) return false;

	stick=move.stick_y(0);
	return math::between(0.f, stick.z, 1.f) && math::between(0.f, stick.x, 1.f);
}
inline bool stickZ(vec3 &stick, const movable3 &move, float intersect) noexcept
{
	if(move.pos.z < 0) return false;
	if(move.pos.z + move.speed.z > intersect) return false;

	stick=move.stick_z(0);
	return math::between(0.f, stick.x, 1.f) && math::between(0.f, stick.y, 1.f);
}
inline bool stickXi(vec3 &stick, const movable3 &move, float intersect) noexcept
{
	if(move.pos.x > intersect) return false;
	if(move.pos.x + move.speed.x < intersect) return false;

	stick=move.stick_x(0);
	return math::between(0.f, stick.y, 1.f) && math::between(0.f, stick.z, 1.f);
}
inline bool stickYi(vec3 &stick, const movable3 &move, float intersect) noexcept
{
	if(move.pos.y > 0) return false;
	if(move.pos.y + move.speed.y < intersect) return false;

	stick=move.stick_y(0);
	return math::between(0.f, stick.z, 1.f) && math::between(0.f, stick.x, 1.f);
}
inline bool stickZi(vec3 &stick, const movable3 &move, float intersect) noexcept
{
	if(move.pos.z > 0) return false;
	if(move.pos.z + move.speed.z < intersect) return false;

	stick=move.stick_z(0);
	return math::between(0.f, stick.x, 1.f) && math::between(0.f, stick.y, 1.f);
}

void CollisionObject::setWorld(const mat4a &mat) noexcept
{
	m_mWorld=mat;
	m_mInverse = mat.inverse();
}
bool CollisionCube::setSlip(movable3 &move) noexcept
{
	vec3 stick;
	if(stickX(stick, move, 1))
	{
		move.pos.x=0;
		return true;
	}
	if(stickY(stick, move, 1))
	{
		move.pos.y=0;
		return true;
	}
	if(stickZ(stick, move, 1))
	{
		move.pos.z=0;
		return true;
	}
	if(stickXi(stick, move, 0))
	{
		move.pos.x=0;
		return true;
	}
	if(stickYi(stick, move, 0))
	{
		move.pos.y=0;
		return true;
	}
	if(stickZi(stick, move, 0))
	{
		move.pos.z=0;
		return true;
	}
	return false;
}
bool CollisionCube::setStick(movable3 &move) noexcept
{
	vec3 stick;
	if(stickX(stick, move, 1)) goto __true;
	if(stickY(stick, move, 1)) goto __true;
	if(stickZ(stick, move, 1)) goto __true;
	if(stickXi(stick, move, 0)) goto __true;
	if(stickYi(stick, move, 0)) goto __true;
	if(stickZi(stick, move, 0)) goto __true;
	return false;

__true:
	move.pos=stick;
	return true;
}
bool CollisionCube::test(const movable3 &move) noexcept
{
	vec3 stick;
	if(stickX(stick, move, 1)) return true;
	if(stickY(stick, move, 1)) return true;
	if(stickZ(stick, move, 1)) return true;
	if(stickXi(stick, move, 0)) return true;
	if(stickYi(stick, move, 0)) return true;
	if(stickZi(stick, move, 0)) return true;
	return false;
}
