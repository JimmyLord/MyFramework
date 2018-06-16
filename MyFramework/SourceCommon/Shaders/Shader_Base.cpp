//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
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
    m_uHandle_InverseView = -1;
    m_uHandle_InverseProj = -1;

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

bool Shader_Base::LoadAndCompile(GLuint premadeProgramHandle)
{
    // Manually create a shader program here, so we can bind the attribute locations.
    GLuint programHandle = premadeProgramHandle;
    if( premadeProgramHandle == 0 )
        programHandle = glCreateProgram();

#if MYFW_WINDOWS
    // Skipping location 0 for AMD drivers that don't like glVertexAttrib4f() calls on location 0.
    int positionIndex = 1;
#else
    // Sticking with location 0 for everything else.
    // For OSX, nothing renders if positions are at location 1.
    int positionIndex = 0;
#endif

    // Explicit binding of locations.
    glBindAttribLocation( programHandle, positionIndex, "a_Position" );
    glBindAttribLocation( programHandle, 2, "a_UVCoord" );
    glBindAttribLocation( programHandle, 3, "a_Normal" );
    glBindAttribLocation( programHandle, 4, "a_VertexColor" );
    glBindAttribLocation( programHandle, 5, "a_BoneIndex" );
    glBindAttribLocation( programHandle, 6, "a_BoneWeight" );

    if( BaseShader::LoadAndCompile( programHandle ) == false )
        return false;

    m_aHandle_Position =    positionIndex; //GetAttributeLocation( m_ProgramHandle, "a_Position" );
    m_aHandle_UVCoord =     2; //GetAttributeLocation( m_ProgramHandle, "a_UVCoord" );
    m_aHandle_Normal =      3; //GetAttributeLocation( m_ProgramHandle, "a_Normal" );
    m_aHandle_VertexColor = 4; //GetAttributeLocation( m_ProgramHandle, "a_VertexColor" );
    m_aHandle_BoneIndex =   5; //GetAttributeLocation( m_ProgramHandle, "a_BoneIndex" );
    m_aHandle_BoneWeight =  6; //GetAttributeLocation( m_ProgramHandle, "a_BoneWeight" );

    m_uHandle_World =         GetUniformLocation( m_ProgramHandle, "u_World" );
    m_uHandle_ViewProj =      GetUniformLocation( m_ProgramHandle, "u_ViewProj" );
    m_uHandle_WorldViewProj = GetUniformLocation( m_ProgramHandle, "u_WorldViewProj" );
    m_uHandle_InverseView =   GetUniformLocation( m_ProgramHandle, "u_InverseView" );
    m_uHandle_InverseProj =   GetUniformLocation( m_ProgramHandle, "u_InverseProj" );

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

    if( m_pFile )
    {
        for( unsigned int i=0; i<m_pFile->m_NumExposedUniforms; i++ )
        {
            m_uHandle_ExposedUniforms[i] = GetUniformLocation( m_ProgramHandle, m_pFile->m_ExposedUniforms[i].m_Name );
        }
    }

    m_Initialized = true;

    return true;
}

