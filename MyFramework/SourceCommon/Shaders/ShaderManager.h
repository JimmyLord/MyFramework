//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ShaderManager_H__
#define __ShaderManager_H__

#include "ShaderGroup.h"
#include "../Renderers/BaseClasses/Renderer_Enums.h"

class BufferDefinition;
class GameCore;
class ShaderManager;
class TextureDefinition;

class BaseShader : public TCPPListNode<BaseShader*>
{
public:
    enum Attributes
    {
        Attribute_Position,
        Attribute_UVCoord,
        Attribute_Normal,
        Attribute_VertexColor,
        Attribute_BoneIndex,
        Attribute_BoneWeight,
    };

    GameCore* m_pGameCore;
    TextureDefinition* m_pErrorTexture;

    bool m_Initialized;
    bool m_ShaderFailedToCompile;

    ShaderPassTypes m_PassType;

    MyRE::MaterialBlendTypes m_BlendType;
    MyRE::BlendFactors m_BlendFactorSrc;
    MyRE::BlendFactors m_BlendFactorDest;

    bool m_Emissive;

    const char* m_pFilename;
    MyFileObjectShader* m_pFile; // Vertex shader.
    MyFileObjectShader* m_pFilePixelShader;

#if USE_D3D
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantsBuffer;
#endif

protected:
    char* m_pVSPredefinitions; // Allocated in constructor if needed.
    char* m_pGSPredefinitions; // Allocated in constructor if needed.
    char* m_pFSPredefinitions;

public:
    unsigned int m_ProgramHandle;
    unsigned int m_VertexShaderHandle;
    unsigned int m_GeometryShaderHandle;
    unsigned int m_FragmentShaderHandle;

public:
    BaseShader(GameCore* pGameCore);
    BaseShader(GameCore* pGameCore, ShaderPassTypes type);
    void Init_BaseShader(GameCore* pGameCore);
    virtual ~BaseShader();
    SetClassnameBase( "BaseShader" ); // Only first 8 character count.

    virtual void Init(ShaderPassTypes type);

    void SetErrorTexture(TextureDefinition* pErrorTexture) { m_pErrorTexture = pErrorTexture; }

    void OverridePredefs(const char* VSpredef, const char* GSpredef, const char* FSpredef, bool alsoUseDefaults);

    virtual void Invalidate(bool cleanGLAllocs);
    virtual void CleanGLAllocations();

    virtual void CreateProgram(int VSPreLen, const char* pVSPre, int GSPreLen, const char* pGSPre, int FSPreLen, const char* pFSPre, int numChunks, const char** ppStrings, int* pLengths, GLuint premadeProgramHandle) = 0;

    virtual void LoadFromFile(const char* filename);
    virtual void LoadFromFile();
    virtual bool LoadAndCompile(GLuint premadeProgramHandle = 0);

    virtual int GetAttributeIndex(Attributes attribute) = 0;
    virtual void InitializeAttributeArray(Attributes attribute, uint32 size, MyRE::AttributeTypes type, bool normalized, uint32 stride, const void* pointer) = 0;
    virtual void InitializeAttributeArray(GLint index, uint32 size, MyRE::AttributeTypes type, bool normalized, uint32 stride, const void* pointer) = 0;
    virtual void InitializeAttributeIArray(GLint index, uint32 size, MyRE::AttributeTypes type, uint32 stride, const void* pointer) = 0;
    virtual void DisableAttributeArray(GLint index, Vector3 value) = 0;
    virtual void DisableAttributeArray(GLint index, Vector4 value) = 0;

    virtual void DeactivateShader(BufferDefinition* vbo = nullptr, bool useVAOsIfAvailable = true);

    virtual bool DoVAORequirementsMatch(BaseShader* pShader);

    MyRE::BlendFactors GetShaderBlendFactorSrc();
    MyRE::BlendFactors GetShaderBlendFactorDest();
};

class ShaderManager
{
protected:
    GameCore* m_pGameCore;
    TCPPListHead<BaseShader*> m_ShaderList;

public:
    ShaderManager(GameCore* pGameCore);

    void AddShader(BaseShader* pShader);
    void InvalidateAllShaders(bool cleanGLAllocs);
    void InvalidateAllShadersUsingFile(MyFileObjectShader* pFileToFind);

    // Getters.
    GameCore* GetGameCore() { return m_pGameCore; }
};

#endif //__ShaderManager_H__
