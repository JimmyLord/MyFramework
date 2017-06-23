//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ShaderManager_H__
#define __ShaderManager_H__

class ShaderManager;
class BufferDefinition;

extern ShaderManager* g_pShaderManager;

class BaseShader : public CPPListNode
{
public:
    bool m_Initialized;
    bool m_ShaderFailedToCompile;

    ShaderPassTypes m_PassType;

    MaterialBlendType m_BlendType;

    const char* m_pFilename;
    MyFileObjectShader* m_pFile; // vertex shader
    MyFileObjectShader* m_pFilePixelShader;

#if USE_D3D
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantsBuffer;
#endif

protected:
    char* m_pVSPredefinitions; // allocated in constructor if needed.
    char* m_pGSPredefinitions; // allocated in constructor if needed.
    char* m_pFSPredefinitions;

public:
    unsigned int m_ProgramHandle;
    unsigned int m_VertexShaderHandle;
    unsigned int m_GeometryShaderHandle;
    unsigned int m_FragmentShaderHandle;

public:
    BaseShader();
    BaseShader(ShaderPassTypes type);
    void Init_BaseShader();
    virtual ~BaseShader();
    SetClassnameBase( "BaseShader" ); // only first 8 character count.

    virtual void Init(ShaderPassTypes type);

    void OverridePredefs(const char* VSpredef, const char* GSpredef, const char* FSpredef, bool alsousedefaults);

    virtual void Invalidate(bool cleanglallocs);
    virtual void CleanGLAllocations();

    virtual void LoadFromFile(const char* filename);
    virtual void LoadFromFile();
    virtual bool LoadAndCompile(GLuint premadeprogramhandle = 0);

    void InitializeAttributeArray(GLint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void InitializeAttributeIArray(GLint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
    void DisableAttributeArray(GLint index, Vector3 value);
    void DisableAttributeArray(GLint index, Vector4 value);

    virtual void DeactivateShader(BufferDefinition* vbo = 0, bool usevaosifavailable = true);

    virtual bool DoVAORequirementsMatch(BaseShader* pShader);
};

class ShaderManager
{
public:
    CPPListHead m_ShaderList;

    void AddShader(BaseShader* pShader);
    void InvalidateAllShaders(bool cleanglallocs);
    void InvalidateAllShadersUsingFile(MyFileObjectShader* pFileToFind);
};

#endif //__ShaderManager_H__
