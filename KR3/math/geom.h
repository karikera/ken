#pragma once

#include <KR3/main.h>
#include "coord.h"

namespace kr
{
	class line3;

	class line2_ft
	{
	public:
		vec2 from, to;

		line2_ft() = default;
		line2_ft(const vec2 &from, const vec2 &to) noexcept;
		float length() noexcept;
		const vec2 intersect(const line2_ft &o) const noexcept;
		bool isParallel(const line2_ft &o) const noexcept;
	};

	class line3_ft
	{
	public:
		vec3 from, to;

		line3_ft()=default;
		line3_ft(const vec3 &from, const vec3 &to) noexcept;
		explicit line3_ft(const line3 &pos) noexcept;
		float length() noexcept;
	};

	class line2
	{
	public:
		vec2 pos, dir;

		line2()=default;
		line2(const vec2 &p, const vec2 &s) noexcept;
		const vec2 to() const noexcept;
		float length() const noexcept;
	};

	class line3
	{
	public:
		vec3 pos, dir;

		line3()=default;
		explicit line3(const line3_ft &ln) noexcept;
		line3(const vec3 &p, const vec3 &s) noexcept;
		const vec3 to() const noexcept;
		float length() const noexcept;
		const vec3 stick_x(float fIntercept) const noexcept;
		const vec3 stick_y(float fIntercept) const noexcept;
		const vec3 stick_z(float fIntercept) const noexcept;
		friend float distance(const line3& pos, vec3 p) noexcept;
	};

	class triangle3
	{
	public:
		vec3 v1, v2, v3;

		triangle3() = default;
		triangle3(const vec3 &p1, const vec3 &p2, const vec3 &p3) noexcept;
		bool ray(const line3 &p) const noexcept;
		const vec3 normal() const noexcept;
	};

	class plane3
	{
	public:
		// a*x + b*y + c*z + d = 0
		float a, b, c, d;

		plane3(const vec3 &p, const vec3 &n) noexcept;
		plane3(const vec3 &p1, const vec3 &p2, const vec3 &p3) noexcept;
		explicit plane3(const triangle3 &tri) noexcept;
		const vec3 intersect(const line3 & p) const noexcept;
		bool slip(vec3 * nspeed, const line3 &p) const noexcept;
		bool stick(vec3 * nspeed, const line3 &p) const noexcept;
		bool stick(vec3 &p) const noexcept;
		bool stick(line3 &p) const noexcept;
		bool slip(line3 &p) const noexcept;
		float distance(const vec3 & p) const noexcept;
		bool distance_rate(float * lensq, const line3 &p) const noexcept;
		bool distance(float * len, const line3 &p) const noexcept;
		bool reflect(line3 &npos, float fric, float elas) const noexcept;
		bool contains(const vec3& pt) const noexcept;
		bool test(const line3_ft &ln) const noexcept;
		bool test(const line3 &move) const noexcept;

		static plane3 fromViewLine(const line3& line) noexcept;
	};

}

float dot(const kr::line2_ft & a, const kr::line2_ft &b) noexcept;
float cross(const kr::line2_ft &a, const kr::line2_ft &b) noexcept;
float dot(const kr::line3_ft &t, const kr::line3_ft &o) noexcept;
const kr::vec3 cross(const kr::line3_ft &t, const kr::line3_ft &o) noexcept;
float dot(const kr::line2 &a, const kr::line2 &b) noexcept;
float cross(const kr::line2 &a, const kr::line2 &b) noexcept;
float dot(const kr::line3 &t, const kr::line3 &o) noexcept;
const kr::vec3 cross(const kr::line3 &t, const kr::line3 &o) noexcept;
