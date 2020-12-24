//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "Buffer_OpenGL.h"
#include "FBO_OpenGL.h"
#include "GLHelpers.h"
#include "Renderer_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"
#include "../BaseClasses/Renderer_Base.h"
#include "../BaseClasses/Renderer_Enums.h"
#include "../Old/OpenGLWrapper.h"
#include "../../DataTypes/ColorStructs.h"
#include "../../Particles/ParticleRendererInstanced.h"
#include "../../../SourceCommon/Textures/TextureManager.h"

#if MYFW_OPENGLES2

#define GL_HALF_FLOAT                       0
#define GL_DOUBLE                           0
#define GL_INT_2_10_10_10_REV               0
#define GL_UNSIGNED_INT_2_10_10_10_REV      0
#define GL_UNSIGNED_INT_10F_11F_11F_REV     0

#define GL_STENCIL_INDEX    0
#define GL_DEPTH_STENCIL    0
#define GL_RED              0
#define GL_GREEN            0
#define GL_BLUE             0
#define GL_BGR              0
#define GL_BGRA             0

#define GL_UNSIGNED_BYTE_3_3_2              0
#define GL_UNSIGNED_BYTE_2_3_3_REV          0
#define GL_UNSIGNED_SHORT_5_6_5             0
#define GL_UNSIGNED_SHORT_5_6_5_REV         0
#define GL_UNSIGNED_SHORT_4_4_4_4           0
#define GL_UNSIGNED_SHORT_4_4_4_4_REV       0
#define GL_UNSIGNED_SHORT_5_5_5_1           0
#define GL_UNSIGNED_SHORT_1_5_5_5_REV       0
#define GL_UNSIGNED_INT_8_8_8_8             0
#define GL_UNSIGNED_INT_8_8_8_8_REV         0
#define GL_UNSIGNED_INT_10_10_10_2          0
#define GL_UNSIGNED_INT_2_10_10_10_REV      0
#define GL_UNSIGNED_INT_24_8                0
#define GL_UNSIGNED_INT_10F_11F_11F_REV     0
#define GL_UNSIGNED_INT_5_9_9_9_REV         0
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV   0

#define GL_POINT    0
#define GL_LINE     0
#define GL_FILL     0

#define glClearDepth glClearDepthf

#endif //MYFW_OPENGLES2

//====================================================================================================
// Enum Conversions.
//====================================================================================================
GLint PrimitiveTypeConversionTable[MyRE::PrimitiveType_Undefined] =
{
    GL_POINTS,
    GL_LINES,
    GL_LINE_LOOP,
    GL_LINE_STRIP,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN,
};

GLint FrontFaceWindingConversionTable[MyRE::FrontFaceWinding_NumTypes] =
{
    GL_CW,
    GL_CCW,
};

GLint CullModeConversionTable[MyRE::CullMode_NumModes] =
{
    GL_FRONT,
    GL_BACK,
    GL_FRONT_AND_BACK,
};

GLint AttributeTypeConversionTable[MyRE::AttributeType_NumTypes] =
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_INT,
    GL_UNSIGNED_INT,
    GL_HALF_FLOAT,
    GL_FLOAT,
    GL_DOUBLE,
    GL_FIXED,
    GL_INT_2_10_10_10_REV,
    GL_UNSIGNED_INT_2_10_10_10_REV,
    GL_UNSIGNED_INT_10F_11F_11F_REV,
};

GLint BufferTypeConversionTable[MyRE::BufferType_NumTypes] =
{
    GL_ARRAY_BUFFER,
    GL_ELEMENT_ARRAY_BUFFER,
};

GLint BufferUsageConversionTable[MyRE::BufferUsage_NumTypes] =
{
    GL_STREAM_DRAW,
    GL_STATIC_DRAW,
    GL_DYNAMIC_DRAW,
};

GLint IndexTypeConversionTable[MyRE::IndexType_Undefined] =
{
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_INT,
};

