//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "OpenGLWrapper.h"
#include "../BaseClasses/Renderer_Enums.h"

// TODO: Fix GL Includes.
#include <gl/GL.h>
#include "../SourceWindows/OpenGL/GLExtensions.h"
#include "../OpenGL/GLHelpers.h"

GLStats g_GLStats;

//static GLuint g_CurrentGLBuffers[2] = { 0, 0 };
static GLenum g_CurrentGLActiveTextureUnit = -1;
static GLuint g_CurrentProgram = 0;

GLStats::GLStats()
{
    m_NumFramesDrawn = 0;
    m_TotalDrawCalls = 0;
    m_NumDrawCallsThisFrameSoFar = 0;
    for( int i=0; i<10; i++ )
        m_NumDrawCallsLastFrame[i] = 0;
    m_EvenOddFrame = 0;
    
    m_DrawCallLimit_Canvas = -1;
    m_DrawCallLimit_Index = -1;
    m_DrawCallLimit_BreakPointIndex = -1;

    m_CurrentCanvasID = 0;

    m_PreviousFramebuffer = 0;
    m_PreviousFramebufferWidth = 0;
    m_PreviousFramebufferHeight = 0;

    m_CurrentFramebuffer = 0;
    m_CurrentFramebufferWidth = 0;
    m_CurrentFramebufferHeight = 0;
}

GLStats::~GLStats()
{
}

void GLStats::NewFrame()
{
    m_NumFramesDrawn++;
    
    m_EvenOddFrame++;
    if( m_EvenOddFrame == 2 )
        m_EvenOddFrame = 0;
}

void GLStats::EndFrame()
{
}

void GLStats::NewCanvasFrame(unsigned int canvasid)
{
    m_CurrentCanvasID = canvasid;
    m_NumDrawCallsThisFrameSoFar = 0;
}

void GLStats::EndCanvasFrame()
{
#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 0 )
        g_pPanelMemory->m_DrawCallListDirty = false;
#endif

    if( m_CurrentCanvasID < 10 )
        m_NumDrawCallsLastFrame[m_CurrentCanvasID] = m_NumDrawCallsThisFrameSoFar;
}

int GLStats::GetNumDrawCallsLastFrameForCurrentCanvasID()
{
    return m_NumDrawCallsLastFrame[m_CurrentCanvasID];
}

void MyBindBuffer(GLenum target, GLuint buffer)
{
    MyAssert( target >= GL_ARRAY_BUFFER && target <= GL_ELEMENT_ARRAY_BUFFER );

    // TODO: forget storing this for now... VAOs are messing the the values and breaking things, will need to rethink it.
    //if( g_CurrentGLBuffers[target - GL_ARRAY_BUFFER] == buffer )
    //    return;

    //g_CurrentGLBuffers[target - GL_ARRAY_BUFFER] = buffer;
    glBindBuffer( target, buffer );
}

void MyDeleteBuffers(GLsizei num, GLuint* buffers)
{
    MyAssert( num > 0 );

    for( int i=0; i<num; i++ )
    {
        // TODO: fix along with MyBindBuffer
        //if( g_CurrentGLBuffers[0] == buffers[i] ) g_CurrentGLBuffers[0] = 0;
        //if( g_CurrentGLBuffers[1] == buffers[i] ) g_CurrentGLBuffers[1] = 0;

        glDeleteBuffers( 1, &buffers[i] );
    }
}

void MyActiveTexture(GLenum texture)
{
    if( g_CurrentGLActiveTextureUnit == texture )
        return;

    g_CurrentGLActiveTextureUnit = texture;
    glActiveTexture( texture );
}

void MyUseProgram(GLuint program)
{
    g_CurrentProgram = program;

    glUseProgram( program );
}

void MyEnableVertexAttribArray(GLuint index)
{
    glEnableVertexAttribArray( index );
}

void MyDisableVertexAttribArray(GLuint index)
{
    glDisableVertexAttribArray( index );
}

void MyBindFramebuffer(GLenum target, GLuint framebuffer, unsigned int fbwidth, unsigned int fbheight)
{
    if( g_GLStats.m_CurrentFramebuffer == framebuffer )
        return;

    checkGlError( "before glBindFramebuffer" );

    glBindFramebuffer( target, framebuffer );

    checkGlError( "after glBindFramebuffer" );

    g_GLStats.m_PreviousFramebuffer = g_GLStats.m_CurrentFramebuffer;
    g_GLStats.m_PreviousFramebufferWidth = g_GLStats.m_CurrentFramebufferWidth;
    g_GLStats.m_PreviousFramebufferHeight = g_GLStats.m_CurrentFramebufferHeight;

    g_GLStats.m_CurrentFramebuffer = framebuffer;
    g_GLStats.m_CurrentFramebufferWidth = fbwidth;
    g_GLStats.m_CurrentFramebufferHeight = fbheight;
}

bool MyGLDebug_IsProgramActive(GLuint program)
{
    return g_CurrentProgram == program;
}
