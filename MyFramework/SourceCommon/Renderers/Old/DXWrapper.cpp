//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#if MYFW_WINDOWS

Microsoft::WRL::ComPtr<ID3D11Device1> g_pD3DDevice;
Microsoft::WRL::ComPtr<ID3D11DeviceContext1> g_pD3DContext;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_pD3DRenderTargetView;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_pD3DDepthStencilView;

Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pD3DSampleStateNearestWrap;
Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pD3DSampleStateLinearWrap;

Microsoft::WRL::ComPtr<ID3D11BlendState> g_pD3DBlendStateDisabled;
Microsoft::WRL::ComPtr<ID3D11BlendState> g_pD3DBlendStateEnabled;
Microsoft::WRL::ComPtr<ID3D11BlendState> g_pD3DBlendStateEnabledAdditive;

Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteEnabled;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteEnabled;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteDisabled;
Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteDisabled;

struct GenericShader
{
    bool m_InUse;
    GLenum m_Type;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader; //MAX_VERTEX_SHADERS];
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pFragmentShader; //MAX_FRAGMENT_SHADERS];
};

struct ShaderProgram
{
    bool m_InUse;

    int m_VertexShaderIndex;
    int m_FragmentShaderIndex;
};

GenericShader g_D3DShaderArray[MAX_SHADERS];
ShaderProgram g_D3DShaderProgramArray[MAX_SHADER_PROGRAMS];
GenericBuffer g_D3DBufferObjects[MAX_BUFFERS];
GenericTextures g_D3DTextures[MAX_TEXTURES];
int g_ActiveBufferObject_Array = -1;
int g_ActiveBufferObject_ElementArray = -1;
bool g_DepthMaskEnabled = false;
bool g_DepthTestEnabled = false;

float g_ClearColor[4];

void UpdateDepthState()
{
    if( g_DepthTestEnabled == true && g_DepthMaskEnabled == true )
        g_pD3DContext->OMSetDepthStencilState( g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteEnabled.Get(), 1 );

    else if( g_DepthTestEnabled == false && g_DepthMaskEnabled == true )
        g_pD3DContext->OMSetDepthStencilState( g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteEnabled.Get(), 1 );

    else if( g_DepthTestEnabled == true && g_DepthMaskEnabled == false )
        g_pD3DContext->OMSetDepthStencilState( g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteDisabled.Get(), 1 );

    else if( g_DepthTestEnabled == false && g_DepthMaskEnabled == false )
        g_pD3DContext->OMSetDepthStencilState( g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteDisabled.Get(), 1 );
}

void D3DCreateDepthStencilStates()
{
    HRESULT result;

    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
    ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

    depthDisabledStencilDesc.DepthEnable = true;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthDisabledStencilDesc.StencilEnable = true;
    depthDisabledStencilDesc.StencilReadMask = 0xFF;
    depthDisabledStencilDesc.StencilWriteMask = 0xFF;
    depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Test: Enabled    Write: Enabled
    depthDisabledStencilDesc.DepthEnable = true;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    result = g_pD3DDevice->CreateDepthStencilState( &depthDisabledStencilDesc, &g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteEnabled );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create DepthStencilState\n" );
    }

    // Test: Disabled   Write: Enabled
    depthDisabledStencilDesc.DepthEnable = false;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    result = g_pD3DDevice->CreateDepthStencilState( &depthDisabledStencilDesc, &g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteEnabled );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create DepthStencilState\n" );
    }

    // Test: Enabled    Write: Disabled
    depthDisabledStencilDesc.DepthEnable = true;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    result = g_pD3DDevice->CreateDepthStencilState( &depthDisabledStencilDesc, &g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteDisabled );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create DepthStencilState\n" );
    }

    // Test: Disabled   Write: Disabled
    depthDisabledStencilDesc.DepthEnable = false;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    result = g_pD3DDevice->CreateDepthStencilState( &depthDisabledStencilDesc, &g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteDisabled );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create DepthStencilState\n" );
    }
}

