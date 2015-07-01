//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ShaderGroup_H__
#define __ShaderGroup_H__

class ShaderGroup;
class Shader_Base;
class BaseShader;
class ShaderGroupManager;

enum ShaderPassTypes
{
    ShaderPass_Main,
    ShaderPass_MainNoReceiveShadows,
    ShaderPass_ShadowCastRGBA,
    ShaderPass_NumTypes,
};

extern ShaderGroupManager* g_pShaderGroupManager;
extern const char* g_ShaderPassDefines[ShaderPass_NumTypes];
extern ShaderPassTypes g_ActiveShaderPass;

typedef Shader_Base* (*ShaderGroupShaderAllocationFunction)(ShaderPassTypes passtype);

class ShaderGroup : public CPPListNode, public RefCount
{
    static const int SHADERGROUP_MAX_LIGHTS = 4;
    static const int SHADERGROUP_MAX_BONE_INFLUENCES = 4;

protected:
    MyFileObjectShader* m_pFile;
    BaseShader* m_pShaderPasses[ShaderPass_NumTypes][SHADERGROUP_MAX_LIGHTS+1][SHADERGROUP_MAX_BONE_INFLUENCES+1];

    ShaderGroupShaderAllocationFunction m_pShaderAllocationFunction;

protected:
    void Initialize();
    void SetFileForAllPasses(MyFileObject* pFile);

public:
    ShaderGroup(MyFileObject* pFile);
    ShaderGroup(MyFileObject* pFile, ShaderGroupShaderAllocationFunction pFunc);
    void Create(MyFileObject* pFile, ShaderGroupShaderAllocationFunction pFunc);

    ~ShaderGroup();

    const char* GetName() { return m_pFile->m_FilenameWithoutExtension; }
    MyFileObjectShader* GetFile() { return m_pFile; }

    BaseShader* GlobalPass(int numlights = 0, int numbones = 0);
    BaseShader* GetShader(ShaderPassTypes pass, int numlights = 0, int numbones = 0);

    void OverridePassTypeForAllShaders(ShaderPassTypes originalpasstype, ShaderPassTypes newpasstype);

    void DisableShadowCasting_AndDoItBadly_WillBeReplaced();

public:
#if MYFW_USING_WX
    static void StaticOnDrag(void* pObjectPtr) { ((ShaderGroup*)pObjectPtr)->OnDrag(); }
    void OnDrag();
#endif //MYFW_USING_WX
};

class ShaderGroupManager
{
public:
    CPPListHead m_ShaderGroupList;

public:
    void AddShaderGroup(ShaderGroup* pShaderGroup);
    ShaderGroup* FindShaderGroupByName(const char* name);
    ShaderGroup* FindShaderGroupByFile(MyFileObject* pFile);
    ShaderGroup* FindShaderGroupByFilename(const char* fullpath);
};

#endif //__ShaderGroup_H__
