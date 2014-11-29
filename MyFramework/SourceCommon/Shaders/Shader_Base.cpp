//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
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
    m_aHandle_Position = -1;
    m_aHandle_UVCoord = -1;
    m_aHandle_Normal = -1;
    m_aHandle_VertexColor = -1;

    m_uHandle_World = -1;
    m_uHandle_WorldViewProj = -1;

    m_uHandle_ShadowLightWVP = -1;
    m_uHandle_ShadowTexture = -1;

    m_uHandle_TextureColor = -1;
    m_uHandle_TextureLightmap = -1;
    m_uHandle_TextureTintColor = -1;
    m_uHandle_TextureSpecColor = -1;
    m_uHandle_Shininess = -1;

    m_uHandle_CameraPos = -1;

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

bool Shader_Base::LoadAndCompile()
{
    if( BaseShader::LoadAndCompile() == false )
        return false;

    m_aHandle_Position =    GetAttributeLocation( m_ProgramHandle, "a_Position" );
    m_aHandle_UVCoord =     GetAttributeLocation( m_ProgramHandle, "a_UVCoord" );
    m_aHandle_Normal =      GetAttributeLocation( m_ProgramHandle, "a_Normal" );
    m_aHandle_VertexColor = GetAttributeLocation( m_ProgramHandle, "a_VertexColor" );

    m_uHandle_World =         GetUniformLocation( m_ProgramHandle, "u_World" );
    m_uHandle_WorldViewProj = GetUniformLocation( m_ProgramHandle, "u_WorldViewProj" );

    m_uHandle_ShadowLightWVP = GetUniformLocation( m_ProgramHandle, "u_ShadowLightMVP" );
    m_uHandle_ShadowTexture =  GetUniformLocation( m_ProgramHandle, "u_ShadowTexture" );

    m_uHandle_TextureColor =     GetUniformLocation( m_ProgramHandle, "u_TextureColor" );
    m_uHandle_TextureLightmap =  GetUniformLocation( m_ProgramHandle, "u_TextureLightmap" );
    m_uHandle_TextureTintColor = GetUniformLocation( m_ProgramHandle, "u_TextureTintColor" );
    m_uHandle_TextureSpecColor = GetUniformLocation( m_ProgramHandle, "u_TextureSpecColor" );
    m_uHandle_Shininess =        GetUniformLocation( m_ProgramHandle, "u_Shininess" );

    m_uHandle_CameraPos =     GetUniformLocation( m_ProgramHandle, "u_CameraPos" );

    for( int i=0; i<4; i++ )
    {
        m_uHandle_LightPos[i] =         GetUniformLocation( m_ProgramHandle, "u_LightPos[%d]", i );
        m_uHandle_LightDir[i] =         GetUniformLocation( m_ProgramHandle, "u_LightDir[%d]", i );
        m_uHandle_LightColor[i] =       GetUniformLocation( m_ProgramHandle, "u_LightColor[%d]", i );
        m_uHandle_LightAttenuation[i] = GetUniformLocation( m_ProgramHandle, "u_LightAttenuation[%d]", i );
    }

    m_Initialized = true;

    return true;
}

void Shader_Base::DeactivateShader(BufferDefinition* vbo)
{
    if( vbo && vbo->m_CurrentVAOHandle[g_ActiveShaderPass] )
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

        //MyBindBuffer( GL_ARRAY_BUFFER, 0 );
        //MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    }
}

