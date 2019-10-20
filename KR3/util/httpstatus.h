#pragma once

#include <KR3/main.h>

namespace kr
{
	enum HttpStatus:uint
	{
		HttpUndefined = 0,
		HttpOk = 200,

		HttpMovePermanently = 301,
		HttpFound = 302,
		HttpNotModified = 304,

		HttpBadRequest = 400,
		HttpForbidden = 403,
		HttpNotFound = 404,
		HttpLengthRequired = 411,

		HttpServerInternalError = 500,
		HttpMethodNotImplemented = 501,
		HttpVersionNotSupported = 505,
		HttpCustomError = 555,
	};
}