#pragma once

#include <KR3/main.h>

namespace kr
{
	// 함수를 분해해서, JsValue를 파라미터로 받는 함수로 랩핑 시키는 부분
	
	template <typename FUNC>
	struct JsMetaLambda;
	template <typename CLASS, typename RET, typename ... ARGS>
	struct JsMetaLambda<RET(CLASS::*)(ARGS ...)>
	{
		template <typename T>
		struct numunwrap;
		template <size_t ... idx>
		struct numunwrap<meta::numlist<idx ...> >
		{
			template <typename FUNC>
			static JsValue call(FUNC func, const JsArguments & args);
		};
		using ccfunc = JsFunctionT<RET(ARGS ...)>;
		using counter = meta::make_numlist_counter<sizeof ... (ARGS)>;
		using Call = numunwrap<counter>;
	};
	template <typename CLASS, typename ... ARGS>
	struct JsMetaLambda<void(CLASS::*)(ARGS ...)>
	{
		template <typename T>
		struct numunwrap;
		template <size_t ... idx>
		struct numunwrap<meta::numlist<idx ...> >
		{
			template <typename FUNC>
			static JsValue call(FUNC func, const JsArguments & args);
		};
		using ccfunc = JsFunctionT<void(ARGS ...)>;
		using counter = meta::make_numlist_counter<(sizeof... (ARGS))>;
		using Call = numunwrap<counter>;
	};
	template <typename CLASS, typename RET, typename ... ARGS>
	struct JsMetaLambda<RET(CLASS::*)(ARGS ...) const> :JsMetaLambda<RET(CLASS::*)(ARGS ...)>
	{
	};

	template <typename FUNC>
	struct JsMeta : JsMetaLambda<decltype(&FUNC::operator())>
	{
	};
	template <typename RET, typename ... ARGS>
	struct JsMeta<RET(*)(ARGS ...)>
	{
		template <typename T>
		struct numunwrap;
		template <size_t ... idx>
		struct numunwrap<meta::numlist<idx ...> >
		{
			static JsValue call(RET(*fn)(ARGS ...), const JsArguments & args);
		};
		using ccfunc = JsFunctionT<RET(ARGS ...)>;
		using counter = meta::make_numlist_counter<sizeof ... (ARGS)>;
		using Call = numunwrap<counter>;
	};
	template <typename ... ARGS>
	struct JsMeta<void(*)(ARGS ...)>
	{
		template <typename T>
		struct numunwrap;
		template <size_t ... idx>
		struct numunwrap<meta::numlist<idx ...> >
		{
			static JsValue call(void(*fn)(ARGS ...), const JsArguments & args);
		};
		using ccfunc = JsFunctionT<void(ARGS ...)>;
		using counter = meta::make_numlist_counter<sizeof ... (ARGS)>;
		using Call = numunwrap<counter>;
	};
	template <typename CLASS, typename RET, typename ... ARGS>
	struct JsMeta<RET(CLASS::*)(ARGS ...)>
	{
		template <typename T>
		struct numunwrap;
		template <size_t ... idx>
		struct numunwrap<meta::numlist<idx ...> >
		{
			static JsValue call(RET(CLASS::*fn)(ARGS ...), const JsArguments & args);
		};
		using ccfunc = JsFunctionT<RET(ARGS ...)>;
		using counter = meta::make_numlist_counter<sizeof ... (ARGS)>;
		using Call = numunwrap<counter>;
	};
	template <typename CLASS, typename ... ARGS>
	struct JsMeta<void(CLASS::*)(ARGS ...)>
	{
		template <typename T>
		struct numunwrap;
		template <size_t ... idx>
		struct numunwrap<meta::numlist<idx ...> >
		{
			static JsValue call(void(CLASS::*fn)(ARGS ...), const JsArguments & args);
		};
		using ccfunc = JsFunctionT<void(ARGS ...)>;
		using counter = meta::make_numlist_counter<sizeof ... (ARGS)>;
		using Call = numunwrap<counter>;
	};
	template <typename CLASS, typename RET, typename ... ARGS>
	struct JsMeta<RET(CLASS::*)(ARGS ...) const> :JsMeta<RET(CLASS::*)(ARGS ...)>
	{
	};

}