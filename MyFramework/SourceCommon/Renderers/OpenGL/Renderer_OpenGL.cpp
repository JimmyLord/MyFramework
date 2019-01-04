//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "../Renderer_Enums.h"
#include "../Renderer_Base.h"
#include "Renderer_OpenGL.h"

#include <gl/GL.h>
#include <gl/GLU.h>

#if MYFW_WINDOWS
#include "../SourceWindows/GLExtensions.h"
#include "../SourceWindows/WGLExtensions.h"
#endif //MYFW_WINDOWS

#include "GLHelpers.h"

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
Renderer_OpenGL::Renderer_OpenGL()
{
}

Renderer_OpenGL::~Renderer_OpenGL()
{
}

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

    checkGlError( "OnSurfaceCreated\n" );
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

    checkGlError( "glViewport" );
}

//====================================================================================================
// State Change.
//====================================================================================================
void Renderer_OpenGL::SetClearColor(ColorFloat color)
{
    Renderer_Base::SetClearColor( color );

    glClearColor( color.r, color.g, color.b, color.a );

    checkGlError( "glClearColor" );
}

void Renderer_OpenGL::SetClearDepth(float depth)
{
    Renderer_Base::SetClearDepth( depth );

    glClearDepth( depth );

    checkGlError( "glClearDepth" );
}

void Renderer_OpenGL::SetDepthWriteEnabled(bool enabled)
{
    Renderer_Base::SetDepthWriteEnabled( enabled );

    glDepthMask( enabled );

    checkGlError( "glClearDepth" );
}

void Renderer_OpenGL::SetDepthTestEnabled(bool enabled)
{
    Renderer_Base::SetDepthTestEnabled( enabled );

    if( enabled )
        glEnable( GL_DEPTH_TEST );
    else
        glDisable( GL_DEPTH_TEST );

    checkGlError( "glEnable or glDisable( GL_DEPTH_TEST )" );
}

void Renderer_OpenGL::SetDepthFunction(MyRE::DepthFuncs func)
{
    Renderer_Base::SetDepthFunction( func );

    glDepthFunc( DepthFuncConversionTable[func] );
}

void Renderer_OpenGL::SetCullingEnabled(bool enabled)
{
    Renderer_Base::SetCullingEnabled( enabled );

    if( enabled )
        glEnable( GL_CULL_FACE );
    else
        glDisable( GL_CULL_FACE );

    checkGlError( "glEnable or glDisable( GL_CULL_FACE )" );
}

void Renderer_OpenGL::SetFrontFaceWinding(MyRE::FrontFaceWindings winding)
{
    Renderer_Base::SetFrontFaceWinding( winding );

    glFrontFace( FrontFaceWindingConversionTable[winding] );
}

void Renderer_OpenGL::SetSwapInterval(int32 interval)
{
    Renderer_Base::SetSwapInterval( interval );

    if( wglSwapInterval )
        wglSwapInterval( interval );

    checkGlError( "wglSwapInterval" );
}

void Renderer_OpenGL::SetBlendEnabled(bool enabled)
{
    Renderer_Base::SetBlendEnabled( enabled );

    if( enabled )
        glEnable( GL_BLEND );
    else
        glDisable( GL_BLEND );

    checkGlError( "glEnable or glDisable( GL_BLEND )" );
}

void Renderer_OpenGL::SetBlendFunc(MyRE::BlendFactors srcFactor, MyRE::BlendFactors dstFactor)
{
    Renderer_Base::SetBlendFunc( srcFactor, dstFactor );

    glBlendFunc( BlendFactorConversionTable[srcFactor], BlendFactorConversionTable[dstFactor] );

    checkGlError( "glBlendFunc" );
}

void Renderer_OpenGL::SetLineWidth(float width)
{
    Renderer_Base::SetLineWidth( width );

    glLineWidth( width );
}

//====================================================================================================
// Actions.
//====================================================================================================
void Renderer_OpenGL::ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil)
{
    GLbitfield flags = 0;
    if( clearColor )   flags |= GL_COLOR_BUFFER_BIT;
    if( clearDepth )   flags |= GL_DEPTH_BUFFER_BIT;
    if( clearStencil ) flags |= GL_STENCIL_BUFFER_BIT;

    glClear( flags );

    checkGlError( "glClear" );
}

void Renderer_OpenGL::ClearScissorRegion()
{
    glDisable( GL_SCISSOR_TEST );

    checkGlError( "glDisable( GL_SCISSOR_TEST )" );
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

    checkGlError( "glViewport" );
}

