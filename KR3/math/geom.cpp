#include "stdafx.h"
#include "geom.h"

using namespace kr;

line2_ft::line2_ft(const vec2 &from, const vec2 &to) noexcept
	:from(from), to(to)
{
}
float line2_ft::length() noexcept
{
	return (from-to).length();
}
float dot(const line2_ft &a, const line2_ft &b) noexcept
{
	return dot(a.to-a.from, b.to-b.from);
}
float cross(const line2_ft & a, const line2_ft &b) noexcept
{
	return cross(a.to-a.from, b.to-b.from);
}
const vec2 line2_ft::intersect(const line2_ft &o) const noexcept
{
	//vec2 pd = p2 - p1;
	//float crossv = cross(b.p2 - b.p1, pd);
	//if (fabsf(crossv) < 0.001f) return false;
	//*out = pd * (cross(p1 - b.p1, pd) / crossv) + p1;
	//return true;

	float x1= to.x-from.x;
	float y1= to.y-from.y;
	float x2= o.to.x-o.from.x;
	float y2= o.to.y-o.from.y;

	return vec2(
		(x1*(y2*o.from.x -x2*o.from.y) -x2*(y1*from.x -x1*from.y))/(y2*x1-y1*x2),
		(y1*(x2*o.from.y -y2*o.from.x) -y2*(x1*from.y -y1*from.x))/(x2*y1-x1*y2)
		);
}
bool line2_ft::isParallel(const line2_ft &o) const noexcept
{
	return cross(*this, o)==0;
}

line3_ft::line3_ft(const line3 &pos) noexcept
{
	from=pos.pos;
	to=pos.pos+pos.dir;
}
line3_ft::line3_ft(const vec3 &from, const vec3 &to) noexcept
	:from(from), to(to)
{
}
float line3_ft::length() noexcept
{
	return (from-to).length();
}
float dot(const line3_ft &t, const line3_ft &o) noexcept
{
	return dot((t.to-t.from), (o.to-o.from));
}
const vec3 cross(const line3_ft &t, const line3_ft &o) noexcept
{
	return cross((t.to-t.from), (o.to-o.from));
}

triangle3::triangle3(const vec3 & p1, const vec3 & p2, const vec3 & p3) noexcept
{
	v1 = p1;
	v2 = p2;
	v3 = p3;
}
bool triangle3::ray(const line3 & p) const noexcept
{
	vec3 n1 = v1 - p.pos;
	vec3 n2 = v2 - p.pos;
	if (dot(cross(n1, n2), p.dir) > 0) return false;
	vec3 n3 = v3 - p.pos;
	if (dot(cross(n2, n3), p.dir) > 0) return false;
	if (dot(cross(n3, n1), p.dir) > 0) return false;
	return true;
}
const vec3 triangle3::normal() const noexcept
{
	return cross(v2 - v1, v3 - v1).normalize();
}

line2::line2(const vec2 & p, const vec2 & s)  noexcept
	:pos(p), dir(s)
{
}
const vec2 line2::to() const noexcept
{
	return pos + dir;
}
float line2::length() const noexcept
{
	return dir.length();
}
float dot(const line2 & a, const line2 & b) noexcept
{
	return dot(a.dir, b.dir);
}
float cross(const line2 & a, const line2 & b) noexcept
{
	return cross(a.dir, b.dir);
}

line3::line3(const line3_ft & ln) noexcept
{
	pos = ln.from;
	dir = ln.to - ln.from;
}
line3::line3(const vec3 & p, const vec3 & s) noexcept
	:pos(p), dir(s)
{
}
const vec3 line3::to() const noexcept
{
	return pos + dir;
}
float line3::length() const noexcept
{
	return dir.length();
}
const vec3 line3::stick_x(float fIntercept) const noexcept
{
	vec3 c;
	fIntercept = (fIntercept - pos.x) / dir.x;
	c.x = fIntercept;
	c.y = fIntercept*dir.y + pos.y;
	c.z = fIntercept*dir.z + pos.z;
	return c;
}
const vec3 line3::stick_y(float fIntercept) const noexcept
{
	vec3 c;
	fIntercept = (fIntercept - pos.y) / dir.y;
	c.x = fIntercept*dir.x + pos.x;
	c.y = fIntercept;
	c.z = fIntercept*dir.z + pos.z;
	return c;
}
const vec3 line3::stick_z(float fIntercept) const noexcept
{
	vec3 c;
	fIntercept = (fIntercept - pos.z) / dir.z;
	c.x = fIntercept*dir.x + pos.x;
	c.y = fIntercept*dir.y + pos.y;
	c.z = fIntercept;
	return c;
}

