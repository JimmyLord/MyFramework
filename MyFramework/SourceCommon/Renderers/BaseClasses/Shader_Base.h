//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Shader_Base_H__
#define __Shader_Base_H__

#include "../../Shaders/ShaderManager.h"

class MyLight;
class MaterialDefinition;
class ExposedUniformValue;

class Shader_Base : public BaseShader
{
private:
    static const int MAX_LIGHTS = 4;

protected:

public:
    Shader_Base();
    Shader_Base(ShaderPassTypes type);
    SetClassnameBase( "ShadBase" ); // Only first 8 characters count.
    void Init_Shader_Base();
    virtual ~Shader_Base();

    virtual bool LoadAndCompile(GLuint premadeProgramHandle = 0) = 0;

    virtual void DeactivateShader(BufferDefinition* pVBO = 0, bool useVAOsIfAvailable = true) = 0;

    virtual void InitializeAttributeArrays(VertexFormats vertexFormat, VertexFormat_Dynamic_Desc* pVertexFormatDesc, GLuint vbo, GLuint ibo) = 0;

    virtual bool CompileShader() = 0;
    virtual bool ActivateAndProgramShader(BufferDefinition* pVBO, BufferDefinition* pIBO, MyRE::IndexTypes IBOType, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, MaterialDefinition* pMaterial) = 0;
    virtual bool Activate() = 0;

    virtual void SetupAttributes(BufferDefinition* pVBO, BufferDefinition* pIBO, bool useVAOsIfAvailable) = 0;
    virtual void SetupDefaultAttributes(BufferDefinition* pVBO) = 0;
    virtual void SetDefaultAttribute_Normal(Vector3 value) = 0;
    virtual void ProgramTransforms(MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld) = 0;
    virtual void ProgramMaterialProperties(TextureDefinition* pTexture, ColorByte tint, ColorByte specularColor, float shininess) = 0;
    virtual void ProgramTint(ColorByte tint) = 0;
    virtual void ProgramPointSize(float pointSize) = 0;
    virtual void ProgramUVScaleAndOffset(Vector2 scale, Vector2 offset) = 0;
    virtual void ProgramCamera(Vector3* pCamPos, Vector3* pCamRot) = 0;
    virtual void ProgramLocalSpaceCamera(Vector3* pCamPos, MyMatrix* matInverseWorld) = 0;
    virtual void ProgramLights(MyLight** pLightPtrs, int numLights, MyMatrix* matInverseWorld) = 0;
    virtual void ProgramShadowLightTransform(MyMatrix* matShadowWVP) = 0;
    virtual void ProgramShadowLightTexture(TextureDefinition* pShadowTex) = 0;
    virtual void ProgramLightmap(TextureDefinition* pTexture) = 0;
    virtual void ProgramDepthmap(TextureDefinition* pTexture) = 0;
    virtual void ProgramBoneTransforms(MyMatrix* pTransforms, int numTransforms) = 0;
    virtual void ProgramFramebufferSize(float width, float height) = 0;
    virtual void ProgramExposedUniforms(ExposedUniformValue* valueArray) = 0;

    virtual bool DoVAORequirementsMatch(BaseShader* pShader) = 0;
};

#endif //__Shader_Base_H__
