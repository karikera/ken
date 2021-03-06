#include "stdafx.h"

#ifdef NO_USE_SOCKET

EMPTY_SOURCE

#else

#include "fslpolicy.h"


using namespace kr;

void FSLPolicyClient::onRead() noexcept
{
	TText temp;
	Text text = m_receive.retype<char>()->readwith('>', &temp);
	if (text == nullptr) return;
	if (text == "<policy-file-request/>")
	{
		Text text = "<?xml version=\'1.0\'?><cross-domain-policy><allow-access-from domain=\'*\' to-ports=\'*\' /></cross-domain-policy>\0";

		if (Lock _lock = lock())
		{
			_lock.write(text);
			_lock.flush();
		}
	}
	else
	{
		lock().closeClient();
	}
}
void FSLPolicyClient::onError(Text func, int code) noexcept
{
}

FSLPolicyServer::FSLPolicyServer() noexcept
{
}
MTClient* FSLPolicyServer::onAccept(Socket * socket) noexcept
{
	return _new FSLPolicyClient(socket);
}
void FSLPolicyServer::onError(Text func, int code) noexcept
{
}

#endif