#include "identity.h"


Identity::Identity(AText name) noexcept
	:name(move(name))
{
}
Identity::~Identity() noexcept
{
}
void Identity::noneed() noexcept
{
	if(getReferenceCount() == 0)
	{
		delete this;
	}
}
TText Identity::toString(Class * scope)
{
	return name;
}