void Shader_Base::DeactivateShader(BufferDefinition* pVBO, bool useVAOsIfAvailable)
{
    if( pVBO && pVBO->m_CurrentVAOHandle )
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

void Shader_Base::InitializeAttributeArrays(VertexFormats vertexFormat, VertexFormat_Dynamic_Desc* pVertexFormatDesc, GLuint vbo, GLuint ibo)
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

#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

    MyBindBuffer( GL_ARRAY_BUFFER, vbo );
    MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

    switch( vertexFormat )
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
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   1, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  1, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_1Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_2Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   2, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_2Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_3Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   3, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_3Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_4Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,normal) );
            DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );
            InitializeAttributeArray( m_aHandle_BoneIndex,   4, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  4, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_4Bones,weight)    );
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
            //MyAssert(false); // I stopped using point sprites, so this doesn't have a handle ATM.
            //InitializeAttributeArray( m_aHandle_Size, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,size) );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,pos)       );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,uv)        );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,normal)    );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA,color)     );
            DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );
            DisableAttributeArray( m_aHandle_BoneWeight,  Vector4(1,0,0,0) );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_1Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   1, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  1, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_1Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_2Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   2, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_2Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_3Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   3, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_3Bones,weight)    );
        }
        break;

    case VertexFormat_XYZUVNorm_RGBA_4Bones:
        {
            InitializeAttributeArray( m_aHandle_Position,    3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,pos)    );
            InitializeAttributeArray( m_aHandle_UVCoord,     2, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,uv)     );
            InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,normal) );
            InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,color)  );
            InitializeAttributeArray( m_aHandle_BoneIndex,   4, GL_UNSIGNED_BYTE, GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,boneindex) );
            InitializeAttributeArray( m_aHandle_BoneWeight,  4, GL_FLOAT,         GL_FALSE, g_VertexFormatSizes[vertexFormat], (void*)offsetof(Vertex_XYZUVNorm_RGBA_4Bones,weight)    );
        }
        break;

    case VertexFormat_Dynamic:
        {
            int components = pVertexFormatDesc->num_position_components;
            InitializeAttributeArray( m_aHandle_Position, components, GL_FLOAT,       GL_FALSE, pVertexFormatDesc->stride, (void*)(unsigned long)pVertexFormatDesc->offset_pos );
        
            for( int i=0; i<pVertexFormatDesc->num_uv_channels; i++ )
            {
                if( pVertexFormatDesc->offset_uv[i] )
                    InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT,         GL_FALSE, pVertexFormatDesc->stride, (void*)(unsigned long)pVertexFormatDesc->offset_uv[i] );
                else
                    DisableAttributeArray( m_aHandle_UVCoord, Vector3(0,0,0) );
            }

            if( pVertexFormatDesc->offset_normal )
                InitializeAttributeArray( m_aHandle_Normal,      3, GL_FLOAT,         GL_FALSE, pVertexFormatDesc->stride, (void*)(unsigned long)pVertexFormatDesc->offset_normal );
            else
                DisableAttributeArray( m_aHandle_Normal,      Vector3(0,1,0) );

            if( pVertexFormatDesc->offset_color )
                InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  pVertexFormatDesc->stride, (void*)(unsigned long)pVertexFormatDesc->offset_color );
            else
                DisableAttributeArray( m_aHandle_VertexColor, Vector4(0,0,0,1) );

            if( pVertexFormatDesc->offset_boneindex )
                InitializeAttributeArray( m_aHandle_BoneIndex,   4, GL_UNSIGNED_BYTE, GL_FALSE, pVertexFormatDesc->stride, (void*)(unsigned long)pVertexFormatDesc->offset_boneindex );
            else
                DisableAttributeArray( m_aHandle_BoneIndex,   Vector4(0,0,0,0) );

            if( pVertexFormatDesc->offset_boneweight )
                InitializeAttributeArray( m_aHandle_BoneWeight,  4, GL_FLOAT,         GL_FALSE, pVertexFormatDesc->stride, (void*)(unsigned long)pVertexFormatDesc->offset_boneweight );
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

#if __GNUC__
#pragma GCC diagnostic pop
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

bool Shader_Base::ActivateAndProgramShader(BufferDefinition* pVBO, BufferDefinition* pIBO, int IBOType, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, MaterialDefinition* pMaterial)
{
    MyAssert( pMaterial );

    if( Activate() == false )
    {
        return false;
    }

    if( pVBO )
    {
        SetupAttributes( pVBO, pIBO, true );

        // TODO: Find better way to handle default attributes, MySprite sets this to 0,0,-1
        //       so need to set since VAOs don't change these values.
        if( m_aHandle_Normal != -1 )
        {
            if( pVBO->m_VertexFormat == VertexFormat_Sprite )
                glVertexAttrib3f( m_aHandle_Normal, 0, 0, -1 );
            else
                glVertexAttrib3f( m_aHandle_Normal, 0, 1, 0 );
        }
    }
    checkGlError( "SetupAttributes" );

    ProgramMaterialProperties( pMaterial->GetTextureColor(), pMaterial->m_ColorDiffuse, pMaterial->m_ColorSpecular, pMaterial->m_Shininess );
    ProgramTransforms( pMatProj, pMatView, pMatWorld );

    ProgramUVScaleAndOffset( pMaterial->m_UVScale, pMaterial->m_UVOffset );

    ProgramExposedUniforms( pMaterial->m_UniformValues );

    return true;
}

bool Shader_Base::Activate()
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

