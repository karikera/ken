#pragma once

#include <KR3/main.h>

namespace kr
{
	//
	// Redirect Status code numbers that need to be defined.
	//

	/// <devdoc>
	///    <para>Contains the values of status
	///       codes defined for the HTTP protocol.</para>
	/// </devdoc>
	//UEUE : Any int can be cast to a HttpStatusCode to allow checking for non http1.1 codes.
	enum class HttpStatus :int
	{
		Undefined = -1,

		//
		// Informational 1xx
		//
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Continue = 100,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		SwitchingProtocols = 101,

		//
		// Successful 2xx
		//
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		OK = 200,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Created = 201,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Accepted = 202,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		NonAuthoritativeInformation = 203,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		NoContent = 204,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		ResetContent = 205,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		PartialContent = 206,

		//
		// Redirection 3xx
		//
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		MultipleChoices = 300,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Ambiguous = 300,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		MovedPermanently = 301,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Moved = 301,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Found = 302,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Redirect = 302,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		SeeOther = 303,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		RedirectMethod = 303,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		NotModified = 304,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		UseProxy = 305,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Unused = 306,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		TemporaryRedirect = 307,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		RedirectKeepVerb = 307,

		//
		// Client Error 4xx
		//
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		BadRequest = 400,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Unauthorized = 401,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		PaymentRequired = 402,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Forbidden = 403,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		NotFound = 404,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		MethodNotAllowed = 405,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		NotAcceptable = 406,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		ProxyAuthenticationRequired = 407,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		RequestTimeout = 408,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Conflict = 409,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		Gone = 410,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		LengthRequired = 411,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		PreconditionFailed = 412,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		RequestEntityTooLarge = 413,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		RequestUriTooLong = 414,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		UnsupportedMediaType = 415,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		RequestedRangeNotSatisfiable = 416,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		ExpectationFailed = 417,

		UpgradeRequired = 426,

		//
		// Server Error 5xx
		//
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		InternalServerError = 500,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		NotImplemented = 501,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		BadGateway = 502,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		ServiceUnavailable = 503,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		GatewayTimeout = 504,
		/// <devdoc>
		///    <para>[To be supplied.]</para>
		/// </devdoc>
		HttpVersionNotSupported = 505,
	};
}