//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "ShaderManager.h"
#include "VertexFormats.h"

Shader_Base::Shader_Base()
{
    Init_Shader_Base();
}

Shader_Base::Shader_Base(ShaderPassTypes type)
{
    Init_Shader_Base();
    Init( type );
}

void Shader_Base::Init_Shader_Base()
{
    ClassnameSanityCheck();

    m_aHandle_Position = -1;
    m_aHandle_UVCoord = -1;
    m_aHandle_Normal = -1;
    m_aHandle_VertexColor = -1;
    m_aHandle_BoneIndex = -1;
    m_aHandle_BoneWeight = -1;

    m_uHandle_World = -1;
    m_uHandle_ViewProj = -1;
    m_uHandle_WorldViewProj = -1;

    m_uHandle_PointSize = -1;

    m_uHandle_UVTransform = -1;
    m_uHandle_UVScale = -1;
    m_uHandle_UVOffset = -1;

    m_uHandle_ShadowLightWVPT = -1;
    m_uHandle_ShadowTexture = -1;

    m_uHandle_TextureColor = -1;
    m_uHandle_TextureColorWidth = -1;
    m_uHandle_TextureColorHeight = -1;
    m_uHandle_TextureDepth = -1;
    m_uHandle_TextureLightmap = -1;
    m_uHandle_TextureTintColor = -1;
    m_uHandle_TextureSpecColor = -1;
    m_uHandle_Shininess = -1;

    m_uHandle_BoneTransforms = -1;

    m_uHandle_Time = -1;

    m_uHandle_FramebufferSize = -1;

    m_uHandle_WSCameraPos = -1;
    m_uHandle_LSCameraPos = -1;
    m_uHandle_CameraAngle = -1;
    m_uHandle_CameraRotation = -1;
    m_uHandle_InvCameraRotation = -1;

    m_uHandle_AmbientLight = -1;
    m_uHandle_DirLightDir = -1;
    m_uHandle_DirLightColor = -1;

    for( int i=0; i<MAX_LIGHTS; i++ )
    {
        m_uHandle_LightPos[i] = -1;
        m_uHandle_LightDir[i] = -1;
        m_uHandle_LightColor[i] = -1;
        m_uHandle_LightAttenuation[i] = -1;
    }
}

Shader_Base::~Shader_Base()
{
}

