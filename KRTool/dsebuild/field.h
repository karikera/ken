#pragma once

#include "value.h"
#include <initializer_list>
#include <unordered_map>

class Field:public Value, public LValue
{
public:
	Field(Class * parent, Class * type, AText name) noexcept;
	~Field() noexcept override;
	Keep<Identity> find(Text str) override;
	TText toString(Class * scope) override;
	Field * getField() noexcept override;
	void setPrepare() noexcept override;
	bool prepared() noexcept override;

	Class * const parent;

private:
	bool m_prepare;
};


class ConstField:public Field
{
public:
	ConstField(Class * parent, Class * type, AText name) noexcept;
	~ConstField() noexcept override;
	Value * getMaxValue() noexcept override;
};

class ValueConstField:public ConstField
{
public:
	ValueConstField(Class * parent, AText name, Constant * value) noexcept;
	~ValueConstField() noexcept override;

	const Keep<Constant> value;
};

class Switch:public Field
{
public:
	Switch(Class * parent, AText name, Value * value);
	~Switch() override;
	TText toString(Class * scope) override;
	Keep<Value> getSizeOf(Function * szof) override;
	const Keep<Value> value;
	unordered_map<int, Keep<Class>> switches;

private:
	Keep<ConstField> m_buffer;
};

class If:public Field
{
public:
	If(Class * parent, AText name, Value * value, Class * contain);
	~If() override;

	const Keep<Value> condition;
	const Keep<Class> contain;
};