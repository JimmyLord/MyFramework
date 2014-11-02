//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
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

//#define HTTPInfo "HTTP/1.0\r\nHost: www.flatheadgames.com\r\nUser-Agent: WordGameWebRequest/1.0 (BADA)\r\n\r\n"

WebRequestObject::WebRequestObject()
{
}

WebRequestObject::~WebRequestObject()
{
}

void WebRequestObject::Reset()
{
}

void WebRequestObject::Init(const char* host, int port)
{
}

void WebRequestObject::RequestCloseConnection()
{
}

void WebRequestObject::RequestWebPage(const char* page)
{
    LOGInfo( LOGTag, "WebRequestObject - RequestWebPage\n" );
}

void WebRequestObject::Tick()
{
}

bool WebRequestObject::IsBusy()
{
    return false;
    //LOGInfo( LOGTag, "WebRequestObject - IsBusy() %p\n", m_pConnection );
}

void WebRequestObject::ClearResult()
{
}

void WebRequestObject::InsertString(const char* string, int len)
{
}

char* WebRequestObject::GetResult()
{
    return 0;
}
