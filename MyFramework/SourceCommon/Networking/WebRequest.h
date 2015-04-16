//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __WebRequest_H__
#define __WebRequest_H__

#if MYFW_ANDROID || MYFW_BLACKBERRY || MYFW_BADA || MYFW_IOS || MYFW_OSX || MYFW_EMSCRIPTEN
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // for gethostbyname()
#include <errno.h>
#endif

// TODONACLUPDATE: try to unignore UPDSocket, webrequest and stunclient.

#define MAX_WebRequestLength    100000
#define MAX_WebPageLength       10000

class WebRequestObject
{
protected:
    bool m_Initialized;
    bool m_WaitingForGetHostByName;

    struct in_addr m_InAddr;
    sockaddr_in m_ServerAddress;
    int m_Sock;
    char* m_Hostname;
    unsigned short m_Port;
    bool m_SocketConnected;
    bool m_RequestSent;
    bool m_ResponseReady;

    char m_PageWanted[MAX_WebPageLength+1];
    bool m_RequestPending;
    
    char* m_pPointerToActualResponseInsideBuffer;
    int m_CharactersReceived;
    char* m_pBuffer;
    
    bool m_CloseConnectionOnNextTick;
    bool m_SomethingWentWrong;

    void CreateSocket();
    bool ConnectSocket();

private:
    static void* Thread_GetHostByName(void* obj);

public:
    WebRequestObject();
    ~WebRequestObject();

    void Init(const char* host, unsigned short port);
    void Reset();
    void RequestCloseConnection();

    void RequestStart(const char* page);
    void RequestAddPair(const char* var, int value); // will url encode var and value.
    void RequestAddPair(const char* var, const char* value); // will url encode var and value.
    void RequestEnd();

    void RequestWebPage(const char* page, ...); // make sure all elements are urlencoded
    void Tick(const char* customuseragentchunk = 0);

    bool IsBusy();
    bool DidSomethingGoWrong() { return m_SomethingWentWrong; }

    void ClearResult();
    void InsertString(const char* string, int len);

    char* GetResult();
};

#endif //__WebRequest_H__
