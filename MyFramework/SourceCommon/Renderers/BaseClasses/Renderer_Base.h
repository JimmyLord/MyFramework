//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
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

class MyViewport
{
protected:
    uint32 m_X; // Bottom Left X,Y.
    uint32 m_Y;
    uint32 m_Width; // Width and Height of viewport.
    uint32 m_Height;

public:
    MyViewport() { Set( 0, 0, 0, 0 ); }
    MyViewport(uint32 x, uint32 y, uint32 w, uint32 h) { Set( x, y, w, h ); }

    // Getters.
    uint32 GetX() { return m_X; }
    uint32 GetY() { return m_Y; }
    uint32 GetWidth() { return m_Width; }
    uint32 GetHeight() { return m_Height; }

    // Setters.
    void Set(uint32 x, uint32 y, uint32 w, uint32 h) { m_X = x; m_Y = y; m_Width = w; m_Height = h; }
    void SetX(uint32 x) { m_X = x; }
    void SetY(uint32 y) { m_Y = y; }
    void SetWidth(uint32 w) { m_Width = w; }
    void SetHeight(uint32 h) { m_Height = h; }
};

class Renderer_Base
{
protected:
    bool m_IsValid;

    ColorFloat m_ClearColor;
    float m_ClearDepth;

    bool m_DepthWriteEnabled;
    bool m_DepthTestEnabled;
    MyRE::DepthFuncs m_DepthFunc;

    bool m_CullingEnabled;
    MyRE::FrontFaceWindings m_FrontFaceWinding;
    MyRE::CullModes m_CullMode;

    int32 m_SwapInterval;

    bool m_BlendEnabled;
    MyRE::BlendFactors m_BlendFunc_SrcFactor;
    MyRE::BlendFactors m_BlendFunc_DstFactor;

    float m_LineWidth;
    float m_PointSize;

public:
    Renderer_Base();
    virtual ~Renderer_Base();

    // Getters.
    bool IsValid() { return m_IsValid; }

    // Setters.

    // Events.
    virtual void OnSurfaceCreated();
    virtual void OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height);
    virtual void OnSurfaceLost();

    // State Change.
    virtual void SetClearColor(ColorFloat color);
    virtual void SetClearDepth(float depth);
    virtual void SetDepthWriteEnabled(bool enabled);
    virtual void SetDepthTestEnabled(bool enabled);
    virtual void SetDepthFunction(MyRE::DepthFuncs func);
    virtual void SetCullingEnabled(bool enabled);
    virtual void SetCullMode(MyRE::CullModes mode);
    virtual void SetFrontFaceWinding(MyRE::FrontFaceWindings winding);
    virtual void SetSwapInterval(int32 interval);
    virtual void SetBlendEnabled(bool enabled);
    virtual void SetBlendFunc(MyRE::BlendFactors srcFactor, MyRE::BlendFactors dstFactor);
    virtual void SetLineWidth(float width);
    virtual void SetPointSize(float size);

    // Actions.
    virtual Shader_Base* CreateShader(ShaderPassTypes passType) = 0;

    virtual void ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil) = 0;
    virtual void ClearScissorRegion() = 0;
    virtual void EnableViewport(MyViewport* pViewport, bool enableOrDisableScissorIfNeeded) = 0;

    virtual void BufferData(Buffer_Base* pBuffer, GLuint bufferID, uint32 sizeInBytes, void* pData) = 0;
    virtual void BufferSubData(Buffer_Base* pBuffer, GLuint bufferID, uint32 offset, uint32 sizeInBytes, void* pData) = 0;

    virtual void DrawElements(MyRE::PrimitiveTypes mode, GLsizei count, MyRE::IndexTypes IBOType, const GLvoid* indices, bool hideFromDrawList) = 0;
    virtual void DrawArrays(MyRE::PrimitiveTypes mode, GLint first, GLsizei count, bool hideFromDrawList) = 0;
    virtual void TempHack_SetupAndDrawInstanced(Shader_Base* pShader, uint32 numInstancesToDraw) = 0;
    virtual void TempHack_UnbindVBOAndIBO() = 0;

    virtual void ReadPixels(int x, int y, uint32 width, uint32 height, MyRE::PixelFormats format, MyRE::PixelDataTypes dataType, void* buffer) = 0;

    virtual void SetPolygonMode(MyRE::PolygonDrawModes mode) = 0;
    virtual void SetPolygonOffset(bool enabled, float factor, float units) = 0;

    // Textures/FBOs.
    virtual void SetTextureMinMagFilters(GLuint texture, MyRE::MinFilters min, MyRE::MagFilters mag) = 0;
    virtual void SetTextureWrapModes(GLuint texture, MyRE::WrapModes wrapModeS, MyRE::WrapModes wrapModeT) = 0;
    virtual void BindFramebuffer(GLuint framebuffer) = 0;
};

#endif //__Renderer_Base_H__
