//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/MyFrameworkPCH.h"
#include "NaCLFileObject.h"
#include "MainInstance.h"

NaCLFileObject::NaCLFileObject(pp::Instance* pInstance)
: m_URLRequest(pInstance)
, m_URLLoader(pInstance)
, m_CCFactory(this)
{
}

NaCLFileObject::~NaCLFileObject()
{
}

void NaCLFileObject::GetURL( const char* url )
{
    //LOGInfo( LOGTag, "GetURL %s\n", url );

    m_URLRequest.SetURL( url );
    m_URLRequest.SetMethod( "GET" );
    pp::CompletionCallback cc = m_CCFactory.NewCallback( &NaCLFileObject::OnOpen );
    m_URLLoader.Open( m_URLRequest, cc );
}

// Callback fo the pp::URLLoader::Open().
// Called by pp::URLLoader when response headers are received or when an
// error occurs (in response to the call of pp::URLLoader::Open()).
// Look at <ppapi/c/ppb_url_loader.h> and
// <ppapi/cpp/url_loader.h> for more information about pp::URLLoader.
void NaCLFileObject::OnOpen(int32 result)
{
    //LOGInfo( LOGTag, "OnOpen - result=%d\n", result );

    if( result != PP_OK )
    {
        LOGError( LOGTag, "NaCLFileObject::OnOpen failed: %s\n", m_pFile->GetFullPath() );
        m_pFile->m_FileLoadStatus = FileLoadStatus_Error_FileNotFound;
        return;
    }

    // Here you would process the headers. A real program would want to at least
    // check the HTTP code and potentially cancel the request.
    pp::URLResponseInfo response = m_URLLoader.GetResponseInfo();
    int32 status = response.GetStatusCode();
    //LOGInfo( LOGTag, "OnOpen - status=%d\n", status );

    if( status != 200 )
    {
        LOGError( LOGTag, "NaCLFileObject::OnOpen failed %s: status %d\n", m_pFile->GetFullPath(), status );
        m_pFile->m_FileLoadStatus = FileLoadStatus_Error_FileNotFound;
        return;
    }
    else
    {
        // look for "Content-Length: 1995" in the header
        pp::Var headers_var = response.GetHeaders();
        const char* headers = headers_var.AsString().c_str();
        //LOGInfo( LOGTag, "OnOpen Headers -> %s\n", headers );

        const char* lengthstr = strstr( headers, "Content-Length:" );
        if( lengthstr )
        {
            lengthstr += strlen("Content-Length:");
            //LOGInfo( LOGTag, "Content-Length: found %s\n", lengthstr );
            m_pFile->m_FileLength = atoi( lengthstr );
            //LOGInfo( LOGTag, "Content-Length: found %d\n", m_pFile->m_FileLength );
        }
        else
        {
            LOGError( LOGTag, "NaCLFileObject::OnOpen File Length not reported by web server -> using 10000 will crash if loading file bigger: %s\n", m_pFile->GetFullPath() );
            m_pFile->m_FileLength = 10000;
        }

        //LOGInfo( LOGTag, "OnOpen File Length -> %d\n", m_pFile->m_FileLength );
        // 1 extra character for null terminator for cases where the file buffer is passed as a string, to Lua or glsl parser for example.
        m_pFile->m_pBuffer = MyNew char[m_pFile->m_FileLength+1];
        m_pFile->m_BytesRead = 0;
    }

    // Start streaming.
    ReadBody();
}