void Shader_Base::SetupAttributes(BufferDefinition* pVBO, BufferDefinition* pIBO, bool useVAOsIfAvailable)
{
    if( useVAOsIfAvailable == false || pVBO->m_VAOInitialized[pVBO->m_CurrentBufferIndex] == false )
    {
        if( glBindVertexArray != 0 )
        {
            if( useVAOsIfAvailable )
            {
                pVBO->m_VAOInitialized[pVBO->m_CurrentBufferIndex] = true;

                // First time using this VAO, so we create a VAO and set up all the attributes.
                pVBO->CreateAndBindVAO();
#if _DEBUG && MYFW_WINDOWS
                pVBO->m_DEBUG_VBOUsedOnCreation[pVBO->m_CurrentBufferIndex] = pVBO->m_CurrentBufferID;
                if( pIBO )
                    pVBO->m_DEBUG_IBOUsedOnCreation[pIBO->m_CurrentBufferIndex] = pIBO->m_CurrentBufferID;
#endif
            }
            else
            {
                // Ensure objects rendered without a VAO don't mess with current VAO.
                glBindVertexArray( 0 );
            }
        }

        //MyAssert( vbo->m_VertexFormat == VertexFormat_None || vertformat == vbo->m_VertexFormat );
        MyAssert( pVBO->m_VertexFormat != VertexFormat_None );
        InitializeAttributeArrays( pVBO->m_VertexFormat, pVBO->m_pFormatDesc, pVBO?pVBO->m_CurrentBufferID:0, pIBO?pIBO->m_CurrentBufferID:0 );
    }
    else
    {
#if _DEBUG && MYFW_WINDOWS
        MyAssert( pVBO->m_DEBUG_VBOUsedOnCreation[pVBO->m_CurrentBufferIndex] == pVBO->m_CurrentBufferID );
        if( pIBO )
            MyAssert( pVBO->m_DEBUG_IBOUsedOnCreation[pIBO->m_CurrentBufferIndex] == pIBO->m_CurrentBufferID );
#endif
        glBindVertexArray( pVBO->m_CurrentVAOHandle );
    }
}

void Shader_Base::ProgramTransforms(MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld)
{
    if( m_uHandle_World != -1 )
    {
        if( pMatWorld )
        {
            glUniformMatrix4fv( m_uHandle_World, 1, false, (GLfloat*)&pMatWorld->m11 );
        }
        else
        {
            MyMatrix identity;
            identity.SetIdentity();
            glUniformMatrix4fv( m_uHandle_World, 1, false, (GLfloat*)&identity.m11 );
        }
    }

    MyMatrix* pMatViewProj = 0;
    MyMatrix temp;

    if( pMatProj && pMatView )
    {
        pMatViewProj = &temp;
        *pMatViewProj = *pMatProj * *pMatView;
    }

    if( m_uHandle_ViewProj != -1 )
    {
        glUniformMatrix4fv( m_uHandle_ViewProj, 1, false, (GLfloat*)&pMatViewProj->m11 );
    }

    if( m_uHandle_WorldViewProj != -1 )
    {
        MyMatrix temp;
        if( pMatWorld )
        {
            temp = *pMatWorld;
            if( pMatViewProj )
                temp = *pMatViewProj * temp;
        }
        else
        {
            if( pMatViewProj )
                temp = *pMatViewProj;
            else
                temp.SetIdentity();
        }

        glUniformMatrix4fv( m_uHandle_WorldViewProj, 1, false, (GLfloat*)&temp.m11 );
    }

    if( m_uHandle_InverseView != -1 && pMatView )
    {
        MyMatrix temp;
        temp = *pMatView;
        temp.Inverse();
        glUniformMatrix4fv( m_uHandle_InverseView, 1, false, (GLfloat*)&temp.m11 );
    }

    if( m_uHandle_InverseProj != -1 && pMatProj )
    {
        MyMatrix temp;
        temp = *pMatProj;
        temp.Inverse();
        glUniformMatrix4fv( m_uHandle_InverseProj, 1, false, (GLfloat*)&temp.m11 );
    }
}

