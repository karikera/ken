#pragma once

#include <KR3/main.h>
#include "expression.h"

class Parameters : public kr::Referencable<Parameters>
{
public:
	static Parameters * newInstance(initializer_list<Identity*> params) noexcept;
	Parameters(const Parameters & copy) = delete;
	~Parameters();
	size_t hash() const;
	Keep<Identity>* begin() const;
	Keep<Identity>* end() const;
	Parameters & operator =(const Parameters &) noexcept = delete;
	bool operator ==(const Parameters & params) const;

private:
	Parameters(size_t size) noexcept;

	const size_t m_size;
};

class ParametersPtr : public Keep<Parameters>
{
public:
	using Keep::Keep;
	using Keep::operator =;
	size_t hash() const;
	bool operator ==(const ParametersPtr & params) const;

private:
	Keep<Parameters> m_list;
};

template<> struct hash<ParametersPtr>
{
	std::size_t operator()(const Parameters * k) const;
};

class Function:public Identity
{
public:
	Function(Class * type, string name, size_t paramcount);
	~Function() override;

	Keep<Value> getSizeOf(Function * szof) override;
	Keep<Identity> find(Text id) override;
	Keep<FunctionCall> call(initializer_list<Identity*> params);
	virtual TText toCallString(Parameters * params, Class * scope);
	Keep<Class> returnType;

private:
	unordered_map<ParametersPtr, Keep<FunctionCall>> m_calls;
	const size_t parameter_count;
};

class Method:public Function
{
public:
	Method(Class * type, string name, size_t count);
	~Method() override;
	TText toCallString(Parameters * params, Class * scope) override;
};

class TemplateValue:public Function
{
public:
	TemplateValue(Class * type, string name, size_t count);
	~TemplateValue() override;
	TText toCallString(Parameters * params, Class * scope) override;
};

class OutSizeOf:public TemplateValue
{
public:
	OutSizeOf();
	~OutSizeOf() override;
	TText toCallString(Parameters * params, Class * scope) override;
};

class FunctionCall:public Expression
{
	friend Function;
private:
	FunctionCall(Function * func, Parameters * params);

public:
	TText toString(Class * scope) override;

	Function* const function;
	const Keep<Parameters> params;
};