#include "stdafx.h"
#include "context.h"
#include "value.h"

using namespace kr;

JsValue JsRuntime::run(Text16 source) throws(JsException)
{
	return run(u"[untitled]", source);
}