GLenum BlendFactorConversionTable[MyRE::BlendFactor_NumTypes] = 
{
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    //GL_CONSTANT_COLOR,
    //GL_ONE_MINUS_CONSTANT_COLOR,
    //GL_CONSTANT_ALPHA,
    //GL_ONE_MINUS_CONSTANT_ALPHA,
    GL_SRC_ALPHA_SATURATE,
    //GL_SRC1_COLOR,
    //GL_ONE_MINUS_SRC1_COLOR,
    //GL_SRC1_ALPHA,
    //GL_ONE_MINUS_SRC1_ALPHA,
};

GLint MinFilterConversionTable[MyRE::MinFilter_NumTypes] =
{
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST_MIPMAP_NEAREST,
    GL_LINEAR_MIPMAP_NEAREST,
    GL_NEAREST_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
};

GLint MagFilterConversionTable[MyRE::MagFilter_NumTypes] =
{
    GL_NEAREST,
    GL_LINEAR,
};

GLint WrapModeConversionTable[MyRE::WrapMode_NumTypes] =
{
    GL_CLAMP_TO_EDGE,
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
};

GLint PixelFormatConversionTable[MyRE::PixelFormat_NumTypes] =
{
    GL_STENCIL_INDEX,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_STENCIL,
    GL_RED,
    GL_GREEN,
    GL_BLUE,
    GL_RGB,
    GL_BGR,
    GL_RGBA,
    GL_BGRA,
};

GLint PixelDataTypeConversionTable[MyRE::PixelDataType_NumTypes] =
{
    GL_UNSIGNED_BYTE,
    GL_BYTE,
    GL_UNSIGNED_SHORT,
    GL_SHORT,
    GL_UNSIGNED_INT,
    GL_INT,
    GL_HALF_FLOAT,
    GL_FLOAT,
    GL_UNSIGNED_BYTE_3_3_2,
    GL_UNSIGNED_BYTE_2_3_3_REV,
    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_SHORT_5_6_5_REV,
    GL_UNSIGNED_SHORT_4_4_4_4,
    GL_UNSIGNED_SHORT_4_4_4_4_REV,
    GL_UNSIGNED_SHORT_5_5_5_1,
    GL_UNSIGNED_SHORT_1_5_5_5_REV,
    GL_UNSIGNED_INT_8_8_8_8,
    GL_UNSIGNED_INT_8_8_8_8_REV,
    GL_UNSIGNED_INT_10_10_10_2,
    GL_UNSIGNED_INT_2_10_10_10_REV,
    GL_UNSIGNED_INT_24_8,
    GL_UNSIGNED_INT_10F_11F_11F_REV,
    GL_UNSIGNED_INT_5_9_9_9_REV,
    GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
};

GLint DepthFuncConversionTable[MyRE::DepthFunc_NumTypes] =
{
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS,
};

GLint PolygonDrawModeConversionTable[MyRE::PolygonDrawMode_NumModes] =
{
    GL_POINT,
    GL_LINE,
    GL_FILL,
};

//====================================================================================================
// Renderer_OpenGL.
//====================================================================================================
Renderer_OpenGL::Renderer_OpenGL(GameCore* pGameCore)
: Renderer_Base( pGameCore )
{
}

Renderer_OpenGL::~Renderer_OpenGL()
{
}

//====================================================================================================
//====================================================================================================
// PROTECTED METHODS.
//====================================================================================================
//====================================================================================================

//====================================================================================================
// Buffering methods used by Buffer_OpenGL.
//====================================================================================================
void Renderer_OpenGL::BufferData(Buffer_Base* pBuffer, GLuint bufferID, uint32 sizeInBytes, void* pData)
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)(pBuffer);

    GLenum target = BufferTypeConversionTable[pGLBuffer->m_BufferType];
    GLenum usage = BufferUsageConversionTable[pGLBuffer->m_BufferUsage];

    MyBindBuffer( target, bufferID );
    glBufferData( target, sizeInBytes, pData, usage );
    
    checkGlError( "BufferData" );
}

