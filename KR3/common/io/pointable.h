#pragma once

#include "../container.h"

namespace kr
{

	template <typename T>
	class PointableContainer
	{
	private:
		T m_data;

	protected:
		T* _ptr() noexcept
		{
			return &m_data;
		}
		const T* _ptr() const noexcept
		{
			return &m_data;
		}

	public:
		using Component = typename T::Component;

		PointableContainer(T&& data) noexcept
			:m_data(move(data))
		{
		}
		PointableContainer(const T& data) noexcept
			:m_data(data)
		{
		}
		const T* operator ->() const noexcept
		{
			return &m_data;
		}
		T* operator ->() noexcept
		{
			return &m_data;
		}
	};
	template <typename T>
	class PointableContainer<Keep<T> >
	{
	private:
		Keep<T> m_data;

	protected:
		T* _ptr() const noexcept
		{
			return m_data;
		}

	public:
		using Component = typename T::Component;

		PointableContainer(Keep<T>&& data) noexcept
			:m_data(move(data))
		{
		}
		PointableContainer(const Keep<T>& data) noexcept
			:m_data(data)
		{
		}
		T* operator ->() const noexcept
		{
			return m_data;
		}
	};
	template <typename T>
	class PointableContainer<T*>
	{
	private:
		T* m_data;

	protected:
		T* _ptr() const noexcept
		{
			return m_data;
		}

	public:
		using Component = typename T::Component;

		PointableContainer(T* data) noexcept
			:m_data(data)
		{
		}
		T* operator ->() const noexcept
		{
			return m_data;
		}
	};

}