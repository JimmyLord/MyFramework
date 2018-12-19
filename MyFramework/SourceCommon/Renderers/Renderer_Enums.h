//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Renderer_Enums_H__
#define __Renderer_Enums_H__

namespace MyRE // MyRendererEnums
{
    enum PrimitiveTypes
    {
        // These values match OpenGL primitive types (GL_POINTS, GL_LINES, etc). TODO: don't rely on them lining up.
        PrimitiveType_Points,
        PrimitiveType_Lines,
        PrimitiveType_LineLoop,
        PrimitiveType_LineStrip,
        PrimitiveType_Triangles,
        PrimitiveType_TriangleStrip,
        PrimitiveType_TriangleFan,
        PrimitiveType_Undefined,
    };

    enum IndexTypes
    {
        // These values match OpenGL types. TODO: don't rely on them matching.
        IndexType_U8 = GL_UNSIGNED_BYTE,
        IndexType_U16 = GL_UNSIGNED_SHORT,
        IndexType_U32 = GL_UNSIGNED_INT,
        IndexType_Undefined,
    };
} //namespace MyRE

#endif //__Renderer_Enums_H__
