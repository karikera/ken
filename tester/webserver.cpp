
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
	MyClient(HttpClient * client) noexcept
		: WebSocketSession(client)
	{
	}
	void onText(Text buffer) override
	{
		cout << buffer << endl;
		writeText(buffer);
		flush();
	}
	void onError(Text funcname, int error) noexcept override
	{
	}
};

class MyPage :public WebSocketPage
{
public:
	WebSocketSession * onAccept(HttpClient * client) override
	{
		return new MyClient(client);
	}
};


EventPump* pump = EventPump::getInstance();

void webServerMain() noexcept
{
	Initializer<Socket> _init;
	HttpServer g_server(u".");


	currentDirectory.set(L"D:\\Downloads\\ME\\complex\\ken\\tester");
	g_server.open(80);
	g_server.onPage("/test2", [](HttpClient* client) {
		pump->post([client = (Keep<HttpClient>)client]{
			client->sendText("<script>const socket = new WebSocket('ws://localhost/test');"
				"socket.onopen=()=>{console.log('onopen'); socket.send('test');};"
				"socket.onclose=()=>{console.log('onclose');};"
				"socket.onmessage=(ev)=>{console.log('onmessage: '+ev.data);};"
				"</script>");
			});
		});
	g_server.attachPage("/test", new MyPage);


	pump->messageLoop();
}

#endif