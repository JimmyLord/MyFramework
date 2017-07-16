//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ReallyShittyCode_H__
#define __ReallyShittyCode_H__

#if !MYFW_PPAPI
#define strcpy_s(a,b,c)             strcpy(a,c)
#define sprintf_s                   snprintf
#define snprintf_s                  snprintf
#define strcat_s(a,b,c)             strcat(a,c)
#define strncpy_s(a,b,c,d)          strncpy(a,c,d)
#define sscanf_s                    sscanf
#define vsnprintf_s(a,b,c,d,e)      vsnprintf(a,b,d,e)
#define _stricmp                    strcasecmp
//#define _TRUNCATE
#endif //!MYFW_PPAPI

//#define glBindFramebufferEXT            glBindFramebuffer
//#define glBindRenderbufferEXT           glBindRenderbuffer
//#define glRenderbufferStorageEXT        glRenderbufferStorage
//#define glFramebufferTexture2DEXT       glFramebufferTexture2D
//#define glFramebufferRenderbufferEXT    glFramebufferRenderbuffer
//#define glCheckFramebufferStatusEXT     glCheckFramebufferStatus
//#define glGenFramebuffersEXT            glGenFramebuffers
//#define glGenRenderbuffersEXT           glGenRenderbuffers
//#define glDeleteFramebuffersEXT         glDeleteFramebuffers

#endif