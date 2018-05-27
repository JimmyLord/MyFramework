//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __OpenGLWrapper_H__
#define __OpenGLWrapper_H__

class GLStats
{
public:
    int m_NumFramesDrawn;
    int m_TotalDrawCalls;
    int m_NumDrawCallsThisFrameSoFar;
    int m_NumDrawCallsLastFrame[10]; // store for up to canvasid's up to 10.
    int m_EvenOddFrame;
    
    // Variables used by draw call debugger.
    int m_DrawCallLimit_Canvas;
    int m_DrawCallLimit_Index;
    int m_DrawCallLimit_BreakPointIndex;

    unsigned int m_CurrentCanvasID;

    unsigned int m_PreviousFramebuffer;
    unsigned int m_PreviousFramebufferWidth;
    unsigned int m_PreviousFramebufferHeight;

    unsigned int m_CurrentFramebuffer;
    unsigned int m_CurrentFramebufferWidth;
    unsigned int m_CurrentFramebufferHeight;

public:
    GLStats();
    ~GLStats();

    void NewFrame();
    void EndFrame();

    void NewCanvasFrame(unsigned int canvasid);
    void EndCanvasFrame();

    int GetNumDrawCallsLastFrameForCurrentCanvasID();
};

extern GLStats g_GLStats;

//glPolygonOffset
//glBlendFunc
//glCullFace
//glDepthMask
//glFrontFace
void MyBindBuffer(GLenum target, GLuint buffer);
void MyDeleteBuffers(GLsizei num, GLuint* buffers);
void MyActiveTexture(GLenum texture);
void MyDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, bool hideFromDrawList);
void MyDrawArrays(GLenum mode, GLint first, GLsizei count, bool hideFromDrawList);
void MyUseProgram(GLuint program);

void MyEnableVertexAttribArray(GLuint index);
void MyDisableVertexAttribArray(GLuint index);

void MyBindFramebuffer(GLenum target, GLuint framebuffer, unsigned int fbwidth, unsigned int fbheight);

bool MyGLDebug_IsProgramActive(GLuint program);

#endif //__OpenGLWrapper_H__
