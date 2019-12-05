#include "stdafx.h"
#include "object.h"

#include "class.h"
#include "class.inl"

using namespace kr;


JsObject::JsObject(const JsArguments& args) throws(JsObject)
	:JsObjectT(args)
{
}
JsObject::~JsObject() noexcept
{
}
void JsObject::initMethods(JsClassT<JsObject>* cls) noexcept
{
}

JsValue JsObjectT<JsObject>::newInstanceRaw(JsArgumentsIn args) throws(JsObjectT)
{
	return classObject.newInstanceRaw(args); // args
}

JsClass& JsObjectT<JsObject>::getClass() noexcept
{
	return *s_classInfo.get();
}
void JsObjectT<JsObject>::finallize() noexcept
{
	delete this;
}

void _pri_::JsClassInfo::operator delete(void* p) noexcept
{
}
size_t _pri_::JsClassInfo::getIndex() noexcept
{
	return m_index;
}
JsClass* _pri_::JsClassInfo::get() noexcept
{
	return (JsClass*)m_classObjectBuffer;
}


JsGetter::~JsGetter() noexcept
{
}
JsAccessor::~JsAccessor() noexcept
{
}
