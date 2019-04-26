#pragma once

#include "../main.h"
#include "coord.h"
#include <cstdint>

namespace kr
{
	struct BoundingSphere
	{
		vec3 center;
		float radius;
	};

	class VertexList
	{
	public:
		class Iterator
		{
			friend VertexList;
		public:
			Iterator() noexcept;
			bool operator == (const Iterator & iter) const noexcept;
			bool operator != (const Iterator & iter) const noexcept;
			Iterator & operator --() noexcept;
			const Iterator operator --(int) noexcept;
			Iterator & operator ++() noexcept;
			const Iterator operator ++(int) noexcept;
			const Iterator operator +(std::intptr_t n) const noexcept;
			const Iterator operator -(std::intptr_t n) const noexcept;
			friend const Iterator operator +(std::intptr_t n, const Iterator & i) noexcept;
			const vec3 * operator ->() const noexcept;
			const vec3& operator *() const noexcept;
			const vec3& operator [](std::uintptr_t idx) const noexcept;

		private:
			size_t m_stride;
			const byte * m_ptr;
		};
		VertexList(const void * start, size_t stride, size_t count) noexcept;
		~VertexList() noexcept;

		size_t size() const noexcept;
		Iterator begin() const noexcept;
		Iterator end() const noexcept;
		const vec3& operator [](size_t idx) const noexcept;
		BoundingSphere getBoundingSphere() const noexcept;
		const void * getPointer() const noexcept;
		size_t getStride() const noexcept;

	private:
		const byte * m_start;
		size_t m_stride;
		size_t m_count;
	};
}
