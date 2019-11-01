#pragma once

#include "buffer.h"

namespace kr
{
	template <typename Buffer2D>
	class Join : public Bufferable<Join<Buffer2D>, BufferInfo<typename Buffer2D::Component::Component>>
	{
	public:
		using InnerArray = typename Buffer2D::Component;
		using Component = typename InnerArray::Component;
		using Text = View<Component>;

		Join(const Buffer2D &arr, Text glue) noexcept
			:m_array(arr), m_glue(glue)
		{
			if (m_array.size() == 0)
			{
				m_size = 0;
				return;
			}
			const InnerArray * p = m_array.begin();
			const InnerArray * end = m_array.end();

			size_t gluesz = glue.size();
			size_t sz = p->size();
			p++;
			while (p != end)
			{
				sz += gluesz;
				sz += p->size();
				p++;
			}

			m_size = sz;
		}
		size_t $size() const noexcept
		{
			return m_size;
		}
		size_t $copyTo(Component * dest) const noexcept
		{
			if (m_size == 0) return 0;

			const InnerArray * p = m_array.begin();
			const InnerArray * end = m_array.end();

			dest += p->copyTo(dest);
			p++;
			while (p != end)
			{
				dest += m_glue.copyTo(dest);
				dest += p->copyTo(dest);
				p++;
			}

			return m_size;
		}

	private:
		const Buffer2D & m_array;
		Text m_glue;
		size_t m_size;
	};

	template <typename Buffer3D>
	class Join2D : public Bufferable<Join2D<Buffer3D>, BufferInfo<typename Buffer3D::Component::Component::Component>>
	{
	public:
		using Buffer2D = typename Buffer3D::Component;
		using InnerArray = typename Buffer2D::Component;
		using Component = typename InnerArray::Component;
		using Text = View<Component>;

		Join2D(const Buffer3D &arr, Text glueout, Text gluein) noexcept
			:m_array(arr), m_glueout(glueout), m_gluein(gluein)
		{
			if (m_array.size() == 0)
			{
				m_size = 0;
				return;
			}
			const Buffer2D * p = m_array.begin();
			const Buffer2D * end = m_array.end();

			size_t gluesz = glueout.size();
			size_t sz = Join<Buffer2D>(*p, m_gluein).size();
			p++;
			while (p != end)
			{
				sz += gluesz;
				sz += Join<Buffer2D>(*p, m_gluein).size();
				p++;
			}

			m_size = sz;
		}
		size_t $size() const noexcept
		{
			return m_size;
		}
		size_t $copyTo(typename Text::Component * dest) const noexcept
		{
			if (m_size == 0) return 0;

			const Buffer2D * p = m_array.begin();
			const Buffer2D * end = m_array.end();

			dest += Join<Buffer2D>(*p, m_gluein).copyTo(dest);
			p++;
			while (p != end)
			{
				dest += m_glueout.copyTo(dest);
				dest += Join<Buffer2D>(*p, m_gluein).copyTo(dest);
				p++;
			}

			return m_size;
		}

	private:
		const Buffer3D & m_array;
		Text m_glueout;
		Text m_gluein;
		size_t m_size;
	};

	template <typename Buffer2D>
	Join<Buffer2D> join(const Buffer2D &arr, typename Join<Buffer2D>::Text glue) noexcept
	{
		return Join<Buffer2D>(arr, glue);
	}
	template <typename Buffer3D>
	Join2D<Buffer3D> join2D(const Buffer3D &arr, typename Join2D<Buffer3D>::Text glueout, typename Join2D<Buffer3D>::Text gluein) noexcept
	{
		return Join2D<Buffer3D>(arr, glueout, gluein);
	}
}