void Renderer_OpenGL::BufferSubData(Buffer_Base* pBuffer, GLuint bufferID, uint32 offset, uint32 sizeInBytes, void* pData)
{
    Buffer_OpenGL* pGLBuffer = (Buffer_OpenGL*)(pBuffer);

    GLenum target = BufferTypeConversionTable[pGLBuffer->m_BufferType];

    MyBindBuffer( target, bufferID );
    glBufferSubData( target, offset, sizeInBytes, pData );

    checkGlError( "BufferSubData" );
}

//====================================================================================================
//====================================================================================================
// PUBLIC METHODS.
//====================================================================================================
//====================================================================================================

//====================================================================================================
// Events.
//====================================================================================================
void Renderer_OpenGL::OnSurfaceCreated()
{
    Renderer_Base::OnSurfaceCreated();

    printGLString( "Version", GL_VERSION );
    printGLString( "Vendor", GL_VENDOR );
    printGLString( "Renderer", GL_RENDERER );
    printGLString( "Extensions", GL_EXTENSIONS );

    glClearColor( m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a );
    glClearDepth( m_ClearDepth );

    checkGlError( "OnSurfaceCreated" );
}

void Renderer_OpenGL::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height)
{
    Renderer_Base::OnSurfaceChanged( x, y, width, height );

    // Only draw to part of the window with glScissor and glViewPort.
    if( x != 0 || y != 0 )
    {
        // Scissor test is really only needed for the glClear call.
        glEnable( GL_SCISSOR_TEST );
        glScissor( x, y, width, height );
    }

    glViewport( x, y, width, height );

    checkGlError( "OnSurfaceChanged" );
}

//====================================================================================================
// State Change.
//====================================================================================================
void Renderer_OpenGL::SetClearColor(ColorFloat color)
{
    Renderer_Base::SetClearColor( color );

    glClearColor( color.r, color.g, color.b, color.a );

    checkGlError( "SetClearColor" );
}

void Renderer_OpenGL::SetClearDepth(float depth)
{
    Renderer_Base::SetClearDepth( depth );

    glClearDepth( depth );

    checkGlError( "SetClearDepth" );
}

void Renderer_OpenGL::SetDepthWriteEnabled(bool enabled)
{
    Renderer_Base::SetDepthWriteEnabled( enabled );

    glDepthMask( enabled );

    checkGlError( "SetDepthWriteEnabled" );
}

void Renderer_OpenGL::SetDepthTestEnabled(bool enabled)
{
    Renderer_Base::SetDepthTestEnabled( enabled );

    if( enabled )
        glEnable( GL_DEPTH_TEST );
    else
        glDisable( GL_DEPTH_TEST );

    checkGlError( "SetDepthTestEnabled" );
}

void Renderer_OpenGL::SetDepthFunction(MyRE::DepthFuncs func)
{
    Renderer_Base::SetDepthFunction( func );

    glDepthFunc( DepthFuncConversionTable[func] );

    checkGlError( "SetDepthFunction" );
}

void Renderer_OpenGL::SetCullingEnabled(bool enabled)
{
    Renderer_Base::SetCullingEnabled( enabled );

    if( enabled )
        glEnable( GL_CULL_FACE );
    else
        glDisable( GL_CULL_FACE );

    checkGlError( "SetCullingEnabled" );
}

void Renderer_OpenGL::SetCullMode(MyRE::CullModes mode)
{
    Renderer_Base::SetCullMode( mode );

    glCullFace( CullModeConversionTable[mode] );

    checkGlError( "SetCullMode" );
}

void Renderer_OpenGL::SetFrontFaceWinding(MyRE::FrontFaceWindings winding)
{
    Renderer_Base::SetFrontFaceWinding( winding );

    glFrontFace( FrontFaceWindingConversionTable[winding] );

    checkGlError( "SetFrontFaceWinding" );
}

