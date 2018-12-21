//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
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
    bool draw = ShouldDraw( hideFromDrawList );

    if( draw )
    {
        checkGlError( "glDrawArrays Before" );
        glDrawArrays( mode, first, count );
        checkGlError( "glDrawArrays After" );
    }

    if( hideFromDrawList == false )
    {
        g_GLStats.m_NumDrawCallsThisFrameSoFar++;
    }
}

void Renderer_OpenGL::DrawElements(MyRE::PrimitiveTypes mode, GLsizei count, MyRE::IndexTypes IBOType, const GLvoid* indices, bool hideFromDrawList)
{
    bool draw = ShouldDraw( hideFromDrawList );
    
    if( draw )
    {
        checkGlError( "glDrawElements Before" );
        glDrawElements( mode, count, IBOType, indices );
        checkGlError( "glDrawElements After" );
    }

    if( hideFromDrawList == false )
    {
        g_GLStats.m_NumDrawCallsThisFrameSoFar++;
    }
}