void Renderer_OpenGL::BufferData(BufferDefinition* pBuffer, GLuint bufferID, uint32 sizeInBytes, void* pData)
{
    GLenum target = BufferTypeConversionTable[pBuffer->m_BufferType];
    GLenum usage = BufferUsageConversionTable[pBuffer->m_BufferUsage];

    MyBindBuffer( target, bufferID );
    checkGlError( "MyBindBuffer" );

    glBufferData( target, sizeInBytes, pData, usage );
    checkGlError( "glBufferData" );
}

void Renderer_OpenGL::BufferSubData(BufferDefinition* pBuffer, GLuint bufferID, uint32 offset, uint32 sizeInBytes, void* pData)
{
    GLenum target = BufferTypeConversionTable[pBuffer->m_BufferType];

    MyBindBuffer( target, bufferID );
    checkGlError( "MyBindBuffer" );

    glBufferSubData( target, offset, sizeInBytes, pData );
    checkGlError( "glBufferSubData" );
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

void Renderer_OpenGL::DrawArrays(MyRE::PrimitiveTypes mode, GLint first, GLsizei count, bool hideFromDrawList)
{
    MyAssert( mode < MyRE::PrimitiveType_Undefined );

    bool draw = ShouldDraw( hideFromDrawList );

    if( draw )
    {
        checkGlError( "glDrawArrays Before" );
        glDrawArrays( PrimitiveTypeConversionTable[mode], first, count );
        checkGlError( "glDrawArrays After" );
    }

    if( hideFromDrawList == false )
    {
        g_GLStats.m_NumDrawCallsThisFrameSoFar++;
    }
}

void Renderer_OpenGL::DrawElements(MyRE::PrimitiveTypes mode, GLsizei count, MyRE::IndexTypes IBOType, const GLvoid* indices, bool hideFromDrawList)
{
    MyAssert( mode < MyRE::PrimitiveType_Undefined );

    bool draw = ShouldDraw( hideFromDrawList );
    
    if( draw )
    {
        checkGlError( "glDrawElements Before" );
        glDrawElements( PrimitiveTypeConversionTable[mode], count, IndexTypeConversionTable[IBOType], indices );
        checkGlError( "glDrawElements After" );
    }

    if( hideFromDrawList == false )
    {
        g_GLStats.m_NumDrawCallsThisFrameSoFar++;
    }
}

void Renderer_OpenGL::ReadPixels(int x, int y, uint32 width, uint32 height, MyRE::PixelFormats format, MyRE::PixelDataTypes dataType, void* buffer)
{
    glReadPixels( x, y, width, height, PixelFormatConversionTable[format], PixelDataTypeConversionTable[dataType], buffer );
}

void Renderer_OpenGL::SetPolygonMode(MyRE::PolygonDrawModes mode)
{
    glPolygonMode( GL_FRONT_AND_BACK, PolygonDrawModeConversionTable[mode] );
}

void Renderer_OpenGL::SetPolygonOffset(bool enabled, float factor, float units)
{
    if( enabled )
    {
        glEnable( GL_POLYGON_OFFSET_LINE );
        glEnable( GL_POLYGON_OFFSET_FILL ); // Enabling GL_POLYGON_OFFSET_LINE doesn't work on my intel 4000.
    }
    else
    {
        glDisable( GL_POLYGON_OFFSET_FILL );
        glDisable( GL_POLYGON_OFFSET_LINE );
    }

    glPolygonOffset( factor, units );
}

//====================================================================================================
// Textures/FBOs.
//====================================================================================================
void Renderer_OpenGL::SetTextureMinMagFilters(GLuint texture, MyRE::MinFilters min, MyRE::MagFilters mag)
{
    MyAssert( texture != 0 );
    MyAssert( min < MyRE::MinFilter_NumTypes );
    MyAssert( mag < MyRE::MagFilter_NumTypes );

    // Note: This does not preserve the current texture bindings.
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MinFilterConversionTable[min] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MagFilterConversionTable[mag] );
    glBindTexture( GL_TEXTURE_2D, 0 );

    checkGlError( "SetTextureMinMagFilters" );
}

void Renderer_OpenGL::SetTextureWrapModes(GLuint texture, MyRE::WrapModes wrapModeS, MyRE::WrapModes wrapModeT)
{
    MyAssert( texture != 0 );
    MyAssert( wrapModeS < MyRE::WrapMode_NumTypes );
    MyAssert( wrapModeT < MyRE::WrapMode_NumTypes );

    // Note: This does not preserve the current texture bindings.
    glBindTexture( GL_TEXTURE_2D, texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapModeConversionTable[wrapModeS] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapModeConversionTable[wrapModeT] );
    glBindTexture( GL_TEXTURE_2D, 0 );

    checkGlError( "SetTextureWrapMode" );
}

void Renderer_OpenGL::BindFramebuffer(GLuint framebuffer)
{
    MyBindFramebuffer( GL_FRAMEBUFFER, framebuffer, 0, 0 );
}