void D3DCreateSamplerStates()
{
    HRESULT result;

    // Set up the common properties for our samplers
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory( &samplerDesc, sizeof(D3D11_SAMPLER_DESC) );

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create a texture sampler state for nearest/point sampling and wrapping uv's
    result = g_pD3DDevice->CreateSamplerState( &samplerDesc, &g_pD3DSampleStateNearestWrap );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create SamplerState\n" );
    }

    // Create a texture sampler state for tri-linear sampling and wrapping uv's
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    result = g_pD3DDevice->CreateSamplerState( &samplerDesc, &g_pD3DSampleStateLinearWrap );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create SamplerState\n" );
    }
}

void D3DCreateBlendStates()
{
    HRESULT result;

    // Set up the common properties for our blend states... set up default to disabled.
    D3D11_BLEND_DESC blendStateDesc;
    ZeroMemory( &blendStateDesc, sizeof(D3D11_BLEND_DESC) );

    blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // Create a disabled blend state
    blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
    result = g_pD3DDevice->CreateBlendState( &blendStateDesc, &g_pD3DBlendStateDisabled );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create BlendState\n" );
    }

    // Create an enabled blend state
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    result = g_pD3DDevice->CreateBlendState( &blendStateDesc, &g_pD3DBlendStateEnabled );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create BlendState\n" );
    }

    // Create a additive blend state
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    result = g_pD3DDevice->CreateBlendState( &blendStateDesc, &g_pD3DBlendStateEnabledAdditive );
    if( FAILED( result ) )
    {
        LOGInfo( LOGTag, "Failed to create BlendState\n" );
    }
}

void DXWrapper_InitDeviceAndContext( Microsoft::WRL::ComPtr<ID3D11Device1> pDevice,
                                     Microsoft::WRL::ComPtr<ID3D11DeviceContext1> pContext,
                                     Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView,
                                     Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView )
{
    g_pD3DDevice = pDevice;
    g_pD3DContext = pContext;
    g_pD3DRenderTargetView = pRenderTargetView;
    g_pD3DDepthStencilView = pDepthStencilView;

    g_pD3DContext->OMSetRenderTargets( 1, g_pD3DRenderTargetView.GetAddressOf(), g_pD3DDepthStencilView.Get() );

    D3DCreateDepthStencilStates();
    D3DCreateSamplerStates();
    D3DCreateBlendStates();

    float blendfactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_pD3DContext->OMSetBlendState( g_pD3DBlendStateEnabled.Get(), blendfactor, 0xfff );

    UpdateDepthState();
}

void DXWrapper_InitShadersAndBuffers()
{
    for( int i=0; i<MAX_SHADERS; i++ )
    {
        g_D3DShaderArray[i].m_InUse = false;
        //g_D3DShaderArray[i].m_pVertexShader.Reset();
        //g_D3DShaderArray[i].m_pFragmentShader.Reset();
    }

    for( int i=0; i<MAX_SHADER_PROGRAMS; i++ )
    {
        g_D3DShaderProgramArray[i].m_InUse = false;
    }

    for( int i=0; i<MAX_BUFFERS; i++ )
    {
        g_D3DBufferObjects[i].m_InUse = false;
    }
}

void DXWrapper_CleanupDeviceAndContext()
{
}

void DXWrapper_CleanupShadersAndBuffers()
{
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    g_ClearColor[0] = red;
    g_ClearColor[1] = green;
    g_ClearColor[2] = blue;
    g_ClearColor[3] = alpha;
}

void glClear(GLbitfield mask)
{
    if( g_pD3DContext )
    {
        g_pD3DContext->ClearRenderTargetView( g_pD3DRenderTargetView.Get(), g_ClearColor );
        g_pD3DContext->ClearDepthStencilView( g_pD3DDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );
    }
}