void Shader_Base::InitializeAttributeArrays(VertexFormats vertformat, GLuint vbo, GLuint ibo, int ibotype)
{
#if USE_D3D
    assert( false );
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
    MyBindBuffer( GL_ARRAY_BUFFER, vbo );
    MyBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

    if( vertformat == VertexFormat_Sprite )
    {
        InitializeAttributeArray( m_aHandle_Position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Sprite), (void*)offsetof(Vertex_Sprite,x) );
        InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Sprite), (void*)offsetof(Vertex_Sprite,u) );
        DisableAttributeArray( m_aHandle_Normal );
        DisableAttributeArray( m_aHandle_VertexColor );
    }
    else if( vertformat == VertexFormat_XYZ )
    {
        InitializeAttributeArray( m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZ), (void*)offsetof(Vertex_XYZ,x) );
        DisableAttributeArray( m_aHandle_UVCoord );
        DisableAttributeArray( m_aHandle_Normal );
        DisableAttributeArray( m_aHandle_VertexColor );
    }
    else if( vertformat == VertexFormat_XYZUV )
    {
        InitializeAttributeArray( m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV), (void*)offsetof(Vertex_XYZUV,x) );
        InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV), (void*)offsetof(Vertex_XYZUV,u) );
        DisableAttributeArray( m_aHandle_Normal );
        DisableAttributeArray( m_aHandle_VertexColor );
    }
    else if( vertformat == VertexFormat_XYZUV_RGBA )
    {
        InitializeAttributeArray( m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV_RGBA), (void*)offsetof(Vertex_XYZUV_RGBA,x) );
        InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUV_RGBA), (void*)offsetof(Vertex_XYZUV_RGBA,u) );
        DisableAttributeArray( m_aHandle_Normal );
        InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_XYZUV_RGBA), (void*)offsetof(Vertex_XYZUV_RGBA,r) );
    }
    else if( vertformat == VertexFormat_XYZUVNorm )
    {
        InitializeAttributeArray( m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUVNorm), (void*)offsetof(Vertex_XYZUVNorm,pos) );
        InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUVNorm), (void*)offsetof(Vertex_XYZUVNorm,uv) );
        InitializeAttributeArray( m_aHandle_Normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZUVNorm), (void*)offsetof(Vertex_XYZUVNorm,normal) );
        DisableAttributeArray( m_aHandle_VertexColor );
    }
    else if( vertformat == VertexFormat_XYZNorm )
    {
        InitializeAttributeArray( m_aHandle_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZNorm), (void*)offsetof(Vertex_XYZNorm,pos) );
        DisableAttributeArray( m_aHandle_UVCoord );
        InitializeAttributeArray( m_aHandle_Normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XYZNorm), (void*)offsetof(Vertex_XYZNorm,normal) );
        DisableAttributeArray( m_aHandle_VertexColor );
    }
    else if( vertformat == VertexFormat_PointSprite )
    {
        InitializeAttributeArray( m_aHandle_Position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,x) );
        InitializeAttributeArray( m_aHandle_UVCoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,u) );
        DisableAttributeArray( m_aHandle_Normal );
        InitializeAttributeArray( m_aHandle_VertexColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,r) );
        //assert(false); // I stopped using point sprites, so this doesn't have a handle ATM;
        //InitializeAttributeArray( m_aHandle_Size, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_PointSprite), (void*)offsetof(Vertex_PointSprite,size) );
    }
#endif //USE_D3D
}

bool Shader_Base::DoVAORequirementsMatch(Shader_Base* pShader)
{
    if( pShader->m_aHandle_Position     == m_aHandle_Position &&
        pShader->m_aHandle_UVCoord      == m_aHandle_UVCoord &&
        pShader->m_aHandle_Normal       == m_aHandle_Normal &&
        pShader->m_aHandle_VertexColor  == m_aHandle_VertexColor )
    {
        return true;
    }

    return false;
}

bool Shader_Base::ActivateAndProgramShader(VertexFormats vertformat, BufferDefinition* vbo, BufferDefinition* ibo, int ibotype, MyMatrix* viewprojmatrix, MyMatrix* worldmatrix, GLuint texid, ColorByte tint, ColorByte speccolor, float shininess)
{
    if( m_Initialized == false )
    {
        if( LoadAndCompile() == false )
        {
            //LOGInfo( LOGTag, "Shader_Base::ActivateAndProgramShader - shader not ready.\n" );
            return false;
        }
    }

    glUseProgram( m_ProgramHandle );
    //LOGInfo( LOGTag, "glUseProgram %d\n", m_ProgramHandle );
    checkGlError( "glUseProgram" );

    if( vbo->m_CurrentVAOInitialized[g_ActiveShaderPass] == false )
    {
        if( glBindVertexArray != 0 )
        {
            vbo->m_CurrentVAOInitialized[g_ActiveShaderPass] = true;

            // First time using this VAO, so we create a VAO and set up all the attributes.
            vbo->CreateAndBindVAO();
#if _DEBUG && MYFW_WINDOWS
            vbo->m_DEBUG_ShaderUsedOnCreation[g_ActiveShaderPass] = this;
            vbo->m_DEBUG_VBOUsedOnCreation[vbo->m_DEBUG_CurrentVAOIndex[g_ActiveShaderPass]] = vbo->m_CurrentBufferID;
            if( ibo )
                vbo->m_DEBUG_IBOUsedOnCreation[vbo->m_DEBUG_CurrentVAOIndex[g_ActiveShaderPass]] = ibo->m_CurrentBufferID;
#endif
        }

        assert( vbo->m_VertexFormat == VertexFormat_None || vertformat == vbo->m_VertexFormat );
        InitializeAttributeArrays( vertformat, vbo?vbo->m_CurrentBufferID:0, ibo?ibo->m_CurrentBufferID:0, ibotype );
    }
    else
    {
#if _DEBUG && MYFW_WINDOWS
        assert( vbo->m_DEBUG_ShaderUsedOnCreation[g_ActiveShaderPass] == this ||
                this->DoVAORequirementsMatch( vbo->m_DEBUG_ShaderUsedOnCreation[g_ActiveShaderPass] ) );
        assert( vbo->m_DEBUG_VBOUsedOnCreation[vbo->m_DEBUG_CurrentVAOIndex[g_ActiveShaderPass]] == vbo->m_CurrentBufferID );
        if( ibo )
            assert( vbo->m_DEBUG_IBOUsedOnCreation[vbo->m_DEBUG_CurrentVAOIndex[g_ActiveShaderPass]] == ibo->m_CurrentBufferID );
#endif
        glBindVertexArray( vbo->m_CurrentVAOHandle[g_ActiveShaderPass] );
    }

    ProgramBaseUniforms( viewprojmatrix, worldmatrix, texid, tint, speccolor, shininess );

    return true;
}

