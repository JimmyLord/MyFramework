//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Shader_Base_H__
#define __Shader_Base_H__

class MyLight;

class Shader_Base : public BaseShader
{
    static const int MAX_LIGHTS = 4;

public:
    GLint m_aHandle_Position;
    GLint m_aHandle_UVCoord;
    GLint m_aHandle_Normal;
    GLint m_aHandle_VertexColor;
    GLint m_aHandle_BoneIndex;
    GLint m_aHandle_BoneWeight;

    GLint m_uHandle_World;
    GLint m_uHandle_ViewProj;
    GLint m_uHandle_WorldViewProj;

    GLint m_uHandle_PointSize;

    GLint m_uHandle_ShadowLightWVP;
    GLint m_uHandle_ShadowTexture;

    GLint m_uHandle_TextureColor;
    GLint m_uHandle_TextureLightmap;
    GLint m_uHandle_TextureTintColor;
    GLint m_uHandle_TextureSpecColor;
    GLint m_uHandle_Shininess;

    GLint m_uHandle_BoneTransforms;

    GLint m_uTime;

    GLint m_uHandle_CameraPos;

    GLint m_uHandle_LightPos[MAX_LIGHTS];
    GLint m_uHandle_LightDir[MAX_LIGHTS]; // for spotlights
    GLint m_uHandle_LightColor[MAX_LIGHTS];
    GLint m_uHandle_LightAttenuation[MAX_LIGHTS];

protected:
    //void InitializeAttributeArray(GLint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void InitializeAttributeArrays(VertexFormats vertformat, GLuint vbo, GLuint ibo);
    void InitializeVAO(GLuint vao);

    bool DoVAORequirementsMatch(Shader_Base* pShader);

public:
    Shader_Base();
    Shader_Base(ShaderPassTypes type);
    void Init_Shader_Base();
    virtual ~Shader_Base();

    virtual bool LoadAndCompile();

    virtual void DeactivateShader(BufferDefinition* vbo = 0);

    bool ActivateAndProgramShader(VertexFormats vertformat, BufferDefinition* vbo, BufferDefinition* ibo, int ibotype, MyMatrix* viewprojmatrix, MyMatrix* worldmatrix, GLuint texid, ColorByte tint = ColorByte::White(), ColorByte speccolor = ColorByte::White(), float shininess = 0);
    bool ActivateAndProgramShader();

    void SetupAttributes(VertexFormats vertformat, BufferDefinition* vbo, BufferDefinition* ibo, bool usevaosifavailable);
    void ProgramBaseUniforms(MyMatrix* viewprojmatrix, MyMatrix* worldmatrix, GLuint texid, ColorByte tint, ColorByte speccolor, float shininess);
    void ProgramPosition(MyMatrix* viewprojmatrix, MyMatrix* worldmatrix);
    void ProgramTint(ColorByte tint);
    void ProgramPointSize(float pointsize);
    void ProgramCamera(Vector3* campos);
    void ProgramLights(MyLight* lights, int numlights);
    void ProgramShadowLight(MyMatrix* worldmatrix, MyMatrix* shadowviewprojmatrix, GLuint shadowtexid);
    void ProgramLightmap(GLuint texid);
    void ProgramBoneTransforms(MyMatrix* transforms, int numtransforms);
};

#endif //__Shader_Base_H__
