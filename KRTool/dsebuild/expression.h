#pragma once

#include "value.h"

class Expression:public Value
{
public:
	Expression(Class * type, AText name);

	Keep<Value> getSizeOf(Function * szof) override;
};
class FieldReference:public Expression, public LValue
{
public:
	FieldReference(Value * parent, Field * child);
	TText toString(Class * scope) override;
	Class * getType() noexcept override;
	Field * getField() noexcept override;
	void setPrepare() override;
	bool prepared() override;

	const Keep<Value> parent;
	const Keep<Field> child;
};
class Operation:public Expression
{
private:
	Operation(Value* a, int oper, Value* b);

public:
	TText toString(Class * scope) override;
	static Keep<Value> calculate(Keep<Value> a, int oper, Keep<Value> b);

	list<Keep<Value>> operands;
	const int opercode;
};
class StringExpression:public Expression
{
public:
	StringExpression(Class * type, AText exp);
	~StringExpression() override;
	TText toString(Class * scope) override;

	const string expression;
};
