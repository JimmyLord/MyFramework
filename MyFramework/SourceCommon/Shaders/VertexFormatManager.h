//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VertexFormatManager_H__
#define __VertexFormatManager_H__

#include "VertexFormats.h"

class VertexFormatManager;

class VertexFormatManager
{
    static const int MAX_DYNAMIC_VERTEX_FORMATS = 10; // TODO: fix this hardcodedness

protected:
    MyList<VertexFormat_Dynamic_Desc*> m_pDynamicVertexFormatDesc;

public:
    VertexFormatManager();
    ~VertexFormatManager();

    VertexFormat_Dynamic_Desc* FindDynamicVertexFormat(int numuvs, bool normals, bool tangents, bool bitangents, bool color, int boneinfluences);
    VertexFormat_Dynamic_Desc* GetDynamicVertexFormat(int numuvs, bool normals, bool tangents, bool bitangents, bool color, int boneinfluences);
};

#endif //__VertexFormatManager_H__
