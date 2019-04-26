#pragma once

namespace emscripten
{
	template <typename T> struct class_
	{
		inline class_(const char * name) {}

		template <typename ... ARGS>
		inline class_ & constructor() { return *this; }

		template <typename RET, typename ... ARGS>
		inline class_ & function(const char * name, RET(T::*func)(ARGS ...)) { return *this; }

		template <typename RET, typename ... ARGS>
		inline class_ & class_function(const char * name, RET (*func)(ARGS ...)) { return *this; }

		template <typename TYPE>
		inline class_ & property(const char * name, TYPE (T::*get)(), void(T::*set)(TYPE)) { return *this; }
	};
}


#ifdef __EMSCRIPTEN__
#define EMSCRIPTEN_BINDINGS(name)	void name()
#else
#define EMSCRIPTEN_BINDINGS(name) static_assert(false, "Cannot call EM_ASM!"); void name(_empri_::EmbindingClass class_)
#endif