void Shader_Base::ProgramBaseUniforms(MyMatrix* viewprojmatrix, MyMatrix* worldmatrix, GLuint texid, ColorByte tint, ColorByte speccolor, float shininess)
{
#if USE_D3D
    assert( 0 );
    //MyMatrix temp = *worldmatrix;
    //temp.Multiply( viewprojmatrix );

    //m_ShaderConstants.mvp = temp;
    //m_ShaderConstants.mvp.Transpose();
    //m_ShaderConstants.tint.Set( tint.r, tint.g, tint.b, tint.a );
    //g_pD3DContext->UpdateSubresource( m_pConstantsBuffer.Get(), 0, NULL, &m_ShaderConstants, 0, 0 );
#else
    ProgramPosition( viewprojmatrix, worldmatrix );

    if( m_uHandle_TextureColor != -1 )
    {
        MyActiveTexture( GL_TEXTURE0 + 0 );
        glBindTexture( GL_TEXTURE_2D, texid );

        glUniform1i( m_uHandle_TextureColor, 0 );
    }

    ProgramTint( tint );

    if( m_uHandle_TextureSpecColor != -1 )
        glUniform4f( m_uHandle_TextureSpecColor, speccolor.r / 255.0f, speccolor.g / 255.0f, speccolor.b / 255.0f, speccolor.a / 255.0f );

    if( m_uHandle_Shininess != -1 )
        glUniform1f( m_uHandle_Shininess, shininess );

    checkGlError( "Shader_Base::ActivateAndProgramShader" );
#endif //USE_D3D
}

void Shader_Base::ProgramPosition(MyMatrix* viewprojmatrix, MyMatrix* worldmatrix)
{
    if( m_uHandle_World != -1 )
        glUniformMatrix4fv( m_uHandle_World, 1, false, (GLfloat*)&worldmatrix->m11 );

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

void Shader_Base::ProgramCamera(Vector3* campos)
{
#if USE_D3D
    assert( 0 );
#else
    if( m_uHandle_CameraPos != -1 )
    {
        assert( campos != 0 );
        glUniform3f( m_uHandle_CameraPos, campos->x, campos->y, campos->z );
    }
#endif

    return;
}

void Shader_Base::ProgramLights(MyLight* lights, int numlights)
{
    if( numlights == 0 )
        return;

    assert( numlights <= MAX_LIGHTS );
    assert( lights );

#if USE_D3D
    assert( 0 );
#else
    for( int i=0; i<numlights; i++ )
    {
        if( m_uHandle_LightPos[i] != -1 )
        {
            glUniform3f( m_uHandle_LightPos[i], lights[i].m_Position.x, lights[i].m_Position.y, lights[i].m_Position.z );
        }

        if( m_uHandle_LightDir[i] != -1 )
        {
            glUniform3f( m_uHandle_LightDir[i], lights[i].m_SpotDirection.x, lights[i].m_SpotDirection.y, lights[i].m_SpotDirection.z );
        }

        if( m_uHandle_LightColor[i] != -1 )
        {
            glUniform4f( m_uHandle_LightColor[i], lights[i].m_Color.r, lights[i].m_Color.g, lights[i].m_Color.b, lights[i].m_Color.a );
        }

        if( m_uHandle_LightAttenuation[i] != -1 )
        {
            glUniform3f( m_uHandle_LightAttenuation[i], lights[i].m_Attenuation.x, lights[i].m_Attenuation.y, lights[i].m_Attenuation.z );
        }
    }
#endif

    return;
}

void Shader_Base::ProgramShadowLight(MyMatrix* worldmatrix, MyMatrix* shadowviewprojmatrix, GLuint shadowtexid)
{
    if( m_uHandle_ShadowLightWVP != -1 )
    {
        MyMatrix temp = *shadowviewprojmatrix * *worldmatrix;

        glUniformMatrix4fv( m_uHandle_ShadowLightWVP, 1, false, (GLfloat*)&temp.m11 );
    }

    if( m_uHandle_ShadowTexture != -1 )
    {
        MyActiveTexture( GL_TEXTURE0 + 1 );
        glBindTexture( GL_TEXTURE_2D, shadowtexid );

        glUniform1i( m_uHandle_ShadowTexture, 1 );
    }
}

void Shader_Base::ProgramLightmap(GLuint texid)
{
    if( m_uHandle_TextureLightmap != -1 )
    {
        MyActiveTexture( GL_TEXTURE0 + 2 );
        glBindTexture( GL_TEXTURE_2D, texid );

        glUniform1i( m_uHandle_TextureLightmap, 2 );
    }
}