bool Shader_Base::LoadAndCompile(GLuint premadeprogramhandle)
{
    // Manually create a shader program here, so we can bind the attribute locations
    GLuint programhandle = premadeprogramhandle;
    if( premadeprogramhandle == 0 )
        programhandle = glCreateProgram();

    // Explicit binding of locations,
    // skipping location 0 for AMD drivers that don't like glVertexAttrib4f() calls on location 0
    glBindAttribLocation( programhandle, 1, "a_Position" );
    glBindAttribLocation( programhandle, 2, "a_UVCoord" );
    glBindAttribLocation( programhandle, 3, "a_Normal" );
    glBindAttribLocation( programhandle, 4, "a_VertexColor" );
    glBindAttribLocation( programhandle, 5, "a_BoneIndex" );
    glBindAttribLocation( programhandle, 6, "a_BoneWeight" );

    if( BaseShader::LoadAndCompile( programhandle ) == false )
        return false;

    m_aHandle_Position =    1; //GetAttributeLocation( m_ProgramHandle, "a_Position" );
    m_aHandle_UVCoord =     2; //GetAttributeLocation( m_ProgramHandle, "a_UVCoord" );
    m_aHandle_Normal =      3; //GetAttributeLocation( m_ProgramHandle, "a_Normal" );
    m_aHandle_VertexColor = 4; //GetAttributeLocation( m_ProgramHandle, "a_VertexColor" );
    m_aHandle_BoneIndex =   5; //GetAttributeLocation( m_ProgramHandle, "a_BoneIndex" );
    m_aHandle_BoneWeight =  6; //GetAttributeLocation( m_ProgramHandle, "a_BoneWeight" );

    m_uHandle_World =         GetUniformLocation( m_ProgramHandle, "u_World" );
    m_uHandle_ViewProj =      GetUniformLocation( m_ProgramHandle, "u_ViewProj" );
    m_uHandle_WorldViewProj = GetUniformLocation( m_ProgramHandle, "u_WorldViewProj" );

    m_uHandle_PointSize =     GetUniformLocation( m_ProgramHandle, "u_PointSize" );

    m_uHandle_UVTransform =   GetUniformLocation( m_ProgramHandle, "u_UVTransform" );
    m_uHandle_UVScale =       GetUniformLocation( m_ProgramHandle, "u_UVScale" );
    m_uHandle_UVOffset =      GetUniformLocation( m_ProgramHandle, "u_UVOffset" );

    m_uHandle_ShadowLightWVPT = GetUniformLocation( m_ProgramHandle, "u_ShadowLightWVPT" );
    m_uHandle_ShadowTexture =   GetUniformLocation( m_ProgramHandle, "u_ShadowTexture" );

    m_uHandle_TextureColor =        GetUniformLocation( m_ProgramHandle, "u_TextureColor" );
    m_uHandle_TextureColorWidth =   GetUniformLocation( m_ProgramHandle, "u_TextureColorWidth" );
    m_uHandle_TextureColorHeight =  GetUniformLocation( m_ProgramHandle, "u_TextureColorHeight" );
    m_uHandle_TextureDepth =        GetUniformLocation( m_ProgramHandle, "u_TextureDepth" );
    m_uHandle_TextureLightmap =     GetUniformLocation( m_ProgramHandle, "u_TextureLightmap" );
    m_uHandle_TextureTintColor =    GetUniformLocation( m_ProgramHandle, "u_TextureTintColor" );
    m_uHandle_TextureSpecColor =    GetUniformLocation( m_ProgramHandle, "u_TextureSpecColor" );
    m_uHandle_Shininess =           GetUniformLocation( m_ProgramHandle, "u_Shininess" );

    m_uHandle_BoneTransforms =      GetUniformLocation( m_ProgramHandle, "u_BoneTransforms" );

    m_uHandle_Time =                GetUniformLocation( m_ProgramHandle, "u_Time" );

    m_uHandle_FramebufferSize =     GetUniformLocation( m_ProgramHandle, "u_FBSize" );

    m_uHandle_WSCameraPos =         GetUniformLocation( m_ProgramHandle, "u_WSCameraPos" );
    m_uHandle_LSCameraPos =         GetUniformLocation( m_ProgramHandle, "u_LSCameraPos" );
    m_uHandle_CameraAngle =         GetUniformLocation( m_ProgramHandle, "u_CameraAngle" );
    m_uHandle_CameraRotation =      GetUniformLocation( m_ProgramHandle, "u_CameraRotation" );
    m_uHandle_InvCameraRotation =   GetUniformLocation( m_ProgramHandle, "u_InvCameraRotation" );

    m_uHandle_AmbientLight =        GetUniformLocation( m_ProgramHandle, "u_AmbientLight" );
    m_uHandle_DirLightDir =         GetUniformLocation( m_ProgramHandle, "u_DirLightDir" );
    m_uHandle_DirLightColor =       GetUniformLocation( m_ProgramHandle, "u_DirLightColor" );

    for( int i=0; i<4; i++ )
    {
        m_uHandle_LightPos[i] =         GetUniformLocation( m_ProgramHandle, "u_LightPos[%d]", i );
        m_uHandle_LightDir[i] =         GetUniformLocation( m_ProgramHandle, "u_LightDir[%d]", i );
        m_uHandle_LightColor[i] =       GetUniformLocation( m_ProgramHandle, "u_LightColor[%d]", i );
        m_uHandle_LightAttenuation[i] = GetUniformLocation( m_ProgramHandle, "u_LightAttenuation[%d]", i );
    }

    for( unsigned int i=0; i<m_pFile->m_NumExposedUniforms; i++ )
    {
        m_uHandle_ExposedUniforms[i] = GetUniformLocation( m_ProgramHandle, m_pFile->m_ExposedUniforms[i].m_Name );
    }

    m_Initialized = true;

    return true;
}