GLuint glCreateShader(GLenum type)
{
    for( int i=0; i<MAX_SHADERS; i++ )
    {
        if( g_D3DShaderArray[i].m_InUse == false )
        {
            g_D3DShaderArray[i].m_InUse = true;
            g_D3DShaderArray[i].m_Type = type;
            return i+1;
        }
    }

    return 0;
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths)
{
    MyAssert( count == 1 );

    // shader is precompiled on windows phone, so just pass in value directly
    if( g_D3DShaderArray[shader-1].m_Type == GL_VERTEX_SHADER )
    {
        g_pD3DDevice->CreateVertexShader( strings[0], lengths[0], nullptr, &g_D3DShaderArray[shader-1].m_pVertexShader );
    }
    else if( g_D3DShaderArray[shader-1].m_Type == GL_FRAGMENT_SHADER )
    {
        g_pD3DDevice->CreatePixelShader( strings[0], lengths[0], nullptr, &g_D3DShaderArray[shader-1].m_pFragmentShader );
    }
}

void glCompileShader(GLuint shader)
{
    // Nothing here... we do all shader creation work in glShaderSource.
}

GLuint glCreateProgram()
{
    for( int i=0; i<MAX_SHADERS; i++ )
    {
        if( g_D3DShaderProgramArray[i].m_InUse == false )
        {
            g_D3DShaderProgramArray[i].m_InUse = true;
            g_D3DShaderProgramArray[i].m_VertexShaderIndex = 0;
            g_D3DShaderProgramArray[i].m_FragmentShaderIndex = 0;
            return i+1;
        }
    }

    return 0;
}

void glAttachShader(GLuint program, GLuint shader)
{
    if( g_D3DShaderArray[shader-1].m_Type == GL_VERTEX_SHADER )
        g_D3DShaderProgramArray[program-1].m_VertexShaderIndex = shader-1;
    else if( g_D3DShaderArray[shader-1].m_Type == GL_FRAGMENT_SHADER )
        g_D3DShaderProgramArray[program-1].m_FragmentShaderIndex = shader-1;
}

void glLinkProgram(GLuint program)
{
    // Nothing here... we're not linking in direct x... odd, but will see if works.
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint* param)
{
    if( pname == GL_COMPILE_STATUS )
    {
        if( g_D3DShaderArray[shader-1].m_Type == GL_VERTEX_SHADER )
        {
            *param = g_D3DShaderArray[shader-1].m_pVertexShader != 0;
        }
        else if( g_D3DShaderArray[shader-1].m_Type == GL_FRAGMENT_SHADER )
        {
            *param = g_D3DShaderArray[shader-1].m_pFragmentShader != 0;
        }
    }
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* param)
{
    if( pname == GL_LINK_STATUS )
    {
        *param = 0;

        if( g_D3DShaderProgramArray[program-1].m_InUse )
        {
            if( g_D3DShaderArray[g_D3DShaderProgramArray[program-1].m_VertexShaderIndex].m_Type == GL_VERTEX_SHADER &&
                g_D3DShaderArray[g_D3DShaderProgramArray[program-1].m_FragmentShaderIndex].m_Type == GL_FRAGMENT_SHADER )
            {
                *param = 1;
            }
        }
    }
}

void glUseProgram(GLuint program)
{
    if( program > 1 )
        int bp = 1;
    int vsindex = g_D3DShaderProgramArray[program-1].m_VertexShaderIndex;
    int fsindex = g_D3DShaderProgramArray[program-1].m_FragmentShaderIndex;

    if( vsindex > 5 || vsindex < 0 )
        int bp = 1;

    if( fsindex > 5 || fsindex < 0 )
        int bp = 1;

    if( g_D3DShaderArray[vsindex].m_pVertexShader )
        g_pD3DContext->VSSetShader( g_D3DShaderArray[vsindex].m_pVertexShader.Get(), nullptr, 0 );
    if( g_D3DShaderArray[fsindex].m_pFragmentShader )
        g_pD3DContext->PSSetShader( g_D3DShaderArray[fsindex].m_pFragmentShader.Get(), nullptr, 0 );
}

void glDeleteProgram(GLuint program)
{
    // there are no programs to delete, just turn off the slot.
    g_D3DShaderProgramArray[program-1].m_InUse = false;
}