float dot(const line3 & t, const line3 & o) noexcept
{
	return dot(t.dir, o.dir);
}
const vec3 cross(const line3 & t, const line3 & o) noexcept
{
	return cross(t.dir, o.dir);
}
float distance(const line3 & pos, vec3 p) noexcept
{
	p -= pos.pos;
	float fLength = cross(p, pos.dir).length() / pos.dir.length();  // cross calculate
																		//float fLength= (p - pos.dir * (dot(p,pos.dir) / pos.dir.length())).length(); // dot calculate

	return fLength;
}

plane3::plane3(const vec3 & p, const vec3 & n) noexcept
{
	a = n.x;
	b = n.y;
	c = n.z;

	// a*p.x + b*p.y + c*p.z + d = 0
	// d = -(a*p.x + b*p.y + c*p.z)
	d = -(a * p.x + b * p.y + c * p.z);
}
plane3::plane3(const vec3 & p1, const vec3 & p2, const vec3 & p3) noexcept
	:plane3(p1, cross((p2 - p1), (p3 - p1)))
{
}
plane3::plane3(const triangle3 & tri) noexcept
	:plane3(tri.v1, tri.v2, tri.v3)
{
}
const vec3 plane3::intersect(const line3& p) const noexcept
{
	vec3 norm(a, b, c);
	float dir = dot(p.dir, norm);
	float distance = -(dot(p.pos, norm) + d) / dir;
	return distance * p.dir + p.pos;
}
bool plane3::slip(vec3 * nspeed, const line3 & p) const noexcept
{
	vec3 npos = p.pos + p.dir;
	if (!stick(npos)) return false;
	*nspeed = npos-p.pos;
	return true;
}
bool plane3::stick(vec3 * nspeed, const line3 & p) const noexcept
{
	vec3 norm(a, b, c);
	float dir = dot(p.dir, norm);
	if (dir >= 0) return false;
	float distance = -(dot(p.pos, norm) + d) / dir;
	if (distance > 1) return false;
	*nspeed = distance* p.dir;
	return true;
}
bool plane3::stick(vec3 & p) const noexcept
{
	vec3 norm(a, b, c);
	float pos_dot = dot(p, norm) + d;
	if (pos_dot > 0) return false;
	p += norm * (pos_dot / -norm.length_sq());
	return true;
}
bool plane3::stick(line3 & p) const noexcept
{
	vec3 nspeed;
	if (!stick(&nspeed, p)) return false;
	p.dir = nspeed;
	return true;
}
bool plane3::slip(line3 & p) const noexcept
{
	vec3 nspeed;
	if (!slip(&nspeed, p)) return false;
	p.dir = nspeed;
	return true;
}
float plane3::distance(const vec3& p) const noexcept
{
	// a*x +b*y +c = 0
	// a*(a/b*y) +b*y +c = 0
	// a*a/b*y +b*y +c = 0
	// (a*a/b +b)*y +c = 0
	// y = -c/(a*a/b +b)
	// y = -c*b/(a*a +b*b)
	// y = b * (-d/(a*a +b*b +c*c))

	vec3 norm(a, b, c);
	return (dot(p, norm) + d) / norm.length();
}
bool plane3::distance_rate(float * len_rate, const line3 & p) const noexcept
{
	vec3 norm(a, b, c);
	float dir = dot(p.dir, norm);
	if (dir >= 0) return false;
	float distance = (dot(p.pos, norm) + d) / dir;
	*len_rate = distance;
	return true;
}
bool plane3::distance(float * len, const line3 & p) const noexcept
{
	if (!distance_rate(len, p)) return false;
	*len = *len * p.dir.length();
	return true;
}
bool plane3::reflect(line3 & npos, float fric, float elas) const noexcept
{
	vec3 spe;
	if (!stick(&spe, npos)) return false;
	// npos.dir = (npos.dir- dn)*fric - dn * elas
	// npos.dir = npos.dir*fric- dn*fric - dn * elas
	// npos.dir = npos.dir*fric- dn*(fric + elas);
	vec3 norm(a, b, c);
	npos.dir = npos.dir*fric - (dot(norm, npos.dir)*(fric + elas)/norm.length_sq())*norm;
	npos.pos += spe;
	return true;
}
bool plane3::contains(const vec3& pt) const noexcept
{
	return a * pt.x + b * pt.y + c * pt.z + d <= 0;
}
bool plane3::test(const line3_ft & ln) const noexcept
{
	return contains(ln.from) || contains(ln.to);
}
bool plane3::test(const line3 & move) const noexcept
{
	return contains(move.pos) || contains(move.pos + move.dir);
}
plane3 plane3::fromViewLine(const line3& line) noexcept
{
	return plane3(line.pos, cross(line.pos, line.dir));
}
