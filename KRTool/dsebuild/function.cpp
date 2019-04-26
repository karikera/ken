#include "function.h"
#include "class.h"
#include "root.h"
#include <KR3/main.h>
#include <sstream>
#include <new>

Parameters::Parameters(size_t size) noexcept
	: m_size(size)
{
}
Parameters * Parameters::newInstance(initializer_list<Identity*> params) noexcept
{
	Parameters* list = (Parameters*)_new char[sizeof(Parameters) + sizeof(Keep<Identity>) * params.size()];
	new(list) Parameters(params.size());

	Keep<Identity>* dest = list->begin();
	Keep<Identity>* end = list->end();
	Identity* const* src = params.begin();
	while(dest != end)
	{
		new(dest++) Keep<Identity>(*src++);
	}
	return list;
}
Parameters::~Parameters()
{
	Keep<Identity>* iter = begin();
	Keep<Identity>* e = end();
	while (iter != e)
	{
		(*iter++).~Keep();
	}
}
size_t Parameters::hash() const
{
	return kr::mem::hash(begin(), sizeof(Keep<Identity*>)*m_size);
}
Keep<Identity>* Parameters::begin() const
{
	return (Keep<Identity>*)(this + 1);
}
Keep<Identity>* Parameters::end() const
{
	return begin() +m_size;
}
bool Parameters::operator ==(const Parameters & params) const
{
	size_t size = m_size;
	if(size != params.m_size) return false;
	
	Keep<Identity>* src = params.begin();
	Keep<Identity>* dest = begin();
	Keep<Identity>* end = this->end();
	while(src != end)
	{
		if(*dest++ != *src++) return false;
	}
	return true;
}
size_t ParametersPtr::hash() const
{
	return m_list->hash();
}
bool ParametersPtr::operator ==(const ParametersPtr & params) const
{
	return **this == *params;
}

std::size_t hash<ParametersPtr>::operator()(const Parameters * k) const
{
	return k->hash();
}

Function::Function(Class * type, string name, size_t paramcount):Identity(move(name)), returnType(type), parameter_count(paramcount)
{
}
Function::~Function()
{
}

Keep<Value> Function::getSizeOf(Function * szof)
{
	throw ErrMessage("Cannot get size of function");
}
Keep<Identity> Function::find(Text id)
{
	throw ErrMessage("Function has not member");
}
Keep<FunctionCall> Function::call(initializer_list<Identity*> params)
{
	if(parameter_count != -1)
	{
		if(params.size() != parameter_count) throw ErrMessage("Unmatch parameter count. need %d", parameter_count);
	}
	auto res = m_calls.insert({ Parameters::newInstance(params), nullptr});
	if(!res.second)
	{
		return res.first->second;
	}
	FunctionCall * call = _new FunctionCall(this, Parameters::newInstance(params));
	res.first->second = call;
	return call;
}
void outParameters(TText& ss, Parameters* params, Class * scope)
{
	auto iter = params->begin();
	auto end = params->end();
	if(iter == end) return;

	ss << (*iter++)->toString(scope);
	while(iter != end)
	{
		ss << ',' << (*iter++)->toString(scope);
	}
}
TText Function::toCallString(Parameters * params, Class * scope)
{
	TText ss;
	ss << name << '(';
	outParameters(ss, params, scope);
	ss << ')';
	return ss;
}

Method::Method(Class * type, string name, size_t count):Function(type, name, count+1)
{
}
Method::~Method()
{
}
TText Method::toCallString(Parameters * params, Class * scope)
{
	auto iter = params->begin();
	auto end = params->end();
	if(iter == end) throw ErrMessage("Need this parameter.");
	TText text = (*iter++)->toString(scope);
	text << "::" << name << '(';
	if(iter != end)
	{
		text << (*iter++)->toString(scope);
		while(iter != end)
		{
			text << ',' << (*iter++)->toString(scope);
		}
	}
	text << ')';
	return text;
}

TemplateValue::TemplateValue(Class * type, string name, size_t count):Function(type, name, count)
{
}
TemplateValue::~TemplateValue()
{
}
TText TemplateValue::toCallString(Parameters * params, Class * scope)
{
	TText ss;
	ss << name << '<';
	outParameters(ss, params, scope);
	ss << ">::value";
	return ss;
}

OutSizeOf::OutSizeOf():TemplateValue(&g_root.clsInt, "outsizeof", 1)
{
}
OutSizeOf::~OutSizeOf()
{
}
TText OutSizeOf::toCallString(Parameters * params, Class * scope)
{
	auto iter = params->begin();
	Identity * _this = (*iter);

	if(dynamic_cast<Class*>(_this) && !dynamic_cast<ClassExtern*>(_this))
	{
		return AText::concat(_this->toString(scope), "::template out_sizeof<OS>::value");
	}
	else
	{
		return AText::concat("OS::template out_sizeof<", _this->toString(scope), ">::value");
	}
}

FunctionCall::FunctionCall(Function * func, Parameters * params)
	:Expression(func->returnType, AText::concat('@', func->name, "()"))
	, function(func), params(params)
{
}
TText FunctionCall::toString(Class * scope)
{
	return function->toCallString(params, scope);
}