void Shader_Base::DeactivateShader(BufferDefinition* vbo, bool usevaosifavailable)
{
    if( vbo && vbo->m_CurrentVAOHandle )
    {
        glBindVertexArray( 0 );
    }
    else
    {
        BaseShader::DeactivateShader();

        if( m_aHandle_Position != -1 )
            MyDisableVertexAttribArray( m_aHandle_Position );
        if( m_aHandle_UVCoord != -1 )
            MyDisableVertexAttribArray( m_aHandle_UVCoord );
        if( m_aHandle_Normal != -1 )
            MyDisableVertexAttribArray( m_aHandle_Normal );
        if( m_aHandle_VertexColor != -1 )
            MyDisableVertexAttribArray( m_aHandle_VertexColor );
        if( m_aHandle_BoneIndex != -1 )
            MyDisableVertexAttribArray( m_aHandle_BoneIndex );
        if( m_aHandle_BoneWeight != -1 )
            MyDisableVertexAttribArray( m_aHandle_BoneWeight );

        //MyBindBuffer( GL_ARRAY_BUFFER, 0 );
        //MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    }
}

void Shader_Base::InitializeAttributeArrays(VertexFormats vertformat, VertexFormat_Dynamic_Desc* pVertFormatDesc, GLuint vbo, GLuint ibo)
{
#if USE_D3D
    MyAssert( false );
    //UINT stride = sizeof(Vertex_Sprite);
    //UINT offset = 0;
    //g_pD3DContext->IASetVertexBuffers( 0, 1, g_D3DBufferObjects[vbo-1].m_Buffer.GetAddressOf(), &stride, &offset );
    //if( ibo != 0 )
    //    g_pD3DContext->IASetIndexBuffer( g_D3DBufferObjects[ibo-1].m_Buffer.Get(), DXGI_FORMAT_R16_UINT, 0 );

    //g_pD3DContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    //g_pD3DContext->IASetInputLayout( m_pInputLayout.Get() );

    //g_pD3DContext->VSSetConstantBuffers( 0, 1, m_pConstantsBuffer.GetAddressOf() );

    //g_pD3DContext->PSSetShaderResources( 0, 1, g_D3DTextures[texid-1].m_ResourceView.GetAddressOf() );
    //g_pD3DContext->PSSetSamplers( 0, 1, g_pD3DSampleStateLinearWrap.GetAddressOf() );
#else

#if MYFW_IOS || MYFW_OSX || MYFW_NACL
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

    MyBindBuffer( GL_ARRAY_BUFFER, vbo );
    MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

    switch( vertformat )
    {
    case VertexFormat_Sprite:
        {
            InitializeAttributeArray( m_aHandle_Position,    2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Sprite), (void*)offsetof(Vertex_Sprite,x) );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Sprite), (void*)offsetof(Vertex_Sprite,u) );
            DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZ:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZ), (void*)offsetof(Vertex_XYZ,x) );
            DisableAttributeArray( m_aHandle_UVCoord,     Vector3(0,0,0) );
            DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZUV:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV), (void*)offsetof(Vertex_XYZUV,x) );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV), (void*)offsetof(Vertex_XYZUV,u) );
            DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZUV_RGBA:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV_RGBA), (void*)offsetof(Vertex_XYZUV_RGBA,x) );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV_RGBA), (void*)offsetof(Vertex_XYZUV_RGBA,u) );
            DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_XYZUV_RGBA), (void*)offsetof(Vertex_XYZUV_RGBA,r) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZUVNorm:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUVNorm), (void*)offsetof(Vertex_XYZUVNorm,pos) );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUVNorm), (void*)offsetof(Vertex_XYZUVNorm,uv) );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUVNorm), (void*)offsetof(Vertex_XYZUVNorm,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZUVNorm_1Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   1, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  1, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_2Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   2, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_3Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   3, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_4Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   4, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  4, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,weight)    );
        }
        break;

    case VertexFormat_XYZNorm:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZNorm), (void*)offsetof(Vertex_XYZNorm,pos) );
            DisableAttributeArray( m_aHandle_UVCoord,     Vector3(0,0,0) );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZNorm), (void*)offsetof(Vertex_XYZNorm,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_PointSprite:
        {
            InitializeAttributeArray( m_aHandle_Position,    2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,x) );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,u) );
            DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,r) );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
            //MyAssert(false); // I stopped using point sprites, so this doesn't have a handle ATM;
            //InitializeAttributeArray( m_aHandle_Size, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,size) );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,pos)       );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,uv)        );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,normal)    );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,color)     );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_1Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   1, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  1, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_2Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   2, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_3Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   3, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_4Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   4, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  4, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertformat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,weight)    );
        }
        break;

    case VertexFormat_Dynamic:
        {
            int components = pVertFormatDesc->num_position_components;
            InitializeAttributeArray( m_aHandle_Position, components, GL_FLOAT,       GL_FALSE, pVertFormatDesc->stride, (void*)(unsigned long)pVertFormatDesc->offset_pos );
        
            for( int i=0; i<pVertFormatDesc->num_uv_channels; i++ )
            {
                if( pVertFormatDesc->offset_uv[i] )
                    InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT,         GL_FALSE, pVertFormatDesc->stride, (void*)(unsigned long)pVertFormatDesc->offset_uv[i] );
                else
                    DisableAttributeArray( m_aHandle_UVCoord, Vector3(0,0,0) );
            }

            if( pVertFormatDesc->offset_normal )
                InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, pVertFormatDesc->stride, (void*)(unsigned long)pVertFormatDesc->offset_normal );
            else
                DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );

            if( pVertFormatDesc->offset_color )
                InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  pVertFormatDesc->stride, (void*)(unsigned long)pVertFormatDesc->offset_color );
            else
                DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );

            if( pVertFormatDesc->offset_boneindex )
                InitializeAttributeArray( m_aHandle_BoneIndex,   4, GL_UNSIGNED_BYTE, GL_FALSE, pVertFormatDesc->stride, (void*)(unsigned long)pVertFormatDesc->offset_boneindex );
            else
                DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );

            if( pVertFormatDesc->offset_boneweight )
                InitializeAttributeArray( m_aHandle_BoneWeight,  4, GL_FLOAT,         GL_FALSE, pVertFormatDesc->stride, (void*)(unsigned long)pVertFormatDesc->offset_boneweight );
            else
                DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_None:
    case VertexFormat_Invalid:
    default:
        {
            MyAssert( false );
        }
        break;
    }
    // ADDING_NEW_VertexFormat

