
#include <KR3/main.h>
#include <KR3/initializer.h>

#include <KR3/util/path.h>

#ifdef WIN32

#include <KR3/http/httpd.h>
#include <KR3/http/wspage.h>

using namespace kr;

class MyClient : public WebSocketSession
{
public:
	MyClient(Socket * args) noexcept
		: WebSocketSession(args)
	{
	}
	void onBinary(Buffer buffer) override
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
	HttpServer g_server(u".");

	currentDirectory.set(L"D:\\Projects\\git\\k-square");
	g_server.open(80);
	g_server.attachPage("/test", new MyPage);
	ThreadHandle::getCurrent()->terminate();

}

#endif