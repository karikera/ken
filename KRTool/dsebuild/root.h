#pragma once

#include "class.h"
#include "function.h"
#include "value.h"

class RootClass:public Class
{
public:
	RootClass();
	ClassExtern clsInt;
	ClassExtern clsBool;
	ClassExtern clsChar;
	ClassExtern clsShort;
	ClassExtern clsLong;
	ClassExtern clsPointer;
	Function fnSizeOf;
	TemplateValue fnMaximumOf;
	TemplateValue fnMaxSelect;
	OutSizeOf fnOutSizeOf;
	Constant vZero,vOne;
};

extern RootClass g_root;