#if MYFW_IOS || MYFW_OSX
#pragma GCC diagnostic default "-Winvalid-offsetof"
#endif

#endif //USE_D3D
}

bool Shader_Base::DoVAORequirementsMatch(BaseShader* pShader)
{
    //BaseShader::DoVAORequirementsMatch( pShader );

    if( pShader->IsA( "ShadBase" ) )
    {
        Shader_Base* pShader_Base = (Shader_Base*)pShader;

        if( pShader_Base->m_aHandle_Position     == m_aHandle_Position &&
            pShader_Base->m_aHandle_UVCoord      == m_aHandle_UVCoord &&
            pShader_Base->m_aHandle_Normal       == m_aHandle_Normal &&
            pShader_Base->m_aHandle_VertexColor  == m_aHandle_VertexColor &&
            pShader_Base->m_aHandle_BoneIndex    == m_aHandle_BoneIndex &&
            pShader_Base->m_aHandle_BoneWeight   == m_aHandle_BoneWeight )
        {
            return true;
        }
    }

    return false;
}

bool Shader_Base::CompileShader()
{
    if( m_Initialized == false )
    {
        if( LoadAndCompile() == false )
        {
            //LOGInfo( LOGTag, "Shader_Base::ActivateAndProgramShader - shader not ready.\n" );
            return false;
        }
    }

    return true;
}

