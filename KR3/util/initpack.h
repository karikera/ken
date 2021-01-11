#pragma once

#include <KR3/main.h>

namespace kr
{
	class InitPack
	{
	public:
		using dtor_t = void(*)(void*);
		using ctor_t = dtor_t(*)(void*);

		template <typename T>
		using tdtor_t = void(*)(T*);
		template <typename T>
		using tctor_t = tdtor_t<T>(*)(T*);

		template <typename T>
		void add(tctor_t<T> ctor, T* param) noexcept;

		void init() noexcept;
		void clear() noexcept;

		InitPack() noexcept;
		~InitPack() noexcept;

	private:
		void _add(ctor_t ctor, void* param) noexcept;

		Array<pair<ctor_t, void*>> m_array;
		ondebug(bool m_inited);
	};
	template <typename T>
	void InitPack::add(tctor_t<T> ctor, T* param) noexcept
	{
		return _add((ctor_t)ctor, param);
	}

	template <typename T>
	class Deferred :public kr::Manual<T>
	{
	public:
		Deferred(InitPack& init) noexcept
		{
			init.add<Deferred<T>>([](Deferred<T>* _this) {
				_this->create();
				return (InitPack::tdtor_t<Deferred<T>>)[](Deferred<T>* _this) {
					_this->remove();
				};
			}, this);
		}
		~Deferred() noexcept
		{
		}
	};
}
