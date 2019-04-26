#include "expression.h"
#include "root.h"
#include "field.h"
#include <sstream>
#include <KR3/main.h>

bool isZero(Value * v)
{
	Constant * c = dynamic_cast<Constant*>(v);
	if(c == nullptr) return false;
	return c->value == 0;
}

Expression::Expression(Class * type, AText name):Value(type, move(name))
{
}
Keep<Value> Expression::getSizeOf(Function * szof)
{
	return &g_root.vZero;
}

FieldReference::FieldReference(Value * parent, Field * c)
	:Expression(c->type, AText::concat('@', parent->toString(nullptr), '.', c->toString(nullptr)))
	,parent(parent), child(c)
{
}
TText FieldReference::toString(Class * scope)
{
	TText out = parent->toString(scope);
	out << '.' << child->name;
	return out;
}
Class * FieldReference::getType() noexcept
{
	return child->getType();
}
Field * FieldReference::getField() noexcept
{
	return child;
}
void FieldReference::setPrepare() // ErrMessage
{
	child->setPrepare();
	LValue* lvalue = dynamic_cast<LValue*>((Value*)parent);
	if (lvalue != nullptr) lvalue->setPrepare();
}
bool FieldReference::prepared()
{
	return child->prepared();
}

namespace
{
	// a+b   +a
	// a*5+b   +a
	// a*5+b   -a
	// create concat test function

	TText toOperString(int oper) noexcept
	{
		if(oper < 0x7f)
			return TText({ (char)oper });
		else
			return TText({ (char)(oper), (char)(oper >> 8) });
	}
	bool calculateSumTest(Operation * oper, Value * val) // add test function
	{
		if(oper == nullptr) return false;
		if(oper->opercode != '*') return false;
		if(oper->operands.size() != 2) return false;

		if(oper->operands.front() == val)
		{
			auto & back = oper->operands.back();
			back = Operation::calculate(back, '+', &g_root.vOne);
			return true;
		}
		else if(oper->operands.back() == val)
		{
			auto & front = oper->operands.back();
			front = Operation::calculate(front, '+', &g_root.vOne);
			return true;
		}
		return false;
	}

}


Operation::Operation(Value * a, int oper, Value * b)
	:Expression(a->type, TText::concat('@', a->toString(nullptr), toOperString(oper), b->toString(nullptr)))
	, opercode(oper)
{
	operands.push_back(a);
	operands.push_back(b);
}
TText Operation::toString(Class * scope)
{
	auto iter = operands.begin();
	auto end = operands.end();
	if(iter == end) throw ErrMessage("Empty operation");
	TText out = (*iter++)->toString(scope);
	while(iter != end)
	{
		out << toOperString(opercode);
		out << (*iter++)->toString(scope);
	}
	return out;
}

Keep<Value> Operation::calculate(Keep<Value> a, int oper, Keep<Value> b)
{
	switch(oper)
	{
	case '&': case '|': case '^': 
	case '&&': case '||': case '==': case '!=':
	case '-': case '+': case '=':
	case '*': case '/':
		break;
	default: throw ErrMessage("Unknown operation %s", toOperString(oper).c_str());
	}

	if(isZero(a))
	{
		switch(oper)
		{
		case '&&':
		case '&':
		case '*':
		case '/':
			return &g_root.vZero;
		case '^':
		case '|':
		case '-':
		case '+':
		case '||':
			return b;
		}
	}
	if(isZero(b))
	{
		switch(oper)
		{
		case '&&':
		case '&':
		case '*':
			return &g_root.vZero;
		case '^':
		case '|':
		case '-':
		case '+':
		case '||':
			return a;
		case '/':
			throw ErrMessage("Zero devision");
		}
	}
	if(a == b)
	{
		switch(oper)
		{
		case '||':
		case '|':
		case '&&':
		case '&':
			return a;
		case '==':
			return &g_root.vOne;
		case '!=':
		case '-':
		case '^':
			return &g_root.vZero;
		case '*':
			break;
		case '+':
			if(a == b) return _new Operation(a, '*', _new Constant(&g_root.clsInt, 2));
			break;
		}
	}

	Operation * operA = dynamic_cast<Operation*>((Value*)a);
	Operation * operB = dynamic_cast<Operation*>((Value*)b);
	if(oper == '+')
	{
		if(calculateSumTest(operA, b)) return operA;
		if(calculateSumTest(operB, a)) return operB;
	}

	if(operA != nullptr &&  operA->opercode == oper)
	{
		if(operB != nullptr && operB->opercode == oper)
		{
			for(Value * v : operB->operands)
			{
				operA->operands.push_back(v);
			}
			return operA;
		}
		else
		{
			// add test
			operA->operands.push_back((Value*)b);
			return operA;
		}
	}
	else if(operB != nullptr && operB->opercode == oper)
	{
		// add test
		operB->operands.push_front((Value*)a);
		return operB;
	}

	Constant * constA = dynamic_cast<Constant*>((Value*)a);
	Constant * constB = dynamic_cast<Constant*>((Value*)b);
	if(constA && constB)
	{
		int av = constA->value;
		int bv = constB->value;
		switch(oper)
		{
		case '*': return _new Constant(&g_root.clsInt, av * bv);
		case '/': return _new Constant(&g_root.clsInt, av / bv);
		case '+': return _new Constant(&g_root.clsInt, av + bv);
		case '-': return _new Constant(&g_root.clsInt, av - bv);
		case '&': return _new Constant(&g_root.clsInt, av & bv);
		case '^': return _new Constant(&g_root.clsInt, av ^ bv);
		case '|': return _new Constant(&g_root.clsInt, av | bv);
		case '&&': return _new Constant(&g_root.clsInt, av && bv);
		case '||': return _new Constant(&g_root.clsInt, av || bv);
		case '==': return _new Constant(&g_root.clsInt, av == bv);
		case '!=': return _new Constant(&g_root.clsInt, av != bv);
		}
	}

	return _new Operation(a, oper, b);
}

StringExpression::StringExpression(Class * type, AText exp):
	Expression(type, move(AText() << '@' << exp)), 
	expression(move(exp))
{
}
StringExpression::~StringExpression()
{
}
TText StringExpression::toString(Class * scope)
{
	return expression;
}