bool Shader_Base::ActivateAndProgramShader(BufferDefinition* vbo, BufferDefinition* ibo, int ibotype, MyMatrix* viewprojmatrix, MyMatrix* worldmatrix, MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );

    if( CompileShader() == false )
    {
        return false;
    }

    MyUseProgram( m_ProgramHandle );
    //LOGInfo( LOGTag, "glUseProgram %d\n", m_ProgramHandle );
    checkGlError( "glUseProgram" );

    if( vbo )
    {
        SetupAttributes( vbo, ibo, true );

        // TODO: find better way to handle default attributes, MySprite sets this to 0,0,-1
        //       so need to set since VAOs don't change these values
        if( m_aHandle_Normal != -1 )
        {
            glVertexAttrib3f( m_aHandle_Normal, 0, 1, 0 );
        }
    }
    checkGlError( "SetupAttributes" );

    ProgramBaseUniforms( viewprojmatrix, worldmatrix, pMaterial->GetTextureColor(),
        pMaterial->m_ColorDiffuse, pMaterial->m_ColorSpecular, pMaterial->m_Shininess );

    ProgramUVScaleAndOffset( pMaterial->m_UVScale, pMaterial->m_UVOffset );

    ProgramExposedUniforms( pMaterial->m_UniformValues );

    return true;
}

bool Shader_Base::ActivateAndProgramShader()
{
    if( CompileShader() == false )
    {
        return false;
    }

    MyUseProgram( m_ProgramHandle );
    //LOGInfo( LOGTag, "glUseProgram %d\n", m_ProgramHandle );
    checkGlError( "glUseProgram" );

    return true;
}

void Shader_Base::SetupAttributes(BufferDefinition* vbo, BufferDefinition* ibo, bool usevaosifavailable)
{
    if( usevaosifavailable == false || vbo->m_VAOInitialized[vbo->m_CurrentBufferIndex] == false )
    {
        if( glBindVertexArray != 0 )
        {
            if( usevaosifavailable )
            {
                vbo->m_VAOInitialized[vbo->m_CurrentBufferIndex] = true;

                // First time using this VAO, so we create a VAO and set up all the attributes.
                vbo->CreateAndBindVAO();
#if _DEBUG && MYFW_WINDOWS
                vbo->m_DEBUG_VBOUsedOnCreation[vbo->m_CurrentBufferIndex] = vbo->m_CurrentBufferID;
                if( ibo )
                    vbo->m_DEBUG_IBOUsedOnCreation[ibo->m_CurrentBufferIndex] = ibo->m_CurrentBufferID;
#endif
            }
            else
            {
                // Ensure objects rendered without a VAO don't mess with current VAO.
                glBindVertexArray( 0 );
            }
        }

        //MyAssert( vbo->m_VertexFormat == VertexFormat_None || vertformat == vbo->m_VertexFormat );
        MyAssert( vbo->m_VertexFormat != VertexFormat_None );
        InitializeAttributeArrays( vbo->m_VertexFormat, vbo->m_pFormatDesc, vbo?vbo->m_CurrentBufferID:0, ibo?ibo->m_CurrentBufferID:0 );
    }
    else
    {
#if _DEBUG && MYFW_WINDOWS
        MyAssert( vbo->m_DEBUG_VBOUsedOnCreation[vbo->m_CurrentBufferIndex] == vbo->m_CurrentBufferID );
        if( ibo )
            MyAssert( vbo->m_DEBUG_IBOUsedOnCreation[ibo->m_CurrentBufferIndex] == ibo->m_CurrentBufferID );
#endif
        glBindVertexArray( vbo->m_CurrentVAOHandle );
    }
}

