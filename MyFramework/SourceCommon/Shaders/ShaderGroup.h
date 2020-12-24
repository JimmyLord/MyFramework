//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ShaderGroup_H__
#define __ShaderGroup_H__

#include "../Helpers/RefCount.h"

class BaseShader;
class GameCore;
class MyFileObject;
class MyFileObjectShader;
class Shader_Base;
class ShaderGroup;
class ShaderGroupManager;
class TextureDefinition;

enum ShaderPassTypes
{
    ShaderPass_Main,
    ShaderPass_MainDeferred,
    ShaderPass_MainNoReceiveShadows,
    ShaderPass_ShadowCastRGBA,
    ShaderPass_NumTypes,
};

extern const char* g_ShaderPassDefines[ShaderPass_NumTypes];
extern ShaderPassTypes g_ActiveShaderPass;

typedef Shader_Base* ShaderGroupShaderAllocationFunction(ShaderPassTypes passtype);

class ShaderGroup : public TCPPListNode<ShaderGroup*>, public RefCount
{
public:
    static const int SHADERGROUP_MAX_LIGHTS = 4;
    static const int SHADERGROUP_MAX_BONE_INFLUENCES = 4;

protected:
    GameCore* m_pGameCore;

    MyFileObjectShader* m_pFile;
    BaseShader* m_pShaderPasses[ShaderPass_NumTypes][SHADERGROUP_MAX_LIGHTS+1][SHADERGROUP_MAX_BONE_INFLUENCES+1];

    ShaderGroupShaderAllocationFunction* m_pShaderAllocationFunction;

protected:
    void Initialize(TextureDefinition* pErrorTexture);
    void SetFileForAllPasses(MyFileObject* pFile);

public:
    ShaderGroup(GameCore* pGameCore, TextureDefinition* pErrorTexture);
    ShaderGroup(GameCore* pGameCore, const char* pFilename, TextureDefinition* pErrorTexture);
    ShaderGroup(GameCore* pGameCore, MyFileObject* pFile, TextureDefinition* pErrorTexture);
    ShaderGroup(GameCore* pGameCore, MyFileObject* pFile, ShaderGroupShaderAllocationFunction* pFunc, TextureDefinition* pErrorTexture);
    void Create(GameCore* pGameCore, MyFileObject* pFile, ShaderGroupShaderAllocationFunction* pFunc, TextureDefinition* pErrorTexture);

    ~ShaderGroup();

    const char* GetName();
    MyFileObjectShader* GetFile();

    bool ContainsShader(BaseShader* pShader);

    BaseShader* GlobalPass(int numLights = 0, int numBones = 0);
    BaseShader* GetShader(ShaderPassTypes pass, int numLights = 0, int numBones = 0);

    void OverridePassTypeForAllShaders(ShaderPassTypes originalPassType, ShaderPassTypes newPassType);

    void DisableShadowCasting_AndDoItBadly_WillBeReplaced();

public:
#if MYFW_EDITOR
    enum RightClickOptions
    {
        RightClick_OpenFile = 1000,
        RightClick_UnloadFile,
        RightClick_FindAllReferences,
    };

    void OnPopupClick(ShaderGroup* pShaderGroup, int id);
#endif //MYFW_EDITOR
};

class ShaderGroupManager
{
public:
    TCPPListHead<ShaderGroup*> m_ShaderGroupList;

public:
    void AddShaderGroup(ShaderGroup* pShaderGroup);
    ShaderGroup* FindShaderGroupByName(const char* name);
    ShaderGroup* FindShaderGroupByFile(MyFileObject* pFile);
    ShaderGroup* FindShaderGroupByFilename(const char* fullpath);
};

#endif //__ShaderGroup_H__
