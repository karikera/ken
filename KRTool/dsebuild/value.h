#pragma once

#include "identity.h"

class Value: public Identity
{
public:
	Value(Class * type, AText name);
	virtual ~Value();
	Keep<Identity> find(Text str) override; // ErrMessage
	virtual Class* getType() noexcept;
	virtual Value* getMaxValue();
	Keep<Value> getSizeOf(Function * szof) override;
	void setArray(Value * size)  noexcept;
	Value* getArraySize() noexcept;
	bool isDynamicSize() noexcept;

	Keep<Class> const type;

private:
	Keep<Value> m_size;
};
class LValue
{
public:
	virtual Field * getField() = 0;
	virtual void setPrepare() = 0;
	virtual bool prepared() =0;
};
class UnknownValue:public Value
{
public:
	UnknownValue(Class * type);
	~UnknownValue() override;
	TText toString(Class * scope) override;
};
class Constant:public Value
{
public:
	Constant(Class * type, int value);
	TText toString(Class * scope) override;
	Value* getMaxValue() override;
	const int value;
};
