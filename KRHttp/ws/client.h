
#pragma once

#include "main.h"
#include "sockstream.h"
#include "ruadb.h"
#include "wsstream.h"

#include <unordered_map>
#include <pthread.h>

enum PacketSend
{
    PSLogin,PSJoin,PSRead,PSWrite,PSOut,PSChannel,PSChat,PSAction,
    PSMax
};

class CClient;
class CChannel;

class CClient
{
public:
    CClient(Socket sock,IPADDRESS ip);
    
    CSocketStream* Lock();
    void Unlock();
    
    void SetPosition(float x,float y);
    void Send(BUFFER buff);
    void ReadUserData(CWSOStream &stream);
    unsigned GetID();
    size_t GetChannelIndex();
    void SetChannelIndex(size_t nIndex);
    
    static void Init();
    static void Quit();
    
protected:
    
    CSocketStream m_sock;
    IPADDRESS m_ip;
    
    pthread_t m_thread;
    pthread_mutex_t m_mutex;
    
    CChannel *m_pChannel;
    size_t m_nChannelIndex;
    
    DB::MEMBER m_member;
    char m_dir[256];
    char * m_path;
    int m_playFrame;
    int m_startFrame;
    unsigned m_acode;
    
    POSITION m_pos;
    
    typedef void (CClient::*PROCESS_METHOD)(CWSIStream&);
    static PROCESS_METHOD fnDataProcess[];

    ~CClient();
    
    void _BroadPacket(PacketSend code,CWSIStream &stream);
    
    bool _DataProcess(WS_FRAME &frame,CWSIStream &stream);
    bool _HandShake();
    void * _Thread();
    static void * _ThreadStatic(void * pParam);
};
