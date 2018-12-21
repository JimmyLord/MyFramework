//
// Copyright (c) 2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Renderer_OpenGL_H__
#define __Renderer_OpenGL_H__

#include "../Renderer_Base.h"

class Renderer_OpenGL : public Renderer_Base
{
protected:
public:
    Renderer_OpenGL();
    virtual ~Renderer_OpenGL();

    // Getters.

    // Events.
    virtual void OnSurfaceCreated() override;
    virtual void OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height) override;
    //virtual void OnSurfaceLost() override;

    // State Change.
    virtual void SetClearColor(ColorFloat color) override;
    virtual void SetClearDepth(float depth) override;
    virtual void SetDepthWriteEnabled(bool enabled) override;
    virtual void SetDepthTestEnabled(bool enabled) override;

    // Actions.
    virtual void ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil) override;
    virtual void ClearScissorRegion() override;
    virtual void EnableViewport(MyViewport* pViewport, bool enableOrDisableScissorIfNeeded) override;

    virtual void DrawElements(MyRE::PrimitiveTypes mode, GLsizei count, MyRE::IndexTypes IBOType, const GLvoid* indices, bool hideFromDrawList) override;
    virtual void DrawArrays(MyRE::PrimitiveTypes mode, GLint first, GLsizei count, bool hideFromDrawList) override;
};

#endif //__Renderer_OpenGL_H__
