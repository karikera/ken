#include "class.h"
#include "fstream.h"
#include "field.h"
#include "expression.h"
#include "root.h"

#include <KR3/util/stdext.h>

using namespace std::krext;

Class::Class(AText nm, Class * p, bool n):Identity(move(nm)), parent(p), numeric(n)
{
}
Class::~Class()
{
}
ClassExtern::ClassExtern(AText name, Class * p):Class(move(name), p, false)
{
}
ClassExtern::~ClassExtern()
{
}
Keep<Identity> ClassExtern::find(Text id)
{
	auto res = scope.insert(id, nullptr);
	if(!res.second) return res.first->second;

	ConstField * inst = _new ConstField(this, _new ClassExtern("?", this), id);
	res.first->second = inst;
	return inst;
}
Keep<Value> ClassExtern::getSizeOf(Function * szof)
{
	return szof->call({ this });
}

template <typename INSTANCE, typename UNKNOWNVALUE, typename CONSTANT, typename SWITCH, typename IF, typename SUBSTITUTION>
void switch_type(Identity *v, INSTANCE instance, UNKNOWNVALUE unknownValue,
	CONSTANT constant, SWITCH _switch, IF _if, SUBSTITUTION substitution)
{
	union
	{
		Value * lv;
		Field * inst;
		Constant * cv;
		Operation * ss;
		UnknownValue * uv;
	};

	if(lv = dynamic_cast<Value*>(v))
	{
		if(inst = dynamic_cast<Field*>(v))
		{
			union
			{
				Switch * sw;
				If * iv;
			};
			if(sw = dynamic_cast<Switch*>(v))
			{
				_switch(sw);
			}
			else if(iv = dynamic_cast<If*>(v))
			{
				_if(iv);
			}
			else
			{
				instance(inst);
			}
		}
		else if(uv = dynamic_cast<UnknownValue*>(v))
		{
			unknownValue(uv);
		}
		else if(cv=dynamic_cast<Constant*>(v))
		{
			constant(cv);
		}
		else if(dynamic_cast<Expression*>(v))
		{
			if(ss=dynamic_cast<Operation*>(v))
			{
				substitution(ss);
			}
			else
			{
				g_os << "// expression *****ERROR*****" << endl;
			}
		}
		else
		{
			g_os << "// lvalue *****ERROR*****" << endl;
		}
	}
	else
	{
		g_os << "// value *****ERROR*****" << endl;
	}
}