void glDeleteShader(GLuint shader)
{
    if( g_D3DShaderArray[shader-1].m_Type == GL_VERTEX_SHADER )
    {
        // TODO: does this actually delete the shader?!?
        g_D3DShaderArray[shader-1].m_pVertexShader.Reset();
    }
    else if( g_D3DShaderArray[shader-1].m_Type == GL_FRAGMENT_SHADER )
    {
        // TODO: does this actually delete the shader?!?
        g_D3DShaderArray[shader-1].m_pFragmentShader.Reset();
    }

    g_D3DShaderArray[shader-1].m_InUse = false;
}

void glDetachShader(GLuint program, GLuint shader)
{
    if( g_D3DShaderProgramArray[program-1].m_VertexShaderIndex == shader-1 )
    {
        g_D3DShaderProgramArray[program-1].m_VertexShaderIndex = 0;
    }

    if( g_D3DShaderProgramArray[program-1].m_FragmentShaderIndex == shader-1 )
    {
        g_D3DShaderProgramArray[program-1].m_FragmentShaderIndex = 0;
    }
}


void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    LOGInfo( LOGTag, "********************************* glDeleteBuffers() not implemented\n" );
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
    LOGInfo( LOGTag, "********************************* glDeleteTextures() not implemented\n" );
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    LOGInfo( LOGTag, "********************************* glViewport() not implemented\n" );
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    //LOGInfo( LOGTag, "********************************* glBlendFunc() not implemented\n" );
}

GLint glGetAttribLocation(GLuint program, const GLchar* name)
{
    LOGInfo( LOGTag, "********************************* glGetAttribLocation() not implemented\n" );
    return -1;
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
    LOGInfo( LOGTag, "********************************* glGetUniformLocation() not implemented\n" );
    return -1;
}

void glGenBuffers(GLsizei n, GLuint* buffers)
{
    for( int j=0; j<n; j++ )
    {
        buffers[j] = 0;

        for( int i=0; i<MAX_BUFFERS; i++ )
        {
            if( g_D3DBufferObjects[i].m_InUse == false )
            {
                buffers[j] = i+1;
                g_D3DBufferObjects[i].m_InUse = true;
                break;
            }
        }
    }
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    if( buffer != 0 )
        MyAssert( g_D3DBufferObjects[buffer-1].m_InUse == true );

    if( target == GL_ARRAY_BUFFER )
        g_ActiveBufferObject_Array = buffer-1;
    else if( target == GL_ELEMENT_ARRAY_BUFFER )
        g_ActiveBufferObject_ElementArray = buffer-1;
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    if( g_pD3DDevice == 0 )
    {
        LOGInfo( LOGTag, "Trying to initialize a buffer before d3d is initialized!\n" );
        return;
    }

    int buffer = -1;

    if( target == GL_ARRAY_BUFFER )
        buffer = g_ActiveBufferObject_Array;
    else if( target == GL_ELEMENT_ARRAY_BUFFER )
        buffer = g_ActiveBufferObject_ElementArray;

    if( buffer == -1 )
        return;

    MyAssert( g_D3DBufferObjects[buffer].m_InUse == true );

    if( g_D3DBufferObjects[buffer].m_Buffer == 0 )
    {
        D3D11_BUFFER_DESC BufferDesc;

        // TODO: handle usage
        BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        BufferDesc.ByteWidth = size;
        if( target == GL_ARRAY_BUFFER )
            BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        else if( target == GL_ELEMENT_ARRAY_BUFFER )
            BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;    
        BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        BufferDesc.MiscFlags = 0;
        BufferDesc.StructureByteStride = 0;

        // Now create the vertex buffer.
        HRESULT result = g_pD3DDevice->CreateBuffer( &BufferDesc, 0, &g_D3DBufferObjects[buffer].m_Buffer );
        if( FAILED(result) )
            return;
    }

    // set the buffer data:
    if( data != 0 )
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;

        // Lock the vertex buffer so it can be written to.
        HRESULT result = g_pD3DContext->Map( g_D3DBufferObjects[buffer].m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if( FAILED(result) )
            return;

        // Get a pointer to the data in the vertex buffer.
        void* verticesPtr = (void*)mappedResource.pData;

        // Copy the data into the vertex buffer.
        memcpy( verticesPtr, (void*)data, size );

        // Unlock the vertex buffer.
        g_pD3DContext->Unmap( g_D3DBufferObjects[buffer].m_Buffer.Get(), 0 );
    }
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    if( g_pD3DDevice == 0 )
    {
        LOGInfo( LOGTag, "Trying to buffer data before d3d is initialized!\n" );
        return;
    }

    int buffer = -1;

    if( target == GL_ARRAY_BUFFER )
        buffer = g_ActiveBufferObject_Array;
    else if( target == GL_ELEMENT_ARRAY_BUFFER )
        buffer = g_ActiveBufferObject_ElementArray;

    if( buffer == -1 )
    {
        LOGInfo( LOGTag, "glBufferSubData() buffer not found!\n" );
        return;
    }

    MyAssert( g_D3DBufferObjects[buffer].m_InUse == true );

    if( g_D3DBufferObjects[buffer].m_Buffer == 0 )
    {
        LOGInfo( LOGTag, "glBufferSubData() buffer not allocated!\n" );
        return;
    }

    // set the buffer data:
    if( data != 0 )
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;

        // Lock the vertex buffer so it can be written to.
        HRESULT result = g_pD3DContext->Map( g_D3DBufferObjects[buffer].m_Buffer.Get(), 0,
                                             D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
        if( FAILED(result) )
            return;

        // Get a pointer to the data in the vertex buffer.
        void* verticesPtr = (void*)((int*)mappedResource.pData + offset);

        // Copy the data into the vertex buffer.
        memcpy( verticesPtr, (void*)data, size );

        // Unlock the vertex buffer.
        g_pD3DContext->Unmap( g_D3DBufferObjects[buffer].m_Buffer.Get(), 0 );
    }
}

