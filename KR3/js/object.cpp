#include "stdafx.h"
#include "object.h"

#include "class.h"

using namespace kr;

JsObject::JsObject(const JsArguments& args) throws(JsObject)
	:JsObjectT(args)
{
}
JsObject::~JsObject() noexcept
{
}
Text16 JsObject::getClassName() noexcept
{
	return u"NativeObject";
}
void JsObject::initMethods(JsClassT<JsObject>* cls) noexcept
{
}

JsObjectT<JsObject>::JsObjectT(const JsArguments& args) throws(JsObjectT)
{
}
JsObjectT<JsObject>::~JsObjectT() noexcept
{
}

JsClass& JsObjectT<JsObject>::getClass() noexcept
{
	return *s_classInfo.get();
}
void JsObjectT<JsObject>::finallize() noexcept
{
	delete this;
}

size_t _pri_::JsClassInfo::getIndex() noexcept
{
	return m_index;
}
JsClass* _pri_::JsClassInfo::get() noexcept
{
	return (JsClass*)m_classObjectBuffer;
}
_pri_::JsClassInfo* _pri_::JsClassInfo::next() noexcept
{
	return m_next;
}
