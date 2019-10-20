
#include <KR3/main.h>
#include <KR3/initializer.h>

#include <KR3/util/path.h>

#ifdef WIN32

#include <KRHttp/httpd.h>
#include <KRHttp/wspage.h>

using namespace kr;

class MyClient : public WebSocketSession
{
public:
	MyClient(Socket * args) noexcept
		: WebSocketSession(args)
	{
	}
	void onData(WSStream & stream) override
	{
	}
	void onError(Text funcname, int error) noexcept override
	{
	}
};

class MyPage :public WebSocketPage
{
public:
	WebSocketSession * onAccept(Socket * args) override
	{
		return new MyClient(args);
	}
};

void webServerMain() noexcept
{
	Initializer<Socket> _init;
	HttpServer g_server;

	currentDirectory.set(L"D:\\Projects\\git\\k-square");
	g_server.open(80);
	g_server.attachPage("/test", new MyPage);
	ThreadHandle::getCurrent()->terminate();

}

#endif