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

#ifndef __NaCLWebRequest_H__
#define __NaCLWebRequest_H__

#include <string>
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_response_info.h"
#include "ppapi/cpp/instance.h"

#define MAX_URLLength       10000

class WebRequestObject
{
protected:
    char m_HostString[255];
    NaCLFileObject* m_pFileObject;

    bool m_SomethingWentWrong;

    char m_PageWanted[MAX_URLLength+1];

public:
    WebRequestObject();
    ~WebRequestObject();

    void Init(const char* host, int port);
    void Reset();
    void RequestCloseConnection();

    void RequestStart(const char* page);
    void RequestAddPair(const char* var, int value); // will url encode var and value.
    void RequestAddPair(const char* var, const char* value); // will url encode var and value.
    void RequestEnd();

    void RequestWebPage(const char* page, ...);
    void Tick(const char* customuseragentchunk = 0);

    bool IsBusy();
    bool DidSomethingGoWrong() { return m_SomethingWentWrong; }

    void ClearResult();
    char* GetResult();
};

#endif //__NaCLWebRequest_H__