void Renderer_OpenGL::SetSwapInterval(int32 interval)
{
    Renderer_Base::SetSwapInterval( interval );

#if !MYFW_OPENGLES2
    if( wglSwapInterval )
        wglSwapInterval( interval );
#endif

    checkGlError( "SetSwapInterval" );
}

void Renderer_OpenGL::SetBlendEnabled(bool enabled)
{
    Renderer_Base::SetBlendEnabled( enabled );

    if( enabled )
        glEnable( GL_BLEND );
    else
        glDisable( GL_BLEND );

    checkGlError( "SetBlendEnabled" );
}

void Renderer_OpenGL::SetBlendFunc(MyRE::BlendFactors srcFactor, MyRE::BlendFactors dstFactor)
{
    Renderer_Base::SetBlendFunc( srcFactor, dstFactor );

    glBlendFunc( BlendFactorConversionTable[srcFactor], BlendFactorConversionTable[dstFactor] );

    checkGlError( "SetBlendFunc" );
}

void Renderer_OpenGL::SetLineWidth(float width)
{
    Renderer_Base::SetLineWidth( width );

    glLineWidth( width );

    checkGlError( "SetLineWidth" );
}

void Renderer_OpenGL::SetPointSize(float size)
{
    Renderer_Base::SetPointSize( size );

#if !MYFW_OPENGLES2
    glPointSize( size );
#endif

    checkGlError( "SetPointSize" );
}

//====================================================================================================
// Actions.
//====================================================================================================
Shader_Base* Renderer_OpenGL::CreateShader(ShaderPassTypes passType, TextureDefinition* pErrorTexture)
{
    Shader_Base* pShader = MyNew Shader_OpenGL( m_pGameCore, passType );

#if MYFW_EDITOR
    pShader->SetErrorTexture( pErrorTexture );
#endif

    return pShader;
}

TextureDefinition* Renderer_OpenGL::CreateTexture()
{
    return MyNew Texture_OpenGL();
}

FBODefinition* Renderer_OpenGL::CreateFBO()
{
    return MyNew FBO_OpenGL();
}

void Renderer_OpenGL::ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil)
{
    GLbitfield flags = 0;
    if( clearColor )   flags |= GL_COLOR_BUFFER_BIT;
    if( clearDepth )   flags |= GL_DEPTH_BUFFER_BIT;
    if( clearStencil ) flags |= GL_STENCIL_BUFFER_BIT;

    glClear( flags );

    checkGlError( "ClearBuffers" );
}

void Renderer_OpenGL::ClearScissorRegion()
{
    glDisable( GL_SCISSOR_TEST );

    checkGlError( "ClearScissorRegion" );
}

void Renderer_OpenGL::EnableViewport(MyViewport* pViewport, bool enableOrDisableScissorIfNeeded)
{
    if( enableOrDisableScissorIfNeeded )
    {
        // Set up scissor test if not drawing to the whole window.
        if( pViewport->GetX() != 0 || pViewport->GetY() != 0 )
        {
            // Scissor test is really only needed for the glClear call, glViewport will handle the rest.
            glEnable( GL_SCISSOR_TEST );
            glScissor( pViewport->GetX(), pViewport->GetY(), pViewport->GetWidth(), pViewport->GetHeight() );
        }
        else
        {
            glDisable( GL_SCISSOR_TEST );
        }
    }

    glViewport( pViewport->GetX(), pViewport->GetY(), pViewport->GetWidth(), pViewport->GetHeight() );

    checkGlError( "EnableViewport" );
}