void Shader_Base::ProgramMaterialProperties(TextureDefinition* pTexture, ColorByte tint, ColorByte specularColor, float shininess)
{
    checkGlError( "Shader_Base::ProgramBaseUniforms start" );

#if USE_D3D
    MyAssert( 0 );
    //MyMatrix temp = *pMatWorld;
    //temp.Multiply( matViewProj );

    //m_ShaderConstants.mvp = temp;
    //m_ShaderConstants.mvp.Transpose();
    //m_ShaderConstants.tint.Set( tint.r, tint.g, tint.b, tint.a );
    //g_pD3DContext->UpdateSubresource( m_pConstantsBuffer.Get(), 0, NULL, &m_ShaderConstants, 0, 0 );
#else
    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    if( m_uHandle_TextureColor != -1 )
    {
        // If the shader wants a texture and we didn't pass one in, use a default texture.
        if( pTexture == 0 )
            pTexture = g_pTextureManager->GetErrorTexture();

        MyActiveTexture( GL_TEXTURE0 + 0 );
        glBindTexture( GL_TEXTURE_2D, pTexture->GetTextureID() );

        glUniform1i( m_uHandle_TextureColor, 0 );

        if( m_uHandle_TextureColorWidth != -1 )
        {
            glUniform1f( m_uHandle_TextureColorWidth, (float)pTexture->GetWidth() );
        }

        if( m_uHandle_TextureColorHeight != -1 )
        {
            glUniform1f( m_uHandle_TextureColorHeight, (float)pTexture->GetHeight() );
        }
    }

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    ProgramTint( tint );

    checkGlError( "Shader_Base::ProgramBaseUniforms" );

    if( m_uHandle_TextureSpecColor != -1 )
        glUniform4f( m_uHandle_TextureSpecColor, specularColor.r / 255.0f, specularColor.g / 255.0f, specularColor.b / 255.0f, specularColor.a / 255.0f );

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

void Shader_Base::ProgramTint(ColorByte tint)
{
    if( m_uHandle_TextureTintColor != -1 )
        glUniform4f( m_uHandle_TextureTintColor, tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f, tint.a / 255.0f );
}

void Shader_Base::ProgramPointSize(float pointSize)
{
    if( m_uHandle_PointSize != -1 )
    {
#if !MYFW_OPENGLES2 && !MYFW_OSX
        // Enable GL_PROGRAM_POINT_SIZE to allow vertex shader to set point sizes.
        // TODO: Is this necessary? Works fine without this line on my Intel 4000.
        glEnable( GL_PROGRAM_POINT_SIZE );
#endif

        glUniform1f( m_uHandle_PointSize, pointSize );
    }
    else
    {
#if !MYFW_OPENGLES2 && !MYFW_OSX
        // Disable GL_PROGRAM_POINT_SIZE so point sizes are determined by glPointSize().
        // TODO: Is this necessary? Works fine without this line on my Intel 4000.
        glDisable( GL_PROGRAM_POINT_SIZE );

        glPointSize( pointSize );
#endif
    }
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

void Shader_Base::ProgramCamera(Vector3* pCamPos, Vector3* pCamRot)
{
#if USE_D3D
    MyAssert( 0 );
#else
    if( m_uHandle_WSCameraPos != -1 )
    {
        MyAssert( pCamPos != 0 );
        glUniform3f( m_uHandle_WSCameraPos, pCamPos->x, pCamPos->y, pCamPos->z );
    }

    if( m_uHandle_CameraAngle != -1 )
    {
        MyAssert( pCamRot != 0 );
        glUniform3f( m_uHandle_CameraAngle, pCamRot->x * PI/180.0f, pCamRot->y * PI/180.0f, pCamRot->z * PI/180.0f );
    }

    if( m_uHandle_CameraRotation != -1 )
    {
        MyMatrix matcamrot;
        matcamrot.CreateRotation( *pCamRot );
        glUniformMatrix4fv( m_uHandle_CameraRotation, 1, false, (GLfloat*)&matcamrot.m11 );
    }

    if( m_uHandle_InvCameraRotation != -1 )
    {
        MyMatrix matinvcamrot;
        matinvcamrot.CreateRotation( *pCamRot * -1 );
        glUniformMatrix4fv( m_uHandle_InvCameraRotation, 1, false, (GLfloat*)&matinvcamrot.m11 );
    }
#endif

    return;
}

void Shader_Base::ProgramLocalSpaceCamera(Vector3* pCamPos, MyMatrix* matInverseWorld)
{
    if( m_uHandle_LSCameraPos != -1 )
    {
        MyAssert( pCamPos != 0 );
        MyAssert( matInverseWorld != 0 );

        Vector3 LScampos = *matInverseWorld * *pCamPos;
        glUniform3f( m_uHandle_LSCameraPos, LScampos.x, LScampos.y, LScampos.z );
    }
}

void Shader_Base::ProgramLights(MyLight** pLightPtrs, int numLights, MyMatrix* matInverseWorld)
{
    int numDirs = 0;
    int numPoints = 0;

    if( m_uHandle_AmbientLight != -1 )
    {
        glUniform4f( m_uHandle_AmbientLight, 0.2f, 0.2f, 0.2f, 1.0f );
    }

    if( numLights > 0 )
    {
        MyAssert( pLightPtrs );

#if USE_D3D
        MyAssert( 0 );
#else
        for( int i=0; i<numLights; i++ )
        {
            if( pLightPtrs[i]->m_LightType == LightType_Directional )
            {
                MyAssert( numDirs <= 1 ); // MAX 1 dir light. TODO: un-hardcode

                if( m_uHandle_DirLightDir != -1 )
                {
                    glUniform3f( m_uHandle_DirLightDir, pLightPtrs[i]->m_SpotDirectionVector.x, pLightPtrs[i]->m_SpotDirectionVector.y, pLightPtrs[i]->m_SpotDirectionVector.z );
                }

                if( m_uHandle_DirLightColor != -1 )
                {
                    glUniform3f( m_uHandle_DirLightColor, pLightPtrs[i]->m_Color.r, pLightPtrs[i]->m_Color.g, pLightPtrs[i]->m_Color.b ); //, lightptrs[i]->m_Color.a );
                }

                numDirs++;
            }

            if( pLightPtrs[i]->m_LightType == LightType_Point )
            {
                MyAssert( numPoints <= MAX_LIGHTS );

                if( m_uHandle_LightPos[numPoints] != -1 )
                {
                    //MyAssert( matInverseWorld != 0 );

                    //Vector3 LSlightpos = *matInverseWorld * lightptrs[i]->m_Position;
                    //glUniform3f( m_uHandle_LightPos[i], LSlightpos.x, LSlightpos.y, LSlightpos.z );

                    Vector3 WSlightpos = pLightPtrs[i]->m_Position;
                    glUniform3f( m_uHandle_LightPos[numPoints], WSlightpos.x, WSlightpos.y, WSlightpos.z );
                }

                if( m_uHandle_LightDir[numPoints] != -1 )
                {
                    glUniform3f( m_uHandle_LightDir[numPoints], pLightPtrs[i]->m_SpotDirectionVector.x, pLightPtrs[i]->m_SpotDirectionVector.y, pLightPtrs[i]->m_SpotDirectionVector.z );
                }

                if( m_uHandle_LightColor[numPoints] != -1 )
                {
                    glUniform3f( m_uHandle_LightColor[numPoints], pLightPtrs[i]->m_Color.r, pLightPtrs[i]->m_Color.g, pLightPtrs[i]->m_Color.b ); //, pLightPtrs[i]->m_Color.a );
                }

                if( m_uHandle_LightAttenuation[numPoints] != -1 )
                {
                    Vector3 atten = pLightPtrs[i]->m_Attenuation;
                    glUniform3f( m_uHandle_LightAttenuation[numPoints], atten.x, atten.y, atten.z );
                }

                numPoints++;
            }
        }
    }

    // If there were no directional lights in the scene, then set the uniforms to 0.
    if( numDirs == 0 )
    {
        if( m_uHandle_DirLightDir != -1 )
        {
            glUniform3f( m_uHandle_DirLightDir, 0, 0, 0 );
        }

        if( m_uHandle_DirLightColor != -1 )
        {
            glUniform3f( m_uHandle_DirLightColor, 0, 0, 0 );
        }
    }
#endif

    return;
}

void Shader_Base::ProgramShadowLightTransform(MyMatrix* matShadowWVP)
{
    if( m_uHandle_ShadowLightWVPT != -1 )
    {
        glUniformMatrix4fv( m_uHandle_ShadowLightWVPT, 1, false, (GLfloat*)&matShadowWVP->m11 );
    }
}

void Shader_Base::ProgramShadowLightTexture(TextureDefinition* pShadowTex)
{
    if( m_uHandle_ShadowTexture != -1 )
    {
        if( pShadowTex == 0 )
            pShadowTex = g_pTextureManager->GetErrorTexture();

        MyActiveTexture( GL_TEXTURE0 + 1 );
        glBindTexture( GL_TEXTURE_2D, pShadowTex->GetTextureID() );

        glUniform1i( m_uHandle_ShadowTexture, 1 );
    }
}

void Shader_Base::ProgramLightmap(TextureDefinition* pTexture)
{
    if( m_uHandle_TextureLightmap != -1 )
    {
        if( pTexture == 0 )
            pTexture = g_pTextureManager->GetErrorTexture();

        MyActiveTexture( GL_TEXTURE0 + 2 );
        glBindTexture( GL_TEXTURE_2D, pTexture->GetTextureID() );

        glUniform1i( m_uHandle_TextureLightmap, 2 );
    }
}

void Shader_Base::ProgramDepthmap(TextureDefinition* pTexture)
{
    if( m_uHandle_TextureDepth != -1 && pTexture != 0 )
    {
        if( pTexture == 0 )
            pTexture = g_pTextureManager->GetErrorTexture();

        MyActiveTexture( GL_TEXTURE0 + 3 );
        glBindTexture( GL_TEXTURE_2D, pTexture->GetTextureID() );

        glUniform1i( m_uHandle_TextureDepth, 3 );
    }
}

void Shader_Base::ProgramBoneTransforms(MyMatrix* pTransforms, int numTransforms)
{
    // TODO: make this less rigid...
    // Blackberry has limit of 251 uniform vectors, 50*4 + extras is less than that.
    MyAssert( numTransforms <= 50 ); // Currently set to 50 bones in Bone_AttribsAndUniforms.glsl.

    if( m_uHandle_BoneTransforms != -1 )
        glUniformMatrix4fv( m_uHandle_BoneTransforms, numTransforms, GL_FALSE, &pTransforms[0].m11 );
}

void Shader_Base::ProgramFramebufferSize(float width, float height)
{
    if( m_uHandle_FramebufferSize != -1 )
        glUniform2f( m_uHandle_FramebufferSize, width, height );    
}

void Shader_Base::ProgramExposedUniforms(ExposedUniformValue* valueArray)
{
    int numTexturesSet = 0;

    if( m_pFile == 0 )
        return;

    for( unsigned int i=0; i<m_pFile->m_NumExposedUniforms; i++ )
    {
        switch( m_pFile->m_ExposedUniforms[i].m_Type )
        {
        case ExposedUniformType_Float:
            glUniform1f( m_uHandle_ExposedUniforms[i], valueArray[i].m_Float );
            break;

        case ExposedUniformType_Vec2:
            glUniform2f( m_uHandle_ExposedUniforms[i], valueArray[i].m_Vec2[0], valueArray[i].m_Vec2[1] );
            break;

        case ExposedUniformType_Vec3:
            glUniform3f( m_uHandle_ExposedUniforms[i], valueArray[i].m_Vec3[0], valueArray[i].m_Vec3[1], valueArray[i].m_Vec3[2] );
            break;

        case ExposedUniformType_Vec4:
            glUniform4f( m_uHandle_ExposedUniforms[i], valueArray[i].m_Vec4[0], valueArray[i].m_Vec4[1], valueArray[i].m_Vec4[2], valueArray[i].m_Vec4[3] );
            break;

        case ExposedUniformType_ColorByte:
            glUniform4f( m_uHandle_ExposedUniforms[i], valueArray[i].m_ColorByte[0]/255.0f, valueArray[i].m_ColorByte[1]/255.0f, valueArray[i].m_ColorByte[2]/255.0f, valueArray[i].m_ColorByte[3]/255.0f );
            break;

        case ExposedUniformType_Sampler2D:
            {
                TextureDefinition* pTexture = valueArray[i].m_pTexture;
                if( pTexture == 0 )
                    pTexture = g_pTextureManager->GetErrorTexture();

                if( pTexture )
                {
                    MyActiveTexture( GL_TEXTURE0 + 4 + numTexturesSet );
                    glBindTexture( GL_TEXTURE_2D, pTexture->GetTextureID() );
                    glUniform1i( m_uHandle_ExposedUniforms[i], 4 + numTexturesSet );
                    numTexturesSet++;
                }
            }
            break;

        case ExposedUniformType_NotSet:
        default:
            MyAssert( false );
            break;
        }
    }
}