void Shader_Base::ProgramBaseUniforms(MyMatrix* viewprojmatrix, MyMatrix* worldmatrix, TextureDefinition* pTexture, ColorByte tint, ColorByte speccolor, float shininess)
{
    checkGlError( "Shader_Base::ProgramBaseUniforms start" );

#if USE_D3D
    MyAssert( 0 );
    //MyMatrix temp = *worldmatrix;
    //temp.Multiply( viewprojmatrix );

    //m_ShaderConstants.mvp = temp;
    //m_ShaderConstants.mvp.Transpose();
    //m_ShaderConstants.tint.Set( tint.r, tint.g, tint.b, tint.a );
    //g_pD3DContext->UpdateSubresource( m_pConstantsBuffer.Get(), 0, NULL, &m_ShaderConstants, 0, 0 );
#else
    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    ProgramPosition( viewprojmatrix, worldmatrix );

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    if( m_uHandle_TextureColor != -1 && pTexture != 0 )
    {
        MyActiveTexture( GL_TEXTURE0 + 0 );
        glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

        glUniform1i( m_uHandle_TextureColor, 0 );

        if( m_uHandle_TextureColorWidth != -1 )
        {
            glUniform1f( m_uHandle_TextureColorWidth, (float)pTexture->m_Width );
        }

        if( m_uHandle_TextureColorHeight != -1 )
        {
            glUniform1f( m_uHandle_TextureColorHeight, (float)pTexture->m_Height );
        }
    }

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    ProgramTint( tint );

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    if( m_uHandle_TextureSpecColor != -1 )
        glUniform4f( m_uHandle_TextureSpecColor, speccolor.r / 255.0f, speccolor.g / 255.0f, speccolor.b / 255.0f, speccolor.a / 255.0f );

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    if( m_uHandle_Shininess != -1 )
        glUniform1f( m_uHandle_Shininess, shininess );

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    if( m_uHandle_Time != -1 )
    {
        float time = (float)MyTime_GetUnpausedTime();
        glUniform1f( m_uHandle_Time, time );
    }

    checkGlError( "Shader_Base::ProgramBaseUniforms end" );
#endif //USE_D3D
}

void Shader_Base::ProgramPosition(MyMatrix* viewprojmatrix, MyMatrix* worldmatrix)
{
    if( m_uHandle_World != -1 )
    {
        if( worldmatrix )
        {
            glUniformMatrix4fv( m_uHandle_World, 1, false, (GLfloat*)&worldmatrix->m11 );
        }
        else
        {
            MyMatrix identity;
            identity.SetIdentity();
            glUniformMatrix4fv( m_uHandle_World, 1, false, (GLfloat*)&identity.m11 );
        }
    }

    if( m_uHandle_ViewProj != -1 )
        glUniformMatrix4fv( m_uHandle_ViewProj, 1, false, (GLfloat*)&viewprojmatrix->m11 );

    if( m_uHandle_WorldViewProj != -1 )
    {
        MyMatrix temp;
        if( worldmatrix )
        {
            temp = *worldmatrix;
            if( viewprojmatrix )
                temp = *viewprojmatrix * temp;
        }
        else
        {
            if( viewprojmatrix )
                temp = *viewprojmatrix;
            else
                temp.SetIdentity();
        }

        glUniformMatrix4fv( m_uHandle_WorldViewProj, 1, false, (GLfloat*)&temp.m11 );
    }
}

void Shader_Base::ProgramTint(ColorByte tint)
{
    if( m_uHandle_TextureTintColor != -1 )
        glUniform4f( m_uHandle_TextureTintColor, tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f, tint.a / 255.0f );
}

void Shader_Base::ProgramPointSize(float pointsize)
{
    if( m_uHandle_PointSize != -1 )
        glUniform1f( m_uHandle_PointSize, pointsize );
}

void Shader_Base::ProgramUVScaleAndOffset(Vector2 scale, Vector2 offset)
{
    if( m_uHandle_UVTransform != -1 )
    {
        float matrix[3*3] = { scale.x, 0, 0, 0, scale.y, 0, offset.x, offset.y, 0 };
        glUniformMatrix3fv( m_uHandle_UVTransform, 1, false, matrix );
    }

    if( m_uHandle_UVScale != -1 )
        glUniform2f( m_uHandle_UVScale, scale.x, scale.y );
    
    if( m_uHandle_UVOffset != -1 )
        glUniform2f( m_uHandle_UVOffset, offset.x, offset.y );
}

