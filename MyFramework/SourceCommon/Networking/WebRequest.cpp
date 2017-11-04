//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#if MYFW_WINDOWS
#define close closesocket
#endif

// TODONACLUPDATE: try to unignore UPDSocket, webrequest and stunclient.

#if !MYFW_WINDOWS && !MYFW_WP8
static int WSAGetLastError() { return errno; }
#endif

//#include <cstdio>
//#include <cstring>
//#include <cstdarg>
//
//#include <string>
//#include <vector>
//#include <string>
//#include <algorithm>

#include "URLEncode.h"

#if MYFW_NACL
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (NACL)\r\n\r\n"
#elif MYFW_WINDOWS
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (WIN32)\r\n\r\n"
#elif MYFW_LINUX
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (LINUX)\r\n\r\n"
#elif MYFW_ANDROID
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (ANDROID)\r\n\r\n"
#define HTTPInfoCustom "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (ANDROID %s)\r\n\r\n"
//g_pAndroidDeviceName
#elif MYFW_BLACKBERRY10
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (BB10)\r\n\r\n"
#define HTTPInfoCustom "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (BB10 %s)\r\n\r\n"
#elif MYFW_BLACKBERRY
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (BBPLAYBOOK)\r\n\r\n"
#elif MYFW_BADA
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (BADA)\r\n\r\n"
#elif MYFW_IOS
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (IOS)\r\n\r\n"
#elif MYFW_OSX
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (OSX)\r\n\r\n"
#elif MYFW_WP8
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (WP8)\r\n\r\n"
#elif MYFW_EMSCRIPTEN
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (EMSCRIPTEN)\r\n\r\n"
#endif

WebRequestObject::WebRequestObject()
{
    m_Initialized = false;
    m_WaitingForGetHostByName = false;

    m_Sock = 0;
    m_Hostname = 0;
    m_Port = -1;
    m_SocketConnected = false;
    m_RequestSent = false;
    m_ResponseReady = false;

    m_PageWanted[0] = 0;
    m_RequestPending = false;

    m_pPointerToActualResponseInsideBuffer = 0;

    m_CharactersReceived = 0;
    m_pBuffer = MyNew char[MAX_WebRequestLength];
    m_pBuffer[0] = 0;

    m_CloseConnectionOnNextTick = false;
    m_SomethingWentWrong = false;
}

WebRequestObject::~WebRequestObject()
{
    SAFE_DELETE_ARRAY( m_Hostname );
    SAFE_DELETE_ARRAY( m_pBuffer );
}

void WebRequestObject::Reset()
{
    if( m_Sock != 0 )
        close( m_Sock );
    m_Sock = 0;
    m_SocketConnected = false;
    m_RequestSent = false;
    m_ResponseReady = false;

    m_PageWanted[0] = 0;
    m_RequestPending = false;

    m_pPointerToActualResponseInsideBuffer = 0;

    m_CharactersReceived = 0;
    m_pBuffer[0] = 0;

    m_CloseConnectionOnNextTick = false;
    m_SomethingWentWrong = false;
}

void WebRequestObject::Init(const char* host, unsigned short port)
{
    MyAssert( m_Initialized == false );
    if( m_Initialized )
        return;

    MyAssert( m_WaitingForGetHostByName == false );
    if( m_WaitingForGetHostByName )
        return;

    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

#if 0 //OVERRIDE_WITH_LOCALHOST
    int len = strlen( "localhost" );
    m_Hostname = MyNew char[len+1];
    strcpy( m_Hostname, len+1, "localhost" );
    m_Port = 5103;
#else
    int len = (int)strlen( host );
    m_Hostname = MyNew char[len+1];
    strcpy_s( m_Hostname, len+1, host );
    m_Port = port;
#endif

#if MYFW_EMSCRIPTEN
    //m_Hostname = "208.83.209.11";
#endif
    LOGInfo( LOGTag, "WebRequestObject::Init() calling inet_addr m_Hostname = %s\n", m_Hostname );
    m_InAddr.s_addr = inet_addr( m_Hostname );
    if( m_InAddr.s_addr == INADDR_NONE )
    {
#if _DEBUG
        int err = WSAGetLastError();
        LOGError( LOGTag, "WebRequestObject::Init() inet_addr failed err=%d\n", err );
        LOGError( LOGTag, "WebRequestObject::Init() calling gethostbyname\n" );
#endif

#if USE_PTHREAD
        pthread_t mythread;
        pthread_create( &mythread, 0, &Thread_GetHostByName, this );
        pthread_detach( mythread );

        m_WaitingForGetHostByName = true;
#else
        Thread_GetHostByName( this );
#endif
    }
    else
    {
        memset( (char*)&m_ServerAddress, 0, sizeof(sockaddr_in) );
        m_ServerAddress.sin_family = AF_INET;
        m_ServerAddress.sin_port = htons( m_Port );
        m_ServerAddress.sin_addr.s_addr = m_InAddr.s_addr;

        // initialized from a raw ip address.
        m_Initialized = true;
        m_WaitingForGetHostByName = false;
    }
}

