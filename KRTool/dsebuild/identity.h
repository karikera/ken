#pragma once

#include "reference.h"
#include <KR3/main.h>

using kr::AText;
using kr::TText;
using kr::Text;
using kr::TSZ;
using kr::TSZ16;
using kr::Keep;

class Identity:public kr::Referencable<Identity>
{
public:
	Identity(AText name) noexcept;
	virtual ~Identity() noexcept;
	void noneed() noexcept;
	virtual Keep<Value> getSizeOf(Function * szof)=0;
	virtual Keep<Identity> find(Text id) =0;
	virtual TText toString(Class * scope);

	const AText name;
};

template <typename T, typename T2> T* cast(T2* o)
{
	T* n = dynamic_cast<T*>(o);
	if(n == nullptr)
	{
		o->noneed();
		throw ErrMessage("Type not match");
	}
	return n;
}

template <typename T, typename T2> T* cast(const Keep<T2>& o)
{
	T* n = dynamic_cast<T*>((T2*)o);
	if(n == nullptr)
	{
		throw ErrMessage("Type not match");
	}
	return n;
}