void Shader_Base::ProgramCamera(Vector3* campos, Vector3* camrot, MyMatrix* inverseworldmatrix)
{
#if USE_D3D
    MyAssert( 0 );
#else
    if( m_uHandle_WSCameraPos != -1 )
    {
        MyAssert( campos != 0 );
        glUniform3f( m_uHandle_WSCameraPos, campos->x, campos->y, campos->z );
    }

    if( m_uHandle_LSCameraPos != -1 )
    {
        MyAssert( campos != 0 );
        MyAssert( inverseworldmatrix != 0 );

        Vector3 LScampos = *inverseworldmatrix * *campos;
        glUniform3f( m_uHandle_LSCameraPos, LScampos.x, LScampos.y, LScampos.z );
    }

    if( m_uHandle_CameraAngle != -1 )
    {
        MyAssert( camrot != 0 );
        glUniform3f( m_uHandle_CameraAngle, camrot->x * PI/180.0f, camrot->y * PI/180.0f, camrot->z * PI/180.0f );
    }

    if( m_uHandle_CameraRotation != -1 )
    {
        MyMatrix matcamrot;
        matcamrot.CreateRotation( *camrot );
        glUniformMatrix4fv( m_uHandle_CameraRotation, 1, false, (GLfloat*)&matcamrot.m11 );
    }

    if( m_uHandle_InvCameraRotation != -1 )
    {
        MyMatrix matinvcamrot;
        matinvcamrot.CreateRotation( *camrot * -1 );
        glUniformMatrix4fv( m_uHandle_InvCameraRotation, 1, false, (GLfloat*)&matinvcamrot.m11 );
    }
#endif

    return;
}

void Shader_Base::ProgramLights(MyLight** lightptrs, int numlights, MyMatrix* inverseworldmatrix)
{
    if( numlights == 0 )
        return;

    MyAssert( lightptrs );

    if( m_uHandle_AmbientLight != -1 )
    {
        glUniform4f( m_uHandle_AmbientLight, 0.2f, 0.2f, 0.2f, 1.0f );
    }

#if USE_D3D
    MyAssert( 0 );
#else
    int numdirs = 0;
    int numpoints = 0;
    for( int i=0; i<numlights; i++ )
    {
        if( lightptrs[i]->m_LightType == LightType_Directional )
        {
            MyAssert( numdirs <= 1 ); // MAX 1 dir light. TODO: un-hardcode

            if( m_uHandle_DirLightDir != -1 )
            {
                glUniform3f( m_uHandle_DirLightDir, lightptrs[i]->m_SpotDirectionVector.x, lightptrs[i]->m_SpotDirectionVector.y, lightptrs[i]->m_SpotDirectionVector.z );
            }

            if( m_uHandle_DirLightColor != -1 )
            {
                glUniform3f( m_uHandle_DirLightColor, lightptrs[i]->m_Color.r, lightptrs[i]->m_Color.g, lightptrs[i]->m_Color.b ); //, lightptrs[i]->m_Color.a );
            }

            numdirs++;
        }

        if( lightptrs[i]->m_LightType == LightType_Point )
        {
            MyAssert( numpoints <= MAX_LIGHTS );

            if( m_uHandle_LightPos[numpoints] != -1 )
            {
                //MyAssert( inverseworldmatrix != 0 );

                //Vector3 LSlightpos = *inverseworldmatrix * lightptrs[i]->m_Position;
                //glUniform3f( m_uHandle_LightPos[i], LSlightpos.x, LSlightpos.y, LSlightpos.z );

                Vector3 WSlightpos = lightptrs[i]->m_Position;
                glUniform3f( m_uHandle_LightPos[numpoints], WSlightpos.x, WSlightpos.y, WSlightpos.z );
            }

            if( m_uHandle_LightDir[numpoints] != -1 )
            {
                glUniform3f( m_uHandle_LightDir[numpoints], lightptrs[i]->m_SpotDirectionVector.x, lightptrs[i]->m_SpotDirectionVector.y, lightptrs[i]->m_SpotDirectionVector.z );
            }

            if( m_uHandle_LightColor[numpoints] != -1 )
            {
                glUniform3f( m_uHandle_LightColor[numpoints], lightptrs[i]->m_Color.r, lightptrs[i]->m_Color.g, lightptrs[i]->m_Color.b ); //, lightptrs[i]->m_Color.a );
            }

            if( m_uHandle_LightAttenuation[numpoints] != -1 )
            {
                Vector3 atten = lightptrs[numpoints]->m_Attenuation;
                //atten /= inverseworldmatrix->m11;
                glUniform3f( m_uHandle_LightAttenuation[numpoints], atten.x, atten.y, atten.z );
            }

            numpoints++;
        }
    }
#endif

    return;
}

