//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Renderer_Base_H__
#define __Renderer_Base_H__

class Renderer_Base;
extern Renderer_Base* g_pRenderer;

class Renderer_Base
{
protected:
    bool m_IsValid;

    uint32 m_WindowStartX;
    uint32 m_WindowStartY;
    uint32 m_WindowWidth;
    uint32 m_WindowHeight;

    ColorFloat m_ClearColor;
    float m_ClearDepth;

public:
    Renderer_Base();
    virtual ~Renderer_Base();

    // Getters.
    bool IsValid() { return m_IsValid; }
    uint32 GetWindowStartX() { return m_WindowStartX; }
    uint32 GetWindowStartY() { return m_WindowStartY; }
    uint32 GetWindowWidth() { return m_WindowWidth; }
    uint32 GetWindowHeight() { return m_WindowHeight; }

    // Setters.
    void SetWindowWidth(uint32 newWidth) { m_WindowWidth = newWidth; }
    void SetWindowHeight(uint32 newHeight) { m_WindowHeight = newHeight; }

    // Events.
    virtual void OnSurfaceCreated();
    virtual void OnSurfaceChanged(unsigned int startX, unsigned int startY, unsigned int width, unsigned int height);
    virtual void OnSurfaceLost();

    // Actions.
    virtual void SetClearColor(ColorFloat color);
    virtual void SetClearDepth(float depth);
    virtual void ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil) = 0;

    virtual void DrawElements(MyRE::PrimitiveTypes mode, GLsizei count, MyRE::IndexTypes IBOType, const GLvoid* indices, bool hideFromDrawList) = 0;
    virtual void DrawArrays(MyRE::PrimitiveTypes mode, GLint first, GLsizei count, bool hideFromDrawList) = 0;
};

#endif //__Renderer_Base_H__
