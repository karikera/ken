#include "stdafx.h"
#include "geom.h"

using namespace kr;

//Line2
line2::line2() noexcept
{
}
line2::line2(const vec2 &from, const vec2 &to) noexcept
	:from(from), to(to)
{
}
float line2::length() noexcept
{
	return (from-to).length();
}
float dot(const line2 &a, const line2 &b) noexcept
{
	return dot(a.to-a.from, b.to-b.from);
}
float cross(const line2 & a, const line2 &b) noexcept
{
	return cross(a.to-a.from, b.to-b.from);
}
const vec2 line2::operator &(const line2 &o) const noexcept
{
	float x1= to.x-from.x;
	float y1= to.y-from.y;
	float x2= o.to.x-o.from.x;
	float y2= o.to.y-o.from.y;

	return vec2(
		(x1*(y2*o.from.x -x2*o.from.y) -x2*(y1*from.x -x1*from.y))/(y2*x1-y1*x2),
		(y1*(x2*o.from.y -y2*o.from.x) -y2*(x1*from.y -y1*from.x))/(x2*y1-x1*y2)
		);
}
bool line2::operator /(const line2 &o) const noexcept
{
	return cross(*this, o)==0;
}

//Line3
line3::line3() noexcept
{
}
line3::line3(const movable3 &pos) noexcept
{
	from=pos.pos;
	to=pos.pos+pos.speed;
}
line3::line3(const vec3 &from, const vec3 &to) noexcept
	:from(from), to(to)
{
}
float line3::length() noexcept
{
	return (from-to).length();
}
float dot(const line3 &t, const line3 &o) noexcept
{
	return dot((t.to-t.from), (o.to-o.from));
}
const vec3 cross(const line3 &t, const line3 &o) noexcept
{
	return cross((t.to-t.from), (o.to-o.from));
}

kr::triangle3::triangle3() noexcept
{
}
kr::triangle3::triangle3(const vec3 & p1, const vec3 & p2, const vec3 & p3) noexcept
{
	v1 = p1;
	v2 = p2;
	v3 = p3;
}
bool kr::triangle3::ray(const movable3 & p) const noexcept
{
	vec3 n1 = v1 - p.pos;
	vec3 n2 = v2 - p.pos;
	if (dot(cross(n1, n2), p.speed) > 0) return false;
	vec3 n3 = v3 - p.pos;
	if (dot(cross(n2, n3), p.speed) > 0) return false;
	if (dot(cross(n3, n1), p.speed) > 0) return false;
	return true;
}
const vec3 kr::triangle3::normal() const noexcept
{
	return cross(v2 - v1, v3 - v1).normalize();
}
bool triangle3::test(const movable3 &pos) const noexcept
{
	if(!ray(pos)) return false;
	return plane3(*this).test((line3)pos);
}
bool triangle3::slip(vec3 &out, const movable3 &p) const noexcept
{
	if(ray(p))
	{
		out=p.pos + p.speed;
		return plane3(*this).slip(out, p);
	}
	return false;
}
bool triangle3::slip(movable3 &p) const noexcept
{
	if(ray(p))
	{
		return plane3(*this).slip(p);
	}
	return false;
}
bool triangle3::stick(movable3 &p) const noexcept
{
	if(!ray(p)) return false;
	return plane3(*this).stick(p);
}
bool triangle3::reflect(movable3 &p, float fric, float elas) const noexcept
{
	if(ray(p))
	{
		return plane3(*this).reflect(p, fric, elas);
	}
	return false;
}

kr::movable2::movable2() noexcept
{
}
kr::movable2::movable2(const vec2 & p, const vec2 & s)  noexcept
	:pos(p), speed(s)
{
}
float kr::movable2::length() noexcept
{
	return speed.length();
}
float dot(const movable2 & a, const movable2 & b) noexcept
{
	return dot(a.speed, b.speed);
}
float cross(const movable2 & a, const movable2 & b) noexcept
{
	return cross(a.speed, b.speed);
}

kr::movable3::movable3() noexcept
{
}
kr::movable3::movable3(const line3 & ln) noexcept
{
	pos = ln.from;
	speed = ln.to - ln.from;
}
kr::movable3::movable3(const vec3 & p, const vec3 & s) noexcept
	:pos(p), speed(s)
{
}
float kr::movable3::length() const noexcept
{
	return speed.length();
}
const vec3 kr::movable3::stick_x(float fIntercept) const noexcept
{
	vec3 c;
	fIntercept = (fIntercept - pos.x) / speed.x;
	c.x = fIntercept;
	c.y = fIntercept*speed.y + pos.y;
	c.z = fIntercept*speed.z + pos.z;
	return c;
}
const vec3 kr::movable3::stick_y(float fIntercept) const noexcept
{
	vec3 c;
	fIntercept = (fIntercept - pos.y) / speed.y;
	c.x = fIntercept*speed.x + pos.x;
	c.y = fIntercept;
	c.z = fIntercept*speed.z + pos.z;
	return c;
}
const vec3 kr::movable3::stick_z(float fIntercept) const noexcept
{
	vec3 c;
	fIntercept = (fIntercept - pos.z) / speed.z;
	c.x = fIntercept*speed.x + pos.x;
	c.y = fIntercept*speed.y + pos.y;
	c.z = fIntercept;
	return c;
}