bool ShouldDraw(bool hideFromDrawList)
{
    bool draw = true;
    
    // If this is the canvas being debugged, limit what gets drawn (if there's a limit set).
    if( g_GLStats.m_DrawCallLimit_Canvas == (int)g_GLStats.m_CurrentCanvasID &&
        g_GLStats.m_DrawCallLimit_Index != -1 )
    {
        // Don't draw "hidden/editor" items if we're debugging.
        if( hideFromDrawList == true )
            draw = false;

        // Don't draw object above the draw limit.
        if( g_GLStats.m_NumDrawCallsThisFrameSoFar > g_GLStats.m_DrawCallLimit_Index )
            draw = false;

#if MYFW_WINDOWS
        // For debugging, trigger a breakpoint if we're on a certain draw call.
        if( draw )
        {
            if( g_GLStats.m_DrawCallLimit_BreakPointIndex == g_GLStats.m_NumDrawCallsThisFrameSoFar )
            {
                __debugbreak();
                g_GLStats.m_DrawCallLimit_BreakPointIndex = -1;
            }
        }
#endif
    }

    return draw;
}

void Renderer_OpenGL::DrawArrays(MyRE::PrimitiveTypes mode, uint32 first, uint32 count, bool hideFromDrawList)
{
    MyAssert( mode < MyRE::PrimitiveType_Undefined );

    bool draw = ShouldDraw( hideFromDrawList );

    if( draw )
    {
        glDrawArrays( PrimitiveTypeConversionTable[mode], first, count );
    }

    if( hideFromDrawList == false )
    {
        g_GLStats.m_NumDrawCallsThisFrameSoFar++;
    }

    checkGlError( "glDrawArrays" );
}

void Renderer_OpenGL::DrawElements(MyRE::PrimitiveTypes mode, uint32 count, MyRE::IndexTypes IBOType, const void* indices, bool hideFromDrawList)
{
    MyAssert( mode < MyRE::PrimitiveType_Undefined );

    bool draw = ShouldDraw( hideFromDrawList );
    
    if( draw )
    {
        glDrawElements( PrimitiveTypeConversionTable[mode], count, IndexTypeConversionTable[IBOType], indices );
    }

    if( hideFromDrawList == false )
    {
        g_GLStats.m_NumDrawCallsThisFrameSoFar++;
    }

    checkGlError( "DrawElements" );
}

void Renderer_OpenGL::TempHack_SetupAndDrawInstanced(Shader_Base* pShader, uint32 numInstancesToDraw)
{
#if MYFW_USEINSTANCEDPARTICLES
    Shader_OpenGL* pGLShader = (Shader_OpenGL*)pShader;

    GLint aiposloc = glGetAttribLocation( pGLShader->m_ProgramHandle, "ai_Position" );
    GLint aiscaleloc = glGetAttribLocation( pGLShader->m_ProgramHandle, "ai_Scale" );
    GLint aicolorloc = glGetAttribLocation( pGLShader->m_ProgramHandle, "ai_Color" );

    {
        if( aiposloc != -1 )
        {
            glVertexAttribPointer( aiposloc, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)0 );
            glEnableVertexAttribArray( aiposloc );
            glVertexAttribDivisor( aiposloc, 1 );
        }

        if( aiscaleloc != -1 )
        {
            glVertexAttribPointer( aiscaleloc, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstanceData), (void*)12 );
            glEnableVertexAttribArray( aiscaleloc );
            glVertexAttribDivisor( aiscaleloc, 1 );
        }

        if( aicolorloc != -1 )
        {
            glVertexAttribPointer( aicolorloc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ParticleInstanceData), (void*)16 );
            glEnableVertexAttribArray( aicolorloc );
            glVertexAttribDivisor( aicolorloc, 1 );
        }
    }

    checkGlError( "before glDrawArraysInstanced() in TempHack_SetupAndDrawInstanced()" );

    glDrawArraysInstanced( GL_TRIANGLE_STRIP, 0, 4, numInstancesToDraw );

    checkGlError( "after glDrawArraysInstanced() in TempHack_SetupAndDrawInstanced()" );

    if( aiposloc != -1 )
        glVertexAttribDivisor( aiposloc, 0 );
    if( aiscaleloc != -1 )
        glVertexAttribDivisor( aiscaleloc, 0 );
    if( aicolorloc != -1 )
        glVertexAttribDivisor( aicolorloc, 0 );

    if( aiposloc != -1 )
        glDisableVertexAttribArray( aiposloc );
    if( aiscaleloc != -1 )
        glDisableVertexAttribArray( aiscaleloc );
    if( aicolorloc != -1 )
        glDisableVertexAttribArray( aicolorloc );

    checkGlError( "after glVertexAttribDivisor() in TempHack_SetupAndDrawInstanced()" );