Keep<Identity> Class::findSuper(Text id)
{
	for(Class * s : supers)
	{
		while(s != nullptr)
		{
			auto iter = s->scope.find(id);
			if(iter != scope.end()) return iter->second;
			Identity * res = s->findSuper(id);
			if(res != nullptr) return res;
		}
	}
	return nullptr;
}
bool Class::typeTest(Class * id)
{
	if(id == this) return true;
	for(Class * su : supers)
	{
		if(su->typeTest(id)) return true;
	}
	return false;
}
bool Class::typeTest(Identity * id)
{
	Class * type = dynamic_cast<Class*>(id);
	if(type == nullptr) return false;
	return typeTest(type);
}
Keep<Identity> Class::find(Text id)
{
	auto iter = scope.find(id);
	if(iter != scope.end())
	{
		return iter->second;
	}

	Identity * res = findSuper(id);
	if(res != nullptr) return res;

	if(parent != nullptr) return parent->find(id);
	throw ErrMessage("Invalid identifier %s", TSZ(id).c_str());
}
bool Class::isParent(Identity * id)
{
	if(this == id) return true;
	if(parent == nullptr) return false;
	return parent->isParent(id);
}
bool Class::parentReference(Identity * id, TText * pref)
{
	if(this == id) return true;
	if(parent == nullptr) return false;
	*pref << 'p' << parent->name << "->";
	return parent->parentReference(id, pref);
}
TText Class::toString(Class * sc)
{
	if(sc == nullptr) return name;
	if(sc->isParent(parent)) return name;
	if(typeTest(sc)) return name;
	if(parent == nullptr) throw ErrMessage("is not exists in %s", sc->toString(nullptr).c_str());
	TText out = parent->toString(sc);
	out << "::" << name;
	return out;
}
Keep<Value> Class::getSizeOf(Function * szof)
{
	Keep<Value> exp = nullptr;
	for(Class * s : supers)
	{
		if(exp == nullptr) exp = s->getSizeOf(szof);
		else exp = Operation::calculate(move(exp), '+', s->getSizeOf(szof));
	}
	for(Value * v :fields)
	{
		if(exp == nullptr) exp = v->getSizeOf(szof);
		else exp = Operation::calculate(move(exp), '+', v->getSizeOf(szof));
	}
	if(exp == nullptr) return &g_root.vZero;
	return exp;
}
void Class::readSwitch()
{
	AText switch_name = g_is.readWord('(');
	Keep<Value> value = readExp(')');
	g_is.skipSpace('{');

	Keep<Switch> s = _new Switch(this, switch_name, value);
	fields.push_back(s);

	for(;;)
	{
		switch(g_is.skipSpace(" }"))
		{
		case '}': return;
		case ' ':
			if (g_is.readWord(' ') != "case") 
				throw ErrMessage("Need case keyword");
			AText id = g_is.readWord(':');
			if (!id.numberonly()) throw ErrMessage("Is not number");
			int number = id.to_int();
			auto res = s->switches.insert({ number, nullptr });
			if (!res.second) throw ErrMessage("Already exists case.");
			res.first->second = readClass();
			break;
		}
	}
}
void Class::readIf()
{
	AText if_name = g_is.readWord('(');
	Keep<Value> value = readExp(')');
	Keep<Class> cls = readClass();
	fields.push_back(_new If(this, if_name, value, move(cls)));
}
Keep<Class> Class::readClass()
{
	AText id;
	char det = g_is.readWord(&id, ":({");
	Keep<Class> s = _new Class(id, this, false);
	if(det == ':') // inherit
	{
		AText suname;
		det = g_is.readWord(&suname, "({");
		s->supers.emplace_back(cast<Class>(find(suname)));
	}
	if(det == '(')
	{
		g_is.skipSpace(')');
		g_is.skipSpace('{');
	}

	auto res = scope.insert(id, nullptr);
	if(!res.second)
	{
		throw ErrMessage("%s already declared.", id.c_str());
	}

	s->readClassContents('}');
	g_is.skipSpace(';');
	res.first->second = s;
	subclasses.push_back(s);
	return s;
}
Keep<Class> Class::readType(char endcode)
{
	AText id = g_is.readWord(endcode);
	return cast<Class>(find(id));
}
void Class::readVariableNames(Class * type)
{
	AText id;
	for(;;)
	{
		Value * v;
		char det = g_is.readWord(&id, ",;[");
		if(id == "?")
		{
			fields.push_back(v = _new UnknownValue(type));
		}
		else if ('0' <= *id && *id <= '9')
		{
			int n = id.to_int();
			fields.push_back(v = _new Constant(type, n));
		}
		else
		{
			auto res = scope.insert(id, nullptr);
			if(!res.second)
			{
				throw ErrMessage("%s Already exists", id.c_str());
			}

			Field * inst = _new Field(this, type, id);
			res.first->second = inst;
			v = inst;
			if(id[0] == '$') locals.push_back(inst);
			else fields.push_back(inst);
		}
		if(det == '[')
		{
			v->setArray(readExp(']'));
			det = g_is.skipSpace(",;");
		}
		if(det == ';')
			break;
	}
}
void Class::readClassContents(char endcode)
{
	const char endcodes[] ={ ' ', '@', endcode, '\0' };
	AText id;

	while (!g_is.eof())
	{
		switch(g_is.skipSpace(endcodes))
		{
		case '@':
		{
			Keep<Value> lvalue = readExp('=');
			Keep<Value> rvalue = readExp(';');
			cast<LValue>((Value*)lvalue)->setPrepare();
			fields.push_back(Operation::calculate(move(lvalue), '=', move(rvalue)));
			break;
		}
		case ' ':
			switch(g_is.readWord(&id, " =;"))
			{
			case ' ':
				if (id.empty()) continue;
				if(id == "typedef")
				{
					char chr;
					AText id2;
					do
					{
						chr = g_is.readWord(&id2, ";,");
						auto res = scope.insert(id2, nullptr);
						if(res.second)
						{
							res.first->second = _new ClassExtern(id2, this);
						}
						else
						{
							throw ErrMessage("%s Already exists.", id2.c_str());
						}
					}
					while(chr != ';');
					continue;
				}
				else if(id == "struct")
				{
					readClass();
					continue;
				}
				else if(id == "define")
				{
					AText na = g_is.readWord(' ');
					auto res = scope.insert(na, nullptr);
					if(!res.second) throw ErrMessage("%s Already exists", na.c_str());
					auto * value = _new ValueConstField(this, move(na), cast<Constant>(readExp(';')));
					res.first->second = value;
					consts.push_back(value);
					continue;
				}
				else if(id == "switch")
				{
					readSwitch();
					continue;
				}
				else if(id == "if")
				{
					readIf();
					continue;
				}
				readVariableNames(cast<Class>(find(id)));
				break;
			default:
				if (id.empty()) continue;
				break;
			}
			break;
		default: return;
		}
	}
}
void Class::writeSwitch(Switch *sw)
{
	g_os << "char v_"<< sw->name<<'['<<sw->getSizeOf(&g_root.fnSizeOf)->toString(this)<<"];" << endl;

	g_os << "template <typename LAMBDA> void " << sw->name << "(LAMBDA lambda)" << endl;
	g_os << '{' << endl;
	g_os.indent(4);
	g_os << sw->toString(this) << endl;
	g_os << "{"<< endl;
	for(auto & p : sw->switches)
	{
		g_os << "case " << std::to_string(p.first) << ':';
		g_os << "lambda(("<< p.second->toString(this) <<"*)v_"<<sw->name<<");";
		g_os << "break;"<< endl;
	}
	g_os << "}"<< endl;
	g_os.indent(-4);
	g_os << '}' << endl;

	g_os << "template <typename TYPE> bool is_" << sw->name << "();" << endl;
	for (auto & p : sw->switches)
	{
		g_os << "template <> bool is_" << sw->name << "<"<< p.second->toString(this) <<">()" << endl;
		g_os << "{" << endl;
		g_os.indent(4);
		g_os << "return (" << sw->value->toString(this) << ") == " << std::to_string(p.first) << ';' << endl;
		g_os.indent(-4);
		g_os << '}' << endl;
	}
}
void Class::writeIf(If *iv)
{
	g_os << iv->contain->toString(this) << " v_"<< iv->name <<';' << endl;
	g_os << "bool "<<iv->name << "()" << endl;
	g_os << '{' << endl;
	g_os.indent(4);
	g_os << "return " << iv->condition->toString(this) << ';' << endl;
	g_os.indent(-4);
	g_os << '}' << endl;
}
void Class::writeClass()
{
	g_os << "struct " << name;
	if(!supers.empty())
	{
		g_os << ':';
		bool first = true;
		for(Class * s :supers)
		{
			if(first) first = false;
			else g_os << ',';
			g_os << s->toString(this);
		}
	}
	g_os << endl << '{' << endl;
	g_os.indent(4);
	writeClassContents();
	writeMethods();
	g_os.indent(-4);
	g_os << "};" << endl;
}
void Class::writeClassFields()
{
	for(Value * v : fields)
	{
		switch_type(v, [this](Field* inst)
		{
			if(inst->name[0] == '$') return;
			g_os << inst->type->name << ' '<<inst->name;
			Value *arrsize;
			if(arrsize = inst->getArraySize())
			{
				g_os <<'['<< inst->getMaxValue()->toString(this) <<']';
			}
			g_os<< ';' << endl;


		}, [this](UnknownValue* lv)
		{
		}, [this](Constant* cv)
		{
		}, [this](Switch* sw)
		{
			writeSwitch(sw);
		}, [this](If* iv)
		{
			writeIf(iv);
		}, [this](Operation* ss)
		{
		});
	}
}
void Class::writeClassContents()
{
	for(ValueConstField * c : consts)
	{
		g_os << "static constexpr "<< c->getType()->toString(this)
			<< ' ' << c->name
			<< '=' << c->value->toString(this) << ';' << endl;
	}
	for(Class * cls : subclasses)
	{
		if(dynamic_cast<ClassExtern*>(cls))
		{
			g_os << "// extern " << cls->name << ';' << endl;
		}
		else
		{
			cls->writeClass();
		}
	}
	writeClassFields();
}
void Class::writeMethods()
{
	if(fields.empty() && supers.size() == 1) return;

	g_os << "template <typename OS> struct out_sizeof { static constexpr size_t value = ";
	g_os << getSizeOf(&g_root.fnOutSizeOf)->toString(this) << "; };"<< endl;

	// use
	for(Value * v : fields)
	{
		Switch * sw = dynamic_cast<Switch*>(v);
		if(sw == nullptr) continue;

		g_os<<"template <typename T> T* use_"<<sw->name<<"();" << endl;
		for(auto & pair : sw->switches)
		{
			TText clsname = pair.second->toString(this);
			g_os<<"template <> " << clsname << "* use_"<<sw->name<<"<"<<clsname<<">()" << endl;
			g_os << '{' << endl;
			g_os.indent(4);
			g_os << clsname << "* v = ("<<clsname<<"*)v_" << sw->name << ';' << endl;
			g_os << sw->value->toString(this)<<'=' << std::to_string(pair.first) << ';' << endl;
			g_os << "return v;"<< endl;
			g_os.indent(-4);
			g_os << '}' << endl;
		}
	}

	// write to
	g_os << "template <typename OS> void writeTo(OS & os";
	if(parent != &g_root)
	{
		TText parentname = parent->toString(this);
		g_os << ", "<<parentname<<"* p"<<parentname;
	}
	g_os<<")" << endl;
	g_os << '{' << endl;
	g_os.indent(4);
	for(Field * inst : locals)
	{
		g_os << inst->type->toString(this) << ' ' << inst->toString(this) << ';' << endl;
	}
	for(Class * s : supers)
	{
		TText supertypename = s->parent->toString(this);
		g_os << s->toString(this) << "::writeTo(os, "<<supertypename<<"* p"<<supertypename<<");" << endl;
	}
	for(Value * v : fields)
	{
		switch_type(v, [this](Field* v)
		{
			if(v->name[0] == '$')
			{
				if(v->getArraySize()) throw ErrMessage("Not supported yet");
				g_os << v->type->toString(this)<<' ' << v->name.subarr(1) <<';' << endl;
				return;
			}

			TText value = v->toString(this);
			TText type = v->type->toString(this);
			TText typeoutsize = g_root.fnOutSizeOf.call({ v->type })->toString(this);
			
			if(v->prepared())
			{
				TText nvalue;
				value.replace(&nvalue, '.', '_');
				g_os <<"char * $$mark_"<<nvalue<<" = os;" << endl;
			}

			if(dynamic_cast<ClassExtern*>((Class*)v->type))
			{
				Value *arrsize;
				if(arrsize = v->getArraySize())
				{
					g_os << '{' << endl;
					g_os.indent(4);
					g_os << type << "* i=" << value << ';' << endl;
					g_os << type << "* end= i + " << arrsize->toString(this) << ';' << endl;
					g_os << "for(;i != end;i++) ";
					if(v->prepared())
					{
						g_os<<"os += "<<typeoutsize<<";"<< endl;
					}
					else
					{
						g_os<<"os.write(*i);"<<endl;
					}
					g_os.indent(-4);
					g_os << '}' << endl;
				}
				else
				{
					if(v->prepared())
					{
						g_os<<"os += "<<typeoutsize<<";"<< endl;
					}
					else
					{
						g_os<<"os.write("<<v->toString(this)<<");"<<endl;
					}
				}
			}
			else
			{
				if(v->prepared())
				{
					g_os<<"os += "<<typeoutsize<<";"<< endl;
				}
				else
				{
					g_os << v->toString(this) <<".writeTo(os";
					// TODO: find parent
					g_os <<");"<< endl;
				}
			}


		}, [this](UnknownValue* v)
		{
			TText type = v->type->toString(this);
			Value *arrsize;
			if(arrsize = v->getArraySize())
			{
				g_os << '{' << endl;
				g_os.indent(4);
				g_os << "size_t end = " << arrsize->toString(this) << ';' << endl;
				g_os << "for(size_t i=0 ;i != end;i++) os.write<"<<type<<">(0);"<<endl;
				g_os.indent(-4);
				g_os << '}' << endl;
			}
			else
			{
				g_os<<"os.write<"<<type<<">(0);"<<endl;
			}
		}, [this](Constant* v)
		{
			TText value = v->toString(this);
			TText type = v->type->toString(this);
			Value *arrsize;
			if(arrsize = v->getArraySize())
			{
				g_os << '{' << endl;
				g_os.indent(4);
				g_os << "size_t end = " << arrsize->toString(this) << ';' << endl;
				g_os << "for(size_t i=0 ;i != end;i++) os.write<"<<type<<">(("<<type<<')'<<value<<");"<<endl;
				g_os.indent(-4);
				g_os << '}' << endl;
			}
			else
			{
				g_os<<"os.write<"<<type<<">(("<<type<<')'<<value<<");"<<endl;
			}
		}, [this](Switch* v)
		{
			g_os << '{' << endl;
			g_os.indent(4);
			g_os << "auto _this = this;" << endl;
			g_os << v->name << "([&](auto * p){ p->writeTo(os,_this); });" << endl;
			g_os.indent(-4);
			g_os << '}' << endl;
		}, [this](If* v)
		{
			g_os << "if(" << v->name << "()) v_"<<v->name<<".writeTo(os,this);" << endl;
		}, [this](Operation* ss)
		{
			g_os<<ss->toString(this) <<';'<< endl;
		});
	}
	for(Value * v : fields)
	{
		LValue* inst = dynamic_cast<LValue*>(v);
		if(inst == nullptr) continue;
		if(!inst->prepared()) continue;
		TText value = v->toString(this);
		value.change('.', '_');
		
		if (dynamic_cast<ClassExtern*>((Class*)v->type))
		{
			g_os <<"OS($$mark_"<<value<<").write("<<value<<");"<<endl;
		}
		else
		{
			g_os << value << ".writeTo(OS($$mark_"<<value<<"));"<<endl;
		}
	}
	g_os.indent(-4);
	g_os << '}' << endl;

	// read from
	g_os << "template <typename IS> void readFrom(IS & is)" << endl;
	g_os << '{' << endl;
	g_os.indent(4);
	for(Value * v : fields)
	{
		switch_type(v, [this](Field* v)
		{
			if(v->name[0] == '$') return;
			
			TText value = v->toString(this);
			TText type = v->type->toString(this);
			if (dynamic_cast<ClassExtern*>((Class*)v->type))
			{
				Value * arrsize;
				if (arrsize = v->getArraySize())
				{
					g_os << '{' << endl;
					g_os.indent(4);
					g_os << type << "* i=" << value << ';' << endl;
					g_os << type << "* end= i + " << arrsize->toString(this) << ';' << endl;
					g_os << "for(;i != end;i++) *i = is.read<" << type << ">();" << endl;
					g_os.indent(-4);
					g_os << '}' << endl;
				}
				else
				{
					g_os << value << " = is.read<" << type << ">();" << endl;
				}
			}
			else
			{
				g_os << v->toString(this) <<".readFrom(is";
				// TODO: find parent
				g_os <<");"<< endl;
			}
		}, [this](UnknownValue* v)
		{
			TText type = v->type->toString(this);
			Value *arrsize;
			if(arrsize = v->getArraySize())
			{
				g_os << '{' << endl;
				g_os.indent(4);
				g_os << "size_t end = " << arrsize->toString(this) << ';' << endl;
				g_os << "for(size_t i=0 ;i != end;i++) is.skip<"<<type<<">();"<<endl;
				g_os.indent(-4);
				g_os << '}' << endl;
			}
			else
			{
				g_os<<"is.skip<"<<type<<">(); // unknown"<<endl;
			}
		}, [this](Constant* v)
		{
			TText value = v->toString(this);
			TText type = v->type->toString(this);
			Value *arrsize;
			if(arrsize = v->getArraySize())
			{
				g_os << '{' << endl;
				g_os.indent(4);
				g_os << "size_t end = " << arrsize->toString(this) << ';' << endl;
				g_os << "for(size_t i=0 ;i != end;i++)"<<endl;
				g_os << "    if(is.read<"<<type<<">() != ("<<type<<')'<<value<<");"<<endl;
				g_os << "        throw ::kr::InvalidSourceException(); "<<endl;
				g_os.indent(-4);
				g_os << '}' << endl;
			}
			else
			{
				g_os<<"if(is.read<"<<type<<">() != ("<<type<<')'<< value <<")";
				g_os<<"throw ::kr::InvalidSourceException();"<<endl;
			}
		}, [this](Switch* v)
		{
			g_os << v->name << "([&is](auto * p){ p->readFrom(is); });" << endl;
		}, [this](If* v)
		{
			g_os << "if(" << v->name << "()) v_"<<v->name<<".readFrom(is);" << endl;
		}, [this](Operation* ss)
		{
		}
		);
	}
	g_os.indent(-4);
	g_os << '}' << endl;
}
Keep<Value> Class::readExp(char endcode)
{
	Keep<Value> out = nullptr;
	AText id;
	int oper = 0;

	for(;;)
	{
		switch(oper)
		{
		case '&': if(g_is.skipSpace(" &") == '&') oper = '&&'; break;
		case '|': if(g_is.skipSpace(" |") == '|') oper = '||'; break;
		}
		char chr = g_is.readWord(&id);

		Keep<Value> v = nullptr;
		if('0' <= *id && *id <= '9')
		{
			int number = id.to_int();
			v = _new Constant(&g_root.clsInt, number);
		}
		else if(id == "$POINTER")
		{
			v = _new StringExpression(&g_root.clsInt, "os");
		}
		else
		{
			Keep<Identity> finded = find(id);
			while(chr == '.')
			{
				chr = g_is.readWord(&id);
				finded = finded->find(id);
			}
			v = cast<Value>(finded);
		}

		if(out == nullptr) out = v;
		else out = Operation::calculate(out, oper, v);

		switch(chr)
		{
		case '-': case '+':
		case '*': case '/':
		case '&': case '|':
			oper = chr;
			break;
		default:
			must(chr, endcode);
			return out;
		}
	}
}
bool Class::isDynamicSize() noexcept
{
	for (Class * su : supers)
	{
		if (su->isDynamicSize()) return true;
	}
	for (Value * f : fields)
	{
		if (f->isDynamicSize()) return true;
	}
	return false;
}
