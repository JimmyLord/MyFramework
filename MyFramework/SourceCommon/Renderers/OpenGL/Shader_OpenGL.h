//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Shader_OpenGL_H__
#define __Shader_OpenGL_H__

#include "../../Shaders/ShaderManager.h"

class MyLight;
class MaterialDefinition;
class ExposedUniformValue;

class Shader_OpenGL : public Shader_Base
{
    static const int MAX_LIGHTS = 4;

public:
    GLint m_aHandle_Position;
    GLint m_aHandle_UVCoord; // TODO: Make this an array.
    GLint m_aHandle_Normal;
    GLint m_aHandle_VertexColor;
    GLint m_aHandle_BoneIndex;
    GLint m_aHandle_BoneWeight;

    GLint m_uHandle_World;
    GLint m_uHandle_WorldView;
    GLint m_uHandle_WorldViewProj;
    GLint m_uHandle_View;
    GLint m_uHandle_ViewProj;
    GLint m_uHandle_Proj;
    GLint m_uHandle_InverseView;
    GLint m_uHandle_InverseProj;

    GLint m_uHandle_PointSize;

    GLint m_uHandle_UVTransform;
    GLint m_uHandle_UVScale;
    GLint m_uHandle_UVOffset;

    GLint m_uHandle_ShadowLightWVPT; // In 0 to 1 space(texture/uv? space), not -1 to 1.
    GLint m_uHandle_ShadowTexture;

    GLint m_uHandle_TextureColor;
    GLint m_uHandle_TextureColorWidth;
    GLint m_uHandle_TextureColorHeight;
    GLint m_uHandle_TextureColorTexelSize;
    GLint m_uHandle_TextureDepth;
    GLint m_uHandle_TextureLightmap;
    GLint m_uHandle_TextureTintColor;
    GLint m_uHandle_TextureSpecColor;
    GLint m_uHandle_Shininess;

    GLint m_uHandle_BoneTransforms;

    GLint m_uHandle_Time;
    
    GLint m_uHandle_FramebufferSize;

    GLint m_uHandle_WSCameraPos;
    GLint m_uHandle_LSCameraPos;
    GLint m_uHandle_CameraAngle;
    GLint m_uHandle_CameraRotation;
    GLint m_uHandle_InvCameraRotation;

    GLint m_uHandle_AmbientLight;
    GLint m_uHandle_DirLightDir;
    GLint m_uHandle_DirLightColor;

    GLint m_uHandle_LightPos[MAX_LIGHTS];
    GLint m_uHandle_LightDir[MAX_LIGHTS]; // For spotlights.
    GLint m_uHandle_LightColor[MAX_LIGHTS];
    GLint m_uHandle_LightAttenuation[MAX_LIGHTS];

    GLint m_uHandle_ExposedUniforms[MyFileObjectShader::MAX_EXPOSED_UNIFORMS];

public:
    Shader_OpenGL();
    Shader_OpenGL(ShaderPassTypes type);
    SetClassnameBase( "ShaderGL" ); // Only first 8 characters count.
    void Init_Shader();
    virtual ~Shader_OpenGL();

    virtual bool LoadAndCompile(GLuint premadeProgramHandle = 0);

    virtual void DeactivateShader(BufferDefinition* pVBO = 0, bool useVAOsIfAvailable = true);

    virtual void InitializeAttributeArrays(VertexFormats vertexFormat, VertexFormat_Dynamic_Desc* pVertexFormatDesc, GLuint vbo, GLuint ibo);

    virtual bool CompileShader();
    virtual bool ActivateAndProgramShader(BufferDefinition* pVBO, BufferDefinition* pIBO, MyRE::IndexTypes IBOType, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, MaterialDefinition* pMaterial);
    virtual bool Activate();

    virtual void SetupAttributes(BufferDefinition* pVBO, BufferDefinition* pIBO, bool useVAOsIfAvailable);
    virtual void SetupDefaultAttributes(BufferDefinition* pVBO);
    virtual void ProgramTransforms(MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld);
    virtual void ProgramMaterialProperties(TextureDefinition* pTexture, ColorByte tint, ColorByte specularColor, float shininess);
    virtual void ProgramTint(ColorByte tint);
    virtual void ProgramPointSize(float pointSize);
    virtual void ProgramUVScaleAndOffset(Vector2 scale, Vector2 offset);
    virtual void ProgramCamera(Vector3* pCamPos, Vector3* pCamRot);
    virtual void ProgramLocalSpaceCamera(Vector3* pCamPos, MyMatrix* matInverseWorld);
    virtual void ProgramLights(MyLight** pLightPtrs, int numLights, MyMatrix* matInverseWorld);
    virtual void ProgramShadowLightTransform(MyMatrix* matShadowWVP);
    virtual void ProgramShadowLightTexture(TextureDefinition* pShadowTex);
    virtual void ProgramLightmap(TextureDefinition* pTexture);
    virtual void ProgramDepthmap(TextureDefinition* pTexture);
    virtual void ProgramBoneTransforms(MyMatrix* pTransforms, int numTransforms);
    virtual void ProgramFramebufferSize(float width, float height);
    virtual void ProgramExposedUniforms(ExposedUniformValue* valueArray);

    virtual bool DoVAORequirementsMatch(BaseShader* pShader);
};

#endif //__Shader_OpenGL_H__