#endif //MYFW_USEINSTANCEDPARTICLES
}

void Renderer_OpenGL::TempHack_UnbindVBOAndIBO()
{
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void Renderer_OpenGL::ReadPixels(int x, int y, uint32 width, uint32 height, MyRE::PixelFormats format, MyRE::PixelDataTypes dataType, void* buffer)
{
    glReadPixels( x, y, width, height, PixelFormatConversionTable[format], PixelDataTypeConversionTable[dataType], buffer );

    checkGlError( "ReadPixels" );
}

void Renderer_OpenGL::SetPolygonMode(MyRE::PolygonDrawModes mode)
{
#if !MYFW_OPENGLES2
    glPolygonMode( GL_FRONT_AND_BACK, PolygonDrawModeConversionTable[mode] );
#endif

    checkGlError( "SetPolygonMode" );
}

void Renderer_OpenGL::SetPolygonOffset(bool enabled, float factor, float units)
{
    if( enabled )
    {
#if !MYFW_OPENGLES2
        glEnable( GL_POLYGON_OFFSET_LINE );
#endif
        glEnable( GL_POLYGON_OFFSET_FILL ); // Enabling GL_POLYGON_OFFSET_LINE doesn't work on my intel 4000.
    }
    else
    {
        glDisable( GL_POLYGON_OFFSET_FILL );
#if !MYFW_OPENGLES2
        glDisable( GL_POLYGON_OFFSET_LINE );
#endif
    }

    glPolygonOffset( factor, units );

    checkGlError( "SetPolygonOffset" );
}

//====================================================================================================
// Textures/FBOs.
//====================================================================================================
void Renderer_OpenGL::SetTextureMinMagFilters(TextureDefinition* pTexture, MyRE::MinFilters min, MyRE::MagFilters mag)
{
    MyAssert( pTexture != nullptr );
    MyAssert( min < MyRE::MinFilter_NumTypes );
    MyAssert( mag < MyRE::MagFilter_NumTypes );

    Texture_OpenGL* pGLTexture = (Texture_OpenGL*)pTexture;

    // Note: This does not preserve the current texture bindings.
    glBindTexture( GL_TEXTURE_2D, pGLTexture->GetTextureID() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MinFilterConversionTable[min] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MagFilterConversionTable[mag] );
    glBindTexture( GL_TEXTURE_2D, 0 );

    checkGlError( "SetTextureMinMagFilters" );
}

void Renderer_OpenGL::SetTextureWrapModes(TextureDefinition* pTexture, MyRE::WrapModes wrapModeS, MyRE::WrapModes wrapModeT)
{
    MyAssert( pTexture != nullptr );
    MyAssert( wrapModeS < MyRE::WrapMode_NumTypes );
    MyAssert( wrapModeT < MyRE::WrapMode_NumTypes );

    Texture_OpenGL* pGLTexture = (Texture_OpenGL*)pTexture;

    // Note: This does not preserve the current texture bindings.
    glBindTexture( GL_TEXTURE_2D, pGLTexture->GetTextureID() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapModeConversionTable[wrapModeS] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapModeConversionTable[wrapModeT] );
    glBindTexture( GL_TEXTURE_2D, 0 );

    checkGlError( "SetTextureWrapMode" );
}

void Renderer_OpenGL::BindFramebuffer(GLuint framebuffer)
{
    MyBindFramebuffer( GL_FRAMEBUFFER, framebuffer, 0, 0 );
}