float dot(const movable3 & t, const movable3 & o) noexcept
{
	return dot(t.speed, o.speed);
}
const vec3 cross(const movable3 & t, const movable3 & o) noexcept
{
	return cross(t.speed, o.speed);
}
float distance(const movable3 & pos, vec3 p) noexcept
{
	p -= pos.pos;
	float fLength = cross(p, pos.speed).length() / pos.speed.length();  // cross calculate
																		//float fLength= (p - pos.speed * (dot(p,pos.speed) / pos.speed.length())).length(); // dot calculate

	return fLength;
}

kr::plane3::plane3(const vec3 & p, const vec3 & n) noexcept
{
	pos = p;
	norm = n.normalize();
}
kr::plane3::plane3(const vec3 & p1, const vec3 & p2, const vec3 & p3) noexcept
{
	pos = p1;
	norm = cross((p2 - p1), (p3 - p1)).normalize();
}
kr::plane3::plane3(const triangle3 & tri) noexcept
{
	pos = tri.v1;
	norm = cross((tri.v2 - tri.v1), (tri.v3 - tri.v1)).normalize();
}
bool kr::plane3::_slip(vec3 & left, const movable3 & p) noexcept
{
	vec3 subp = p.pos - pos;
	float fFrom = dot(norm, subp);
	if (fFrom < 0) return false;
	float fPass = dot(norm, subp + p.speed);
	if (fPass > 0) return false;

	float fLen = dot(norm, p.speed);
	left = p.speed * (-fPass) / fLen;
	return true;
}
bool kr::plane3::_stick(vec3 & left, const movable3 & p) noexcept
{
	vec3 subp = p.pos - pos;
	float fFrom = dot(norm, subp);
	if (fFrom < 0) return false;
	float fPass = dot(norm, subp + p.speed);
	if (fPass > 0) return false;

	float fLen = dot(norm, p.speed);
	left = p.speed * (fFrom) / fLen;
	return true;
}
bool kr::plane3::stick(vec3 & p) noexcept
{
	float fDot = dot((p - pos), norm);
	if (fDot > 0) return false;
	p -= norm * fDot;
	return true;
}
bool kr::plane3::stick(movable3 & p) noexcept
{
	vec3 left;
	if (!_stick(left, p)) return false;
	p.speed = left;
	return true;
}
bool kr::plane3::stick(vec3 & speed, movable3 & p) noexcept
{
	vec3 left;
	if (!_stick(left, p)) return false;
	speed = left;
	return true;
}
bool kr::plane3::slip(movable3 & p) noexcept
{
	vec3 left;
	if (!_slip(left, p)) return false;
	p.speed = left;
	return true;
}
bool kr::plane3::slip(vec3 & speed, const movable3 & p) noexcept
{
	vec3 left;
	if (!_slip(left, p)) return false;
	speed = left;
	return true;
}
bool kr::plane3::distance_sq(float & lensq, const movable3 & p) noexcept
{
	vec3 npos;
	if (!_slip(npos, p)) return false;
	lensq = npos.length_sq();
	return true;
}
bool kr::plane3::distance(float & len, const movable3 & p) noexcept
{
	vec3 npos;
	if (!_slip(npos, p)) return false;
	len = npos.length();
	return true;
}
bool kr::plane3::reflect(movable3 & npos, float fric, float elas) noexcept
{
	vec3 spe;
	if (!stick(spe, npos)) return false;
	// npos.speed = (npos.speed- dn)*fric - dn * elas
	// npos.speed = npos.speed*fric- dn*fric - dn * elas
	// npos.speed = npos.speed*fric- dn*(fric + elas);
	npos.speed = npos.speed*fric - (dot(norm, npos.speed)*(fric + elas))*norm;
	npos.pos += spe;
	return true;
}
bool kr::plane3::test(const line3 & ln) noexcept
{
	if (dot(norm, ln.from - pos) < 0) return false;
	if (dot(norm, ln.to - pos) > 0) return false;
	return true;
}
bool kr::plane3::test(const movable3 & move) noexcept
{
	vec3 bpos = move.pos - pos;
	if (dot(norm, bpos) < 0) return false;
	if (dot(norm, bpos + move.speed) > 0) return false;
	return true;
}
