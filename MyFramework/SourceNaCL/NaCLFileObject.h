//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __NaCLFileObject_H__
#define __NaCLFileObject_H__

#include "../SourceCommon/Helpers/FileManager.h"
#include "../SourceCommon/Helpers/MyFileObject.h"

#include <string>
#include "ppapi/utility/completion_callback_factory.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_response_info.h"
#include "ppapi/cpp/instance.h"

class TextureDefinition;

class NaCLFileObject
{
protected:
    char m_TempReadBuffer[4096];

public:
    MyFileObject* m_pFile;

    pp::URLRequestInfo m_URLRequest;
    pp::URLLoader m_URLLoader;
    pp::CompletionCallbackFactory<NaCLFileObject> m_CCFactory;

public:
    NaCLFileObject(pp::Instance* pInstance);
    ~NaCLFileObject();
    
    void GetURL( const char* url );

private:
    void OnOpen(int32 result);
    void OnRead(int32 result);
    void ReadBody();
    void AppendDataBytes(const char* buffer, int32 numbytes);
};

MyFileObject* RequestFile(const char* filename);

char* LoadFile(const char* filename, int* length = 0);
GLuint LoadTexture(const char* filename);

#endif //__NaCLFileObject_H__