#if USE_PTHREAD
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void* WebRequestObject::Thread_GetHostByName(void* obj)
{
    WebRequestObject* pthis = (WebRequestObject*)obj;
    {
        bool error = false;
#if USE_PTHREAD
        pthread_mutex_lock( &mutex ); // gethostbyname doesn't seem thread-safe on bb10(maybe others).
#endif
        {
            // TODO: replace with getaddrinfo? 
            struct hostent* pHost = gethostbyname( pthis->m_Hostname );
            if( pHost )
            {
                //m_InAddr = *(struct in_addr*)(*pHost->h_addr_list);
                pthis->m_InAddr = *(struct in_addr*)*pHost->h_addr_list;
            }
            else
            {
                error = true;
            }
        }
#if USE_PTHREAD
        pthread_mutex_unlock( &mutex );
#endif

        if( error )
        {
#if _DEBUG && !MYFW_USING_WX
            //can't print in wx because we're on another thread.
            int err = WSAGetLastError();
            LOGError( LOGTag, "THREAD-WebRequestObject::Init() inet_addr and gethostbyname failed err=%d\n", err );
#endif
            pthis->m_SomethingWentWrong = true;
            pthis->m_RequestPending = false;
            return 0;
        }
    }

    memset( (char*)&pthis->m_ServerAddress, 0, sizeof(sockaddr_in) );
    pthis->m_ServerAddress.sin_family = AF_INET;
    pthis->m_ServerAddress.sin_port = htons( pthis->m_Port );
    pthis->m_ServerAddress.sin_addr.s_addr = pthis->m_InAddr.s_addr;

#if _DEBUG && !MYFW_USING_WX
    //can't print in wx because we're on another thread.
    LOGInfo( LOGTag, "THREAD-WebRequestObject::Init() ip=%d.%d.%d.%d\n",
        (pthis->m_InAddr.s_addr & 0x000000ff) >>  0,
        (pthis->m_InAddr.s_addr & 0x0000ff00) >>  8,
        (pthis->m_InAddr.s_addr & 0x00ff0000) >> 16,
        (pthis->m_InAddr.s_addr & 0xff000000) >> 24 );
#endif

    pthis->m_Initialized = true;
    pthis->m_WaitingForGetHostByName = false;

#if USE_PTHREAD
    pthread_exit( 0 );
#endif

    return 0;
}

void WebRequestObject::RequestCloseConnection()
{
    MyAssert( m_Sock != 0 );
    if( m_Sock != 0 )
        close( m_Sock );
    m_Sock = 0;

    m_CloseConnectionOnNextTick = true;
}

void WebRequestObject::CreateSocket()
{
#if MYFW_WINDOWS
    m_Sock = socket( AF_INET, SOCK_STREAM, 0 );//IPPROTO_TCP );
#else
    m_Sock = socket( AF_INET, SOCK_STREAM, 0 );
#endif

    if( m_Sock == 0 )
    {
#if _DEBUG
        int sockerr = WSAGetLastError();
        LOGError( LOGTag, "WebRequestObject::CreateSocket() failed m_Sock=%d errno=%d\n", m_Sock, sockerr );
#endif
        m_SomethingWentWrong = true;
        m_RequestPending = false;
        return;
    }

    LOGInfo( LOGTag, "WebRequestObject::CreateSocket() m_Sock=%d ip=%d.%d.%d.%d\n", m_Sock,
        (m_InAddr.s_addr & 0x000000ff) >>  0,
        (m_InAddr.s_addr & 0x0000ff00) >>  8,
        (m_InAddr.s_addr & 0x00ff0000) >> 16,
        (m_InAddr.s_addr & 0xff000000) >> 24 );

#if MYFW_WINDOWS || MYFW_WP8
    unsigned long i=1;
    int nonblockerr = ioctlsocket( m_Sock, FIONBIO, &i ); // set non-blocking
#elif MYFW_ANDROID || MYFW_BLACKBERRY || MYFW_IOS || MYFW_OSX || MYFW_EMSCRIPTEN || MYFW_LINUX
    int nonblockerr = fcntl( m_Sock, F_SETFL, O_NONBLOCK );
#elif SYSTEM_NDS
    int i=1;
    int nonblockerr = ioctl( m_Sock, FIONBIO, &i ); // set non-blocking
#endif

    if( nonblockerr == -1 )
    {
#if _DEBUG
        int sockerr = WSAGetLastError();
        LOGError( LOGTag, "WebRequestObject::CreateSocket() fcntl failed m_Sock=%d errno=%d\n", m_Sock, sockerr );
#endif
        close( m_Sock );
        m_Sock = 0;
        m_SomethingWentWrong = true;
        m_RequestPending = false;
        return;
    }
}

