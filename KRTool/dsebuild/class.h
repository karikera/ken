#pragma once

#include "identity.h"
#include <KR3/main.h>
#include <KR3/data/map.h>


class Class:public Identity
{
public:
	Class(AText name, Class * p, bool numeric);
	virtual ~Class();
	Keep<Identity> findSuper(Text id);
	bool typeTest(Class * id);
	bool typeTest(Identity * id);
	Keep<Identity> find(Text id) override;
	bool isParent(Identity * id);
	bool parentReference(Identity * id, TText * pref);
	TText toString(Class * scope) override;
	Keep<Value> getSizeOf(Function * szof) override;
	void readSwitch();
	void readIf();
	Keep<Class> readClass();
	Keep<Class> readType(char endcode);
	void readVariableNames(Class * type);
	void readClassContents(char endcode);
	void writeSwitch(Switch *sw);
	void writeIf(If *iv);
	void writeClass();
	void writeClassFields();
	void writeClassContents();
	void writeMethods();
	Keep<Value> readExp(char endcode);
	bool isDynamicSize() noexcept;

	const bool numeric;
	Keep<Class> const parent;
	list<Keep<Class>> supers;
	list<Keep<Class>> subclasses;
	list<Keep<Value>> fields;
	list<Keep<ValueConstField>> consts;
	list<Keep<Field>> locals;
	kr::ReferenceMap<Text, Keep<Identity>> scope;
};
class ClassExtern:public Class
{
public:
	ClassExtern(AText name, Class * parent);
	~ClassExtern() override;
	Keep<Identity> find(Text id) override;
	Keep<Value> getSizeOf(Function * szof) override;
};
