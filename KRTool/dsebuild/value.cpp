#include "value.h"
#include "field.h"
#include "root.h"
#include "expression.h"

Value::Value(Class * c, AText name)
	:Identity(name), type(c), m_size(nullptr)
{
}
Value::~Value()
{
}
Class* Value::getType() noexcept
{
	return type;
}
Keep<Identity> Value::find(Text str) // ErrMessage
{
	throw ErrMessage("Cannot found %s", TSZ(str).c_str());
}
Value* Value::getMaxValue()
{
	if(dynamic_cast<ConstField*>((Value*)m_size) || dynamic_cast<Constant*>((Value*)m_size))
	{
		return m_size;
	}
	else
	{
		return g_root.fnMaximumOf.call({ (Class*)m_size->type });
	}
}
Keep<Value> Value::getSizeOf(Function * szof)
{
	Keep<Value> size = type->getSizeOf(szof);
	if(m_size != nullptr)
	{
		return Operation::calculate(getMaxValue(), '*', size);
	}
	return size;
}
void Value::setArray(Value * size) noexcept
{
	m_size = size;
}
Value* Value::getArraySize() noexcept
{
	return m_size;
}
bool Value::isDynamicSize() noexcept
{
	if(m_size != nullptr) return true;
	return type->isDynamicSize();
}

UnknownValue::UnknownValue(Class * c):Value(c, "?")
{
}
UnknownValue::~UnknownValue()
{
}
TText UnknownValue::toString(Class * scope)
{
	return TText::concat("(", type->toString(scope), ")?");
}

Constant::Constant(Class *c, int v):Value(c, decf(v)), value(v)
{
}
TText Constant::toString(Class * scope)
{
	TText text;
	text << value;
	return text;
}
Value* Constant::getMaxValue()
{
	return this;
}