bool WebRequestObject::ConnectSocket()
{
    MyAssert( m_Sock != 0 );
    if( m_Sock == 0 )
    {
        LOGError( LOGTag, "WebRequestObject::ConnectSocket() m_Sock == 0\n" );
        return true; // error
    }

    int ret = connect( m_Sock, (sockaddr*)&m_ServerAddress, sizeof(sockaddr) );
    if( ret == -1 )
    {
        int err = WSAGetLastError();

        if( err == WSAEISCONN )
        {
            LOGInfo( LOGTag, "WebRequestObject::ConnectSocket() connected m_Sock = %d\n", m_Sock );
            m_SocketConnected = true;
            return false;
        }

        if( err != WSAEINPROGRESS && err != WSAEWOULDBLOCK && err != WSAEALREADY && err != WSAEINVAL && err != WSAEISCONN )
        {
            LOGError( LOGTag, "WebRequestObject::ConnectSocket() error = %d, m_Sock = %d\n", err, m_Sock );
            close( m_Sock );
            m_Sock = 0;
            m_SomethingWentWrong = true;
            m_RequestPending = false;
            return true; // error
        }

        LOGInfo( LOGTag, "WebRequestObject::ConnectSocket() still connecting m_Sock = %d\n", m_Sock );

        return true; // still connecting.
    }

    return true; // connected
}

void WebRequestObject::RequestStart(const char* page)
{
    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

    ClearResult();
    Reset();

    sprintf_s( m_PageWanted, MAX_WebPageLength, "%s", page );
    strcat_s( m_PageWanted, MAX_WebPageLength, "?" );
}

// will url encode var and value.
void WebRequestObject::RequestAddPair(const char* var, int value)
{
    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

    char valuestring[20];
    sprintf_s( valuestring, 20, "%d", value );

    RequestAddPair( var, valuestring );
}

// will url encode var and value.
void WebRequestObject::RequestAddPair(const char* var, const char* value)
{
    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

    char* varencoded = url_encode( var );
    char* valueencoded = url_encode( value );

    strcat_s( m_PageWanted, MAX_WebPageLength, varencoded );
    strcat_s( m_PageWanted, MAX_WebPageLength, "=" );
    strcat_s( m_PageWanted, MAX_WebPageLength, valueencoded );
    strcat_s( m_PageWanted, MAX_WebPageLength, "&" );

    free( varencoded );
    free( valueencoded );
}

void WebRequestObject::RequestEnd()
{
    LOGInfo( LOGTag, "WebRequestObject - RequestWebPage\n" );

    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

    m_PageWanted[ strlen(m_PageWanted)-1 ] = 0; // remove the final ? or &

    m_RequestPending = true;
    CreateSocket();
}

// make sure all elements are urlencoded
void WebRequestObject::RequestWebPage(const char* page, ...)
{
    LOGInfo( LOGTag, "WebRequestObject - RequestWebPage\n" );

    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

    ClearResult();
    Reset();

    m_RequestPending = true;

    va_list arg;
    va_start(arg, page);
#if MYFW_WINDOWS || MYFW_WP8
    vsnprintf_s( m_PageWanted, sizeof(m_PageWanted), _TRUNCATE, page, arg );
#else
    vsnprintf( m_PageWanted, sizeof(m_PageWanted), page, arg );
#endif
    va_end(arg);
    m_PageWanted[MAX_WebPageLength+1-1] = 0; // vsnprintf_s might do this, but docs are unclear

    CreateSocket();
}

