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

#include "CommonHeader.h"

static int WSAGetLastError() { return errno; }

#include "../SourceCommon/Networking/URLEncode.h"
#include "emscripten.h"

#if MYFW_EMSCRIPTEN
#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (EMSCRIPTEN)\r\n\r\n"
#endif

#ifndef HTTPInfoCustom
#define HTTPInfoCustom HTTPInfo
#endif

WebRequestObject::WebRequestObject()
{
    m_HostString[0] = 0;

    m_Initialized = false;
    m_RequestSent = false;
    m_ResponseReady = false;

    m_PageWanted[0] = 0;
    m_RequestPending = false;

    m_SomethingWentWrong = false;
}

WebRequestObject::~WebRequestObject()
{
}

void WebRequestObject::Reset()
{
    m_RequestSent = false;
    m_ResponseReady = false;

    m_PageWanted[0] = 0;
    m_RequestPending = false;

    m_SomethingWentWrong = false;
}

void WebRequestObject::Init(const char* host, unsigned short port)
{
    LOGInfo( LOGTag, "WebRequestObject::Init %s:%d\n", host, port );

    MyAssert( m_Initialized == false );
    if( m_Initialized )
        return;

    MyAssert( m_RequestPending == false );
    if( m_RequestPending )
        return;

    sprintf( m_HostString, "%s:%d", host, port );
    //sprintf( m_HostString, "%s:%d", "127.0.0.1", 5103 );
    //sprintf( m_HostString, "%s:%d", "www.flatheadgames.com", port );
    //sprintf( m_HostString, "%s", host );
    LOGInfo( LOGTag, "WebRequestObject::Init %s\n", m_HostString );

    m_Initialized = true;

    m_pBuffer = MyNew char[MAX_WebPageLength+1];
    m_pBuffer[0] = 0;
}

void WebRequestObject::RequestStart(const char* page)
{
    LOGInfo( LOGTag, "WebRequestObject::RequestStart %s\n", page );

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

    char tempstr[MAX_URLLength];
    sprintf( tempstr, "http://%s%s", m_HostString, m_PageWanted );

    LOGInfo( LOGTag, "WebRequestObject::RequestWebPage %s\n", tempstr );
    emscripten_async_wget_data( tempstr, this, OnLoad, OnError );

    m_RequestPending = true;
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

    char tempstr[MAX_URLLength];
    sprintf( tempstr, "http://%s%s", m_HostString, page );

    LOGInfo( LOGTag, "WebRequestObject::RequestWebPage %s\n", tempstr );
    emscripten_async_wget_data( tempstr, this, OnLoad, OnError );

    m_RequestPending = true;
}

void WebRequestObject::OnLoad(void* webreqobj, void* buffer, int buffersize)
{
    WebRequestObject* pthis = (WebRequestObject*)webreqobj;

    LOGInfo( LOGTag, "WebRequestObject::OnLoad - recv'd %s\n", buffer );
    strcpy_s( pthis->m_pBuffer, MAX_WebPageLength+1, (char*)buffer );

    pthis->m_RequestPending = false;
}

void WebRequestObject::OnError(void* webreqobj)
{
    WebRequestObject* pthis = (WebRequestObject*)webreqobj;

    LOGInfo( LOGTag, "WebRequestObject::OnError\n" );

    pthis->m_SomethingWentWrong = true;
    pthis->m_RequestPending = false;
}

void WebRequestObject::Tick(const char* customuseragentchunk)
{
}

bool WebRequestObject::IsBusy()
{
    if( m_RequestPending == true && m_ResponseReady == false && m_SomethingWentWrong == false )
        return true;

    return false;
}

void WebRequestObject::ClearResult()
{
    m_pBuffer[0] = 0;
}

char* WebRequestObject::GetResult()
{
    if( m_pBuffer[0] == 0 )
        return 0;

    return m_pBuffer;
}
