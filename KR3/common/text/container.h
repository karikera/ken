#pragma once

#include "../container.h"

#define INHERIT_ARRAY() \
	INHERIT_COMPONENT(); \
	using typename Super::Self; 

