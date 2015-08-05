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

GLStats g_GLStats;

//static GLuint g_CurrentGLBuffers[2] = { 0, 0 };
static GLenum g_CurrentGLActiveTextureUnit = -1;

GLStats::GLStats()
{
    m_NumFramesDrawn = 0;
    m_TotalDrawCalls = 0;
    m_NumDrawCallsThisFrameSoFar = 0;
    m_NumDrawCallsLastFrame = 0;
    m_EvenOddDrawCall = 0;
}

GLStats::~GLStats()
{
}

void GLStats::NewFrame()
{
    m_NumFramesDrawn++;
    m_NumDrawCallsLastFrame = m_NumDrawCallsThisFrameSoFar;
    m_NumDrawCallsThisFrameSoFar = 0;
    
    m_EvenOddDrawCall++;
    if( m_EvenOddDrawCall == 2 )
        m_EvenOddDrawCall = 0;
}

void GLStats::EndFrame()
{
#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 0 )
        g_pPanelMemory->m_DrawCallListDirty = false;
#endif
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

void MyDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    g_GLStats.m_NumDrawCallsThisFrameSoFar++;

#if MYFW_USING_WX
    if( g_pPanelMemory->m_DrawCallIndexToDraw == -1 || g_pPanelMemory->m_DrawCallIndexToDraw == g_GLStats.m_NumDrawCallsThisFrameSoFar )
#endif
    {
        checkGlError( "glDrawElements Before" );
        glDrawElements( mode, count, type, indices );
        checkGlError( "glDrawElements After" );
    }

#if MYFW_USING_WX
    if( g_GLCanvasIDActive == 0 && g_pPanelMemory->m_DrawCallListDirty == true )
    {
        g_pPanelMemory->AddDrawCall( g_GLStats.m_NumDrawCallsThisFrameSoFar, "Global", "draw" );
    }
#endif
}

void MyDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    g_GLStats.m_NumDrawCallsThisFrameSoFar++;

    glDrawArrays( mode, first, count );
    //checkGlError( "glDrawArrays" );
}

void MyEnableVertexAttribArray(GLuint index)
{
    glEnableVertexAttribArray( index );
}

void MyDisableVertexAttribArray(GLuint index)
{
    glDisableVertexAttribArray( index );
}
