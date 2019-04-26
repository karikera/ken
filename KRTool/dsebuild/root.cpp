#include "root.h"

RootClass::RootClass()
	:Class("root", nullptr, false)
	, clsInt("int", &g_root)
	, clsBool("bool", &g_root)
	, clsChar("char", &g_root)
	, clsShort("short", &g_root)
	, clsLong("long", &g_root)
	, clsPointer("char*", &g_root)
	, fnSizeOf(&clsInt, "sizeof", 1)
	, fnOutSizeOf()
	, fnMaximumOf(&clsInt, "maximumof", 1)
	, fnMaxSelect(&clsInt, "meta::maximum", -1)
	, vZero(&clsInt, 0)
	, vOne(&clsInt, 1)
{
	Identity* lists[] ={ &clsInt, &clsBool, &clsChar, &clsShort, &clsLong };
	for(Identity * id : lists)
	{
		scope.insert(id->name, id);
		id->AddRef();
	}
	scope.insert("Pointer", &clsPointer);
	
	clsPointer.AddRef();
	fnSizeOf.AddRef();
	fnOutSizeOf.AddRef();
	fnMaximumOf.AddRef();
	fnMaxSelect.AddRef();
	vZero.AddRef();
	vOne.AddRef();
	g_root.AddRef();
}

RootClass g_root;
