#pragma once

namespace kr
{
	class JsLambda
	{
	public:
		class Data
		{
		public:
			virtual void call() = 0;
			virtual void remove() = 0;
		private:

		};

	private:

		template <typename LAMBDA>
		class DataImpl : public Data
		{
		private:
			LAMBDA m_lambda;

		public:
			DataImpl(LAMBDA lambda)
				:m_lambda(move(lambda))
			{
			}
			virtual void call() override
			{
				m_lambda();
			}
			virtual void remove() override
			{
				delete this;
			}
		};
		Data* m_data;

	public:
		JsLambda() noexcept
		{
			m_data = nullptr;
		}
		template <typename LAMBDA>
		JsLambda(LAMBDA lambda) noexcept
		{
			m_data = _new DataImpl<LAMBDA>(move(lambda));
		}
		JsLambda(JsLambda&& c) noexcept
		{
			m_data = c.m_data;
			c.m_data = nullptr;
		}
		~JsLambda() noexcept
		{
			if (m_data != nullptr) m_data->remove();
		}
		Data* detach() noexcept
		{
			Data* data = m_data;
			m_data = nullptr;
			return data;
		}
		void operator ()() const noexcept
		{
			m_data->call();
		}
		JsLambda& operator =(JsLambda&& c) noexcept
		{
			this->~JsLambda();
			m_data = c.m_data;
			c.m_data = nullptr;
			return *this;
		}
	};
}