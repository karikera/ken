#include "stdafx.h"
#include "context.h"
#include "value.h"

using namespace kr;

JsValue JsRuntime::run(Text16 source) throws(JsException)
{
	return run(u"[untitled]", source);
}


JsContext::Scope::Scope(JsContext& ctx) noexcept
	:m_context(&ctx)
{
	ctx.enter();
}
JsContext::Scope::~Scope() noexcept
{
	m_context->exit();
}