void glEnableVertexAttribArray(GLuint index)
{
    LOGInfo( LOGTag, "********************************* glEnableVertexAttribArray() not implemented\n" );
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    LOGInfo( LOGTag, "********************************* glVertexAttribPointer() not implemented\n" );
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    LOGInfo( LOGTag, "********************************* glUniformMatrix4fv() not implemented\n" );
}
void glUniform1f(GLint location, GLfloat v0)
{
    LOGInfo( LOGTag, "********************************* glUniform1f() not implemented\n" );
}
void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    LOGInfo( LOGTag, "********************************* glUniform2f() not implemented\n" );
}
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    LOGInfo( LOGTag, "********************************* glUniform4f() not implemented\n" );
}
void glUniform1i(GLint location, GLint64EXT x)
{
    LOGInfo( LOGTag, "********************************* glUniform1i() not implemented\n" );
}

void glActiveTexture(GLenum texture)
{
    LOGInfo( LOGTag, "********************************* glActiveTexture() not implemented\n" );
}
void glBindTexture(GLenum unit, GLenum value)
{
    LOGInfo( LOGTag, "********************************* glBindTexture() not implemented\n" );
}
void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    LOGInfo( LOGTag, "********************************* glTexParameteri() not implemented\n" );
}

void glDisableVertexAttribArray(GLuint index)
{
    //LOGInfo( LOGTag, "********************************* glDisableVertexAttribArray() not implemented\n" );
}

void glEnable(GLuint id)
{
    if( id == GL_DEPTH_TEST )
    {
        g_DepthTestEnabled = true;
        UpdateDepthState();
    }

    //LOGInfo( LOGTag, "********************************* glEnable() not implemented\n" );
}

void glDisable(GLuint id)
{
    if( id == GL_DEPTH_TEST )
    {
        g_DepthTestEnabled = false;
        UpdateDepthState();
    }

    //LOGInfo( LOGTag, "********************************* glDisable() not implemented\n" );
}

void glDepthMask(GLboolean flag)
{
    g_DepthMaskEnabled = flag;
    UpdateDepthState();
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    g_pD3DContext->Draw( count, first );
    //LOGInfo( LOGTag, "********************************* glDrawArrays() not implemented\n" );
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    g_pD3DContext->DrawIndexed( count, 0, 0 );
    //LOGInfo( LOGTag, "********************************* glDrawElements() not implemented\n" );
}

#endif //MYFW_WINDOWS