// Callback fo the pp::URLLoader::ReadResponseBody().
// |result| contains the number of bytes read or an error code.
// Appends data from this->buffer_ to this->url_response_body_.
void NaCLFileObject::OnRead(int32 result)
{
    //LOGInfo( LOGTag, "OnRead - result=%d\n", result );

    if( result == PP_OK )
    {
        LOGInfo( LOGTag, "NaCLFileObject::OnRead - File Load Complete: %s\n", m_pFile->GetFullPath() );

        // Streaming the file is complete... null terminate the string stored in the file
        m_pFile->m_pBuffer[m_pFile->m_FileLength] = 0;
        m_pFile->m_FileLoadStatus = FileLoadStatus_Success;
    }
    else if( result > 0 )
    {
        // The URLLoader just filled "result" number of bytes into our buffer.
        // Save them and perform another read.
        AppendDataBytes( m_TempReadBuffer, result );
        ReadBody();
    }
    else
    {
        // A read error occurred.
        LOGError( LOGTag, "NaCLFileObject::OnRead failed %s: result %d\n", m_pFile->GetFullPath(), result );
        m_pFile->m_FileLoadStatus = FileLoadStatus_Error_Other;
    }
}

// Reads the response body (asynchronously) into this->buffer_.
// OnRead() will be called when bytes are received or when an error occurs.
void NaCLFileObject::ReadBody()
{
    //LOGInfo( LOGTag, "ReadBody\n" );

    // Note that you specifically want an "optional" callback here. This will
    // allow ReadBody() to return synchronously, ignoring your completion
    // callback, if data is available. For fast connections and large files,
    // reading as fast as we can will make a large performance difference
    // However, in the case of a synchronous return, we need to be sure to run
    // the callback we created since the loader won't do anything with it.
    pp::CompletionCallback cc = m_CCFactory.NewOptionalCallback( &NaCLFileObject::OnRead );
    
    int32 result = 1; // Any number bigger than 0 to enter loop below.
    while( result > 0 )
    {
        result = m_URLLoader.ReadResponseBody( m_TempReadBuffer, sizeof(m_TempReadBuffer), cc );
        //LOGInfo( LOGTag, "ReadResponseBody result=%d\n", result );

        // Handle streaming data directly. Note that we *don't* want to call
        // OnRead here, since in the case of result > 0 it will schedule
        // another call to this function. If the network is very fast, we could
        // end up with a deeply recursive stack.
        if( result > 0 )
        {
            AppendDataBytes( m_TempReadBuffer, result );
        }
    }

    if( result != PP_OK_COMPLETIONPENDING )
    {
        //LOGInfo( LOGTag, "result != PP_OK_COMPLETIONPENDING\n" );

        // Either we reached the end of the stream (result == PP_OK) or there was
        // an error. We want OnRead to get called no matter what to handle
        // that case, whether the error is synchronous or asynchronous. If the
        // result code *is* COMPLETIONPENDING, our callback will be called
        // asynchronously.
        cc.Run( result );
    }
    else
    {
        //LOGInfo( LOGTag, "result = PP_OK_COMPLETIONPENDING\n" );
    }
}

// Append data bytes read from the URL onto the internal buffer.  Does
// nothing if |numbytes| is 0.
void NaCLFileObject::AppendDataBytes(const char* buffer, int32 numbytes)
{
    //LOGInfo( LOGTag, "AppendDataBytes - %d, %s\n", numbytes, buffer );
    //LOGInfo( LOGTag, "AppendDataBytes - %d, total %d\n", numbytes, m_pFile->m_BytesRead + numbytes );

    if( numbytes <= 0 )
        return;

    // Make sure we don't get a buffer overrun.
    if( m_pFile->m_BytesRead + numbytes > m_pFile->m_FileLength )
    {
        LOGError( LOGTag, "NaCLFileObject::AppendDataBytes - m_BytesRead + numbytes > m_FileLength %d + %d > %d: %s\n", m_pFile->m_BytesRead, numbytes, m_pFile->m_FileLength, m_pFile->GetFullPath() );
        //MyAssert( false );
        return;
    }

    memcpy( &m_pFile->m_pBuffer[m_pFile->m_BytesRead], buffer, numbytes );
    m_pFile->m_BytesRead += numbytes;
}

//MyFileObject* RequestFile(const char* filename)
//{
//    return g_ pFileManager->RequestFile( filename );
//}

char* LoadFile(const char* filename, int* length)
{
    return 0;
}

GLuint LoadTexture(const char* filename)
{
    return 0;
}
