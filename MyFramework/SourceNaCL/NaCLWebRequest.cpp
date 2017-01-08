//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MainInstance.h"
#include "NaCLFileObject.h"

#include "../SourceCommon/Networking/URLEncode.h"

WebRequestObject::WebRequestObject()
{
    m_pFileObject = 0;
    m_HostString[0] = 0;
}

WebRequestObject::~WebRequestObject()
{
    SAFE_DELETE( m_pFileObject );
}

void WebRequestObject::Init(const char* host, int port)
{
    LOGInfo( LOGTag, "WebRequestObject::Init %s:%d\n", host, port );
    sprintf( m_HostString, "%s:%d", host, port );
    LOGInfo( LOGTag, "WebRequestObject::Init %s\n", m_HostString );
    m_SomethingWentWrong = false;
}

void WebRequestObject::Reset()
{
    LOGInfo( LOGTag, "WebRequestObject::Reset\n" );

    m_SomethingWentWrong = false;
    SAFE_DELETE( m_pFileObject );
}

void WebRequestObject::RequestCloseConnection()
{
    LOGInfo( LOGTag, "WebRequestObject::RequestCloseConnection\n" );

    SAFE_DELETE( m_pFileObject );
}

void WebRequestObject::RequestStart(const char* page)
{
    MyAssert( IsBusy() == false );
    if( IsBusy() )
        return;

    ClearResult();
    Reset();

    sprintf_s( m_PageWanted, MAX_URLLength, "%s", page );
    strcat_s( m_PageWanted, MAX_URLLength, "?" );

    //LOGInfo( LOGTag, "WebRequestObject - RequestStart %s\n", m_PageWanted );
}

// will url encode var and value.
void WebRequestObject::RequestAddPair(const char* var, int value)
{
    MyAssert( IsBusy() == false );
    if( IsBusy() )
        return;

    char valuestring[20];
    sprintf_s( valuestring, 20, "%d", value );

    RequestAddPair( var, valuestring );

    //LOGInfo( LOGTag, "WebRequestObject - RequestAddPair %s\n", m_PageWanted );
}

// will url encode var and value.
void WebRequestObject::RequestAddPair(const char* var, const char* value)
{
    MyAssert( IsBusy() == false );
    if( IsBusy() )
        return;

    char* varencoded = url_encode( var );
    char* valueencoded = url_encode( value );

    strcat_s( m_PageWanted, MAX_URLLength, varencoded );
    strcat_s( m_PageWanted, MAX_URLLength, "=" );
    strcat_s( m_PageWanted, MAX_URLLength, valueencoded );
    strcat_s( m_PageWanted, MAX_URLLength, "&" );

    free( varencoded );
    free( valueencoded );

    //LOGInfo( LOGTag, "WebRequestObject - RequestAddPair %s\n", m_PageWanted );
}

void WebRequestObject::RequestEnd()
{
    LOGInfo( LOGTag, "WebRequestObject - RequestWebPage\n" );

    MyAssert( IsBusy() == false );
    if( IsBusy() )
        return;

    m_PageWanted[ strlen(m_PageWanted)-1 ] = 0; // remove the final ? or &

    char tempstr[MAX_URLLength];
    sprintf( tempstr, "http://%s%s", m_HostString, m_PageWanted );

    //LOGInfo( LOGTag, "WebRequestObject - RequestEnd host - %s\n", m_HostString );
    //LOGInfo( LOGTag, "WebRequestObject - RequestEnd page - %s\n", m_PageWanted );
    //LOGInfo( LOGTag, "WebRequestObject - RequestEnd temp - %s\n", tempstr );

    NaCLFileObject* naclfile = MyNew NaCLFileObject( g_pInstance );

    m_pFileObject = MyNew MyFileObject();
    m_pFileObject->m_pNaClFileObject = naclfile;
    naclfile->m_pFile = m_pFileObject;

    naclfile->GetURL( m_PageWanted );
    //m_pFileObject->GetURL( tempstr );
    //m_pFileObject->GetURL( page );
}

void WebRequestObject::RequestWebPage(const char* page, ...)
{
    LOGInfo( LOGTag, "WebRequestObject::RequestWebPage %s\n", page );

    va_list arg;
    va_start( arg, page );
    vsnprintf( m_PageWanted, sizeof(m_PageWanted), page, arg );
    va_end( arg );
    m_PageWanted[MAX_URLLength+1-1] = 0; // vsnprintf_s might do this, but docs are unclear

    char tempstr[MAX_URLLength];
    sprintf( tempstr, "http://%s%s", m_HostString, m_PageWanted );

    LOGInfo( LOGTag, "WebRequestObject::RequestWebPage %s\n", tempstr );

    NaCLFileObject* naclfile = MyNew NaCLFileObject( g_pInstance );

    m_pFileObject = MyNew MyFileObject();
    m_pFileObject->m_pNaClFileObject = naclfile;
    naclfile->m_pFile = m_pFileObject;

    naclfile->GetURL( m_PageWanted );
    //m_pFileObject->GetURL( tempstr );
    //m_pFileObject->GetURL( page );
}

void WebRequestObject::Tick(const char* customuseragentchunk)
{
}

bool WebRequestObject::IsBusy()
{
    if( m_pFileObject && m_pFileObject->m_FileLoadStatus != FileLoadStatus_Success )
        return true;

    return false;
}

void WebRequestObject::ClearResult()
{
    LOGInfo( LOGTag, "WebRequestObject::ClearResult\n" );

    SAFE_DELETE( m_pFileObject );
}

char* WebRequestObject::GetResult()
{
    //LOGInfo( LOGTag, "WebRequestObject::GetResult\n" );

    if( m_pFileObject && m_pFileObject->m_FileLoadStatus == FileLoadStatus_Success )
        return m_pFileObject->m_pBuffer;

    return 0;
}

