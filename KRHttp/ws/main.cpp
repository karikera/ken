#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <mysql/mysql.h>

#include "main.h"
#include "client.h"
#include "channel.h"
#include "base64.h"

void ServerProc()
{
    Socket wsserver = nullptr;
    Socket flserver = nullptr;
    
    try
    {
        wsserver = new SOCKET;
        if(wsserver->Invalid()) throw "WS Server creation failed.";
        if(!wsserver->SetServer("192.168.0.2",14128)) throw "WS Server bind failed.";
        wsserver->SetTimer(2,0);
        cout << "WS Server started." << endl;

        flserver = new SOCKET;
        if(flserver->Invalid()) throw "FL Server creation failed.";
        if(!flserver->SetServer("192.168.0.2",843)) throw "FL Server bind failed.";
        flserver->SetTimer(2,0);
        cout << "FL Server started." << endl;

        CChannel::Init();
        CClient::Init();

        fd_set fds_init;
        FD_ZERO(&fds_init);
        FD_SET(STDIN_FILENO, &fds_init);
        FD_SET((int)flserver,&fds_init);
        FD_SET((int)wsserver,&fds_init);
        
        int maxfd = STDIN_FILENO;
        if((int)flserver > maxfd) maxfd = (int)flserver;
        if((int)wsserver > maxfd) maxfd = (int)wsserver;
        maxfd++;

        for(;;)
        {
            fd_set fds = fds_init;
            timeval tvOneSecond={1,0};
            
            int state = select(maxfd, &fds, NULL, NULL, &tvOneSecond);
            if(state == 0) continue;
            
            if(FD_ISSET(STDIN_FILENO, &fds))
            {
                CommandProcess();
            }
            else if(FD_ISSET((int)wsserver, &fds))
            {
                IPADDRESS ip;
                Socket client = wsserver->Accept(&ip);
                if(client->Invalid()) continue;
                new CClient(client,ip);
            }
            else if(FD_ISSET((int)flserver, &fds))
            {
                char temp[256];
                IPADDRESS ip;
                Socket client = flserver->Accept(&ip);
                int len = client->Receive(temp,255,0);
                if(len == -1)
                {
                    delete client;
                    continue;
                }

                temp[len+1] = '\0';
                delete client;
            }
        }
    }
    catch(...)
    {
        cout << "Server terminating." << endl;
        g_bQuit = true;
        if(wsserver != NULL) delete wsserver;
        if(flserver != NULL) delete flserver;
        CClient::Quit();
        CChannel::Quit();
        cout << "Server terminated." << endl;
        throw;
    }
}
