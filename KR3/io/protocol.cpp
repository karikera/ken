#include "stdafx.h"
#include "protocol.h"


namespace
{
	using PROTOCOL_GENERATOR = kr::Protocol* (*)(kr::Text16 uri);

}

int __dummy = 0;