void Shader_Base::ProgramShadowLight(MyMatrix* shadowwvp, TextureDefinition* pShadowTex)
{
    if( m_uHandle_ShadowLightWVPT != -1 )
    {
        glUniformMatrix4fv( m_uHandle_ShadowLightWVPT, 1, false, (GLfloat*)&shadowwvp->m11 );
    }

    if( m_uHandle_ShadowTexture != -1 )
    {
        MyActiveTexture( GL_TEXTURE0 + 1 );
        glBindTexture( GL_TEXTURE_2D, pShadowTex->m_TextureID );

        glUniform1i( m_uHandle_ShadowTexture, 1 );
    }
}

void Shader_Base::ProgramLightmap(TextureDefinition* pTexture)
{
    if( m_uHandle_TextureLightmap != -1 )
    {
        MyActiveTexture( GL_TEXTURE0 + 2 );
        glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

        glUniform1i( m_uHandle_TextureLightmap, 2 );
    }
}

void Shader_Base::ProgramDepthmap(TextureDefinition* pTexture)
{
    if( m_uHandle_TextureDepth != -1 && pTexture != 0 )
    {
        MyActiveTexture( GL_TEXTURE0 + 3 );
        glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

        glUniform1i( m_uHandle_TextureDepth, 3 );
    }
}

void Shader_Base::ProgramBoneTransforms(MyMatrix* transforms, int numtransforms)
{
    // TODO: make this less rigid...
    // Blackberry has limit of 251 uniform vectors, 50*4 + extras is less than that.
    MyAssert( numtransforms <= 50 ); // currently set to 50 bones in Bone_AttribsAndUniforms.glsl

    if( m_uHandle_BoneTransforms != -1 )
        glUniformMatrix4fv( m_uHandle_BoneTransforms, numtransforms, GL_FALSE, &transforms[0].m11 );
}

void Shader_Base::ProgramFramebufferSize(float width, float height)
{
    if( m_uHandle_FramebufferSize != -1 )
        glUniform2f( m_uHandle_FramebufferSize, width, height );    
}

void Shader_Base::ProgramExposedUniforms(ExposedUniformValue* valuearray)
{
    for( unsigned int i=0; i<m_pFile->m_NumExposedUniforms; i++ )
    {
        switch( m_pFile->m_ExposedUniforms[i].m_Type )
        {
        case ExposedUniformType_Float:
            glUniform1f( m_uHandle_ExposedUniforms[i], valuearray[i].m_Float );
            break;

        case ExposedUniformType_Vec2:
            glUniform2f( m_uHandle_ExposedUniforms[i], valuearray[i].m_Vec2[0], valuearray[i].m_Vec2[1] );
            break;

        case ExposedUniformType_Vec3:
            glUniform3f( m_uHandle_ExposedUniforms[i], valuearray[i].m_Vec3[0], valuearray[i].m_Vec3[1], valuearray[i].m_Vec3[2] );
            break;

        case ExposedUniformType_Vec4:
            glUniform4f( m_uHandle_ExposedUniforms[i], valuearray[i].m_Vec4[0], valuearray[i].m_Vec4[1], valuearray[i].m_Vec4[2], valuearray[i].m_Vec4[3] );
            break;

        case ExposedUniformType_ColorByte:
            glUniform4f( m_uHandle_ExposedUniforms[i], valuearray[i].m_ColorByte[0]/255.0f, valuearray[i].m_ColorByte[1]/255.0f, valuearray[i].m_ColorByte[2]/255.0f, valuearray[i].m_ColorByte[3]/255.0f );
            break;

        case ExposedUniformType_Sampler2D:
            break;

        case ExposedUniformType_NotSet:
        default:
            MyAssert( false );
            break;
        }
    }
}
