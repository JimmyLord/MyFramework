//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyFileObjectShader_H__
#define __MyFileObjectShader_H__

class MyFileObjectShader;

struct IncludeFileInfo
{
    MyFileObjectShader* m_pIncludedFile;
    int m_Include_StartIndex;
    int m_Include_EndIndex;

    IncludeFileInfo()
    {
        m_pIncludedFile = 0;
        m_Include_StartIndex = -1;
        m_Include_EndIndex = -1;
    }
};

enum ExposedUniformType
{
    ExposedUniformType_Float,
    ExposedUniformType_Vec2,
    ExposedUniformType_Vec3,
    ExposedUniformType_Vec4,
    ExposedUniformType_Vec4Color,
    ExposedUniformType_Sampler2D,
    ExposedUniformType_NotSet,
};

struct ExposedUniformInfo
{
    ExposedUniformType m_Type;
    char m_Name[32];

    ExposedUniformInfo()
    {
        m_Type = ExposedUniformType_NotSet;
        m_Name[0] = 0;
    }
};

class MyFileObjectShader : public MyFileObject
{
    static const int MAX_INCLUDES = 50;
    static const int MAX_EXPOSED_UNIFORMS = 50;

public:
    bool m_IsAnIncludeFile;

    bool m_ScannedForIncludes;
    bool m_ScannedForExposedUniforms;

    unsigned int m_NumIncludes;
    IncludeFileInfo m_pIncludes[MAX_INCLUDES];

    unsigned int m_NumExposedUniforms;
    ExposedUniformInfo m_pExposedUniforms[MAX_EXPOSED_UNIFORMS];

public:
    MyFileObjectShader();
    virtual ~MyFileObjectShader();
    SetClassnameWithParent( "MyFileShader", MyFileObject ); // only first 8 character count.

    virtual void UnloadContents();

    void ClearIncludedFiles();
    void CheckFileForIncludesAndAddToList();
    bool AreAllIncludesLoaded();

    void ParseAndCleanupExposedUniforms();

    int GetShaderChunkCount();
    int GetShaderChunks(const char** pStrings, int* pLengths);
};


#endif //__MyFileObjectShader_H__