void WebRequestObject::Tick(const char* customuseragentchunk)
{
    if( m_Initialized == false )
        return;

    if( m_RequestPending == false )
        return;

    if( m_WaitingForGetHostByName == true )
    {
        return;
    }

    if( m_Sock == 0 )
        return;

    if( m_SocketConnected == false )
        ConnectSocket();

    if( m_SocketConnected && m_RequestSent == false )
    {
        char reqbuf[MAX_WebPageLength]; // need minimum of 4(GET ) + 256(webpage+args) + strlen( HTTPInfo ); // 357
        //char* pageencoded = url_encode( m_PageWanted );
        char* pageencoded = m_PageWanted;
        if( customuseragentchunk == 0 )
        {
            sprintf_s( reqbuf, MAX_WebPageLength, "GET %s %s", pageencoded, HTTPInfo );
        }
        else
        {
            char agent[1000];
#ifdef HTTPInfoCustom
            sprintf_s( agent, 1000, HTTPInfoCustom, customuseragentchunk );
#else
            sprintf_s( agent, 1000, HTTPInfo );
#endif
            sprintf_s( reqbuf, MAX_WebPageLength, "GET %s %s", pageencoded, agent );
        }
        //free( pageencoded );
        int len = (int)strlen( reqbuf );
        int numbytes = (int)send( m_Sock, reqbuf, len, 0 );
        if( numbytes == -1 )
        {
            if( errno == ENOTCONN )
                LOGError( LOGTag, "WebRequestObject::Tick() send errno == ENOTCONN\n" );

            LOGError( LOGTag, "WebRequestObject::Tick() send numbytes = -1, m_Sock = %d, errno = %d, len = %d, req = %s\n",
                m_Sock, errno, len, reqbuf );

            close( m_Sock );
            m_Sock = 0;
            m_SomethingWentWrong = true;
            m_RequestPending = false;
            return; // error
        }
        //int err = shutdown( m_Sock, 1 ); // SD_SEND
        //if( err == -1 )
        //{
        //    close( m_Sock );
        //    m_Sock = -1;
        //    m_SomethingWentWrong = true;
        //    m_RequestPending = false;
        //    return; // error
        //}

        m_RequestSent = true;

        LOGInfo( LOGTag, "WebRequestObject::RequestWebPage() bytes sent=%d %s\n", numbytes, m_PageWanted ); //reqbuf );
    }

    if( m_SocketConnected && m_RequestSent && m_ResponseReady == false )
    {
        int numbytes = (int)recv( m_Sock, &m_pBuffer[m_CharactersReceived], MAX_WebRequestLength-m_CharactersReceived, 0 );

        if( numbytes == -1 )
        {
            int err = WSAGetLastError();
            if( err != WSAEWOULDBLOCK )
            {
                LOGError( LOGTag, "WebRequestObject::Tick() recv err=%d\n", err );
                close( m_Sock );
                m_Sock = 0;
                m_SomethingWentWrong = true;
                m_RequestPending = false;
                return; // error
            }
        }

        if( numbytes > 0 )
        {
            m_CharactersReceived += numbytes;
            m_pBuffer[m_CharactersReceived] = 0;

            LOGInfo( LOGTag, "WebRequestObject recv() bytes=%d\n", numbytes );
            //LOGInfo( LOGTag, "WebRequestObject recv() bytes=%d %s\n", numbytes, m_pBuffer );
        }
        if( numbytes == 0 )
        {
            if( m_CharactersReceived == 0 )
            {
                LOGError( LOGTag, "WebRequestObject::Tick() timeout?\n" );
                close( m_Sock );
                m_Sock = 0;
                m_SomethingWentWrong = true;
                m_RequestPending = false;
                return; // error
            }

            m_ResponseReady = true;
            m_RequestPending = false;

            MyAssert( m_Sock != 0 );
            if( m_Sock != 0 )
                close( m_Sock );
            m_Sock = 0;

            char* statuscode = strstr( m_pBuffer, " " );
            int errorcode = atoi( statuscode );

            if( errorcode == 200 ) // all's good.
            {
                m_pPointerToActualResponseInsideBuffer = 0;
                char* lastCRLF = m_pBuffer;

                while( lastCRLF - m_pBuffer < m_CharactersReceived )
                {
                    char* nextCRLF = strstr( lastCRLF, "\r\n" ) + 2;
                    if( nextCRLF - lastCRLF == 2 )
                    {
                        m_pPointerToActualResponseInsideBuffer = nextCRLF;
                        break;
                    }

                    lastCRLF = nextCRLF;
                }
            }
            else
            {
                LOGError( LOGTag, "WebRequestObject::Tick() errorcode = %d\n", errorcode );
                m_SomethingWentWrong = true;
                m_RequestPending = false;
            }
        }
    }
}

bool WebRequestObject::IsBusy()
{
    //LOGInfo( LOGTag, "WebRequestObject - IsBusy() %p\n", m_pConnection );

    if( m_RequestPending == true && m_ResponseReady == false && m_SomethingWentWrong == false )
        return true;

    return false;
}

void WebRequestObject::ClearResult()
{
    m_pBuffer[0] = 0;
    m_pPointerToActualResponseInsideBuffer = 0;
    m_CharactersReceived = 0;
}

void WebRequestObject::InsertString(const char* string, int len)
{
    if( m_CharactersReceived + len >= MAX_WebRequestLength - 1 )
    {
        MyAssert( false );
        return;
    }

    memcpy( &m_pBuffer[m_CharactersReceived], string, len );
    m_CharactersReceived += len;
    m_pBuffer[m_CharactersReceived] = 0;
}

char* WebRequestObject::GetResult()
{
    return m_pPointerToActualResponseInsideBuffer;
}
