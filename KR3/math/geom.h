#pragma once

#include <KR3/main.h>
#include "coord.h"

namespace kr
{
	class movable3;

	class line2
	{
	public:
		vec2 from, to;

		line2() noexcept;
		line2(const vec2 &from, const vec2 &to) noexcept;
		float length() noexcept;
		const vec2 operator &(const line2 &o) const noexcept;
		bool operator /(const line2 &o) const noexcept;
	};

	class line3
	{
	public:
		vec3 from, to;

		line3() noexcept;
		line3(const vec3 &from, const vec3 &to) noexcept;
		explicit line3(const movable3 &pos) noexcept;
		float length() noexcept;
	};

	class movable2
	{
	public:
		vec2 pos, speed;

		movable2() noexcept;
		movable2(const vec2 &p, const vec2 &s) noexcept;
		float length() noexcept;
	};

	class movable3
	{
	public:
		vec3 pos, speed;

		movable3() noexcept;
		explicit movable3(const line3 &ln) noexcept;
		movable3(const vec3 &p, const vec3 &s) noexcept;
		float length() const noexcept;
		const vec3 stick_x(float fIntercept) const noexcept;
		const vec3 stick_y(float fIntercept) const noexcept;
		const vec3 stick_z(float fIntercept) const noexcept;
		friend float distance(const movable3& pos, vec3 p) noexcept;
	};

	class triangle3
	{
	public:
		vec3 v1, v2, v3;

		triangle3() noexcept;
		triangle3(const vec3 &p1, const vec3 &p2, const vec3 &p3) noexcept;
		bool ray(const movable3 &p) const noexcept;
		const vec3 normal() const noexcept;
		bool test(const movable3 &pos) const noexcept;
		bool slip(vec3 &out, const movable3 &p) const noexcept;
		bool slip(movable3 &p) const noexcept;
		bool stick(movable3 &p) const noexcept;
		bool reflect(movable3 &p, float fric, float elas) const noexcept;
	};

	class plane3
	{
	public:
		vec3 pos;
		vec3 norm;

		plane3(const vec3 &p, const vec3 &n) noexcept;
		plane3(const vec3 &p1, const vec3 &p2, const vec3 &p3) noexcept;
		explicit plane3(const triangle3 &tri) noexcept;
		bool _slip(vec3 &left, const movable3 &p) noexcept;
		bool _stick(vec3 &left, const movable3 &p) noexcept;
		bool stick(vec3 &p) noexcept;
		bool stick(movable3 &p) noexcept;
		bool stick(vec3& speed, movable3 &p) noexcept;
		bool slip(movable3 &p) noexcept;
		bool slip(vec3& speed, const movable3 &p) noexcept;
		bool distance_sq(float &lensq, const movable3 &p) noexcept;
		bool distance(float &len, const movable3 &p) noexcept;
		bool reflect(movable3 &npos, float fric, float elas) noexcept;
		bool test(const line3 &ln) noexcept;
		bool test(const movable3 &move) noexcept;
	};

}

float dot(const kr::line2 & a, const kr::line2 &b) noexcept;
float cross(const kr::line2 &a, const kr::line2 &b) noexcept;
float dot(const kr::line3 &t, const kr::line3 &o) noexcept;
const kr::vec3 cross(const kr::line3 &t, const kr::line3 &o) noexcept;
float dot(const kr::movable2 &a, const kr::movable2 &b) noexcept;
float cross(const kr::movable2 &a, const kr::movable2 &b) noexcept;
float dot(const kr::movable3 &t, const kr::movable3 &o) noexcept;
const kr::vec3 cross(const kr::movable3 &t, const kr::movable3 &o) noexcept;
