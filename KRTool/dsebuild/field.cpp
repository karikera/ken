#include "field.h"
#include "fstream.h"
#include "root.h"
#include "expression.h"

#include <sstream>

Field::Field(Class * parent, Class * c, AText str) noexcept
	:Value(c, str), parent(parent)
{
	m_prepare = false;
}
Field::~Field() noexcept
{
}
Keep<Identity> Field::find(Text str)
{
	return _new FieldReference(this, cast<Field>(type->find(str)));
}
TText Field::toString(Class * scope)
{
	if(scope == nullptr) return name;
	TText out;
	if (scope->parentReference(parent, &out))
		out << name;
	else
		out << parent->toString(scope) << "::" << name;
	return out;
}

Field * Field::getField() noexcept
{
	return this;
}
void Field::setPrepare() noexcept
{
	m_prepare = true;
}
bool Field::prepared() noexcept
{
	return m_prepare;
}

ConstField::ConstField(Class * parent, Class * type, AText name) noexcept :Field(parent, type, name)
{
}
ConstField::~ConstField() noexcept
{
}
Value * ConstField::getMaxValue() noexcept
{
	return this;
}

ValueConstField::ValueConstField(Class * parent, AText name, Constant * v) noexcept
	:ConstField(parent, v->getType(), move(name)), value(v)
{
}
ValueConstField::~ValueConstField() noexcept
{
}

Switch::Switch(Class * parent, AText name, Value * v)
	:Field(parent, v->getType(), name)
	, value(v), m_buffer(nullptr)
{
}
Switch::~Switch()
{
}
TText Switch::toString(Class * scope)
{
	return TText::concat("switch(", value->toString(scope), ")");
}
Keep<Value> Switch::getSizeOf(Function * szof)
{
	std::vector<Identity*> values;
	values.reserve(10);
	for(auto & pair : switches)
	{
		values.push_back(szof->call({ pair.second }));
	}

	Identity ** first = &*values.begin();
	Identity ** end = first + values.size();

	return g_root.fnMaxSelect.call(initializer_list<Identity*>(first, end));
}

If::If(Class * parent, AText name, Value * value, Class * contain)
	:Field(parent,value->getType(),name)
	, condition(value)
	,contain(contain)
{
}
If::~If()
{
}