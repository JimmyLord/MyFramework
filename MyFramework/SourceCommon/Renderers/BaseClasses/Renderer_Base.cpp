//
// Copyright (c) 2018-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "Renderer_Enums.h"
#include "Renderer_Base.h"

Renderer_Base* g_pRenderer = 0;

Renderer_Base::Renderer_Base()
{
    g_pRenderer = this;

    m_IsValid = false;

    m_ClearColor.Set( 0.0f, 0.0f, 0.2f, 0.0f );
    m_ClearDepth = 1.0f;

    m_DepthWriteEnabled = false;
    m_DepthTestEnabled = false;
    m_DepthFunc = MyRE::DepthFunc_Less;

    m_CullingEnabled = false;
    m_CullMode = MyRE::CullMode_Back;
    m_FrontFaceWinding = MyRE::FrontFaceWinding_CounterClockwise;

    m_SwapInterval = 1;

    m_BlendEnabled = false;
    m_BlendFunc_SrcFactor = MyRE::BlendFactor_One;
    m_BlendFunc_DstFactor = MyRE::BlendFactor_Zero;

    m_LineWidth = 1.0f;
    m_PointSize = 1.0f;
}

Renderer_Base::~Renderer_Base()
{
}

//====================================================================================================
// Events.
//====================================================================================================
void Renderer_Base::OnSurfaceCreated()
{
    LOGInfo( LOGTag, "[Flow] onSurfaceCreated()\n" );

    m_IsValid = true;
}

void Renderer_Base::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height)
{
}

void Renderer_Base::OnSurfaceLost()
{
    LOGInfo( LOGTag, "[Flow] onSurfaceLost()\n" );

    m_IsValid = false;
}

//====================================================================================================
// State Change.
//====================================================================================================
void Renderer_Base::SetClearColor(ColorFloat color)
{
    m_ClearColor = color;
}

void Renderer_Base::SetClearDepth(float depth)
{
    m_ClearDepth = depth;
}

void Renderer_Base::SetDepthWriteEnabled(bool enabled)
{
    m_DepthWriteEnabled = enabled;
}

void Renderer_Base::SetDepthTestEnabled(bool enabled)
{
    m_DepthTestEnabled = enabled;
}

void Renderer_Base::SetDepthFunction(MyRE::DepthFuncs func)
{
    m_DepthFunc = func;
}

void Renderer_Base::SetCullingEnabled(bool enabled)
{
    m_CullingEnabled = enabled;
}

void Renderer_Base::SetCullMode(MyRE::CullModes mode)
{
    m_CullMode = mode;
}

void Renderer_Base::SetFrontFaceWinding(MyRE::FrontFaceWindings winding)
{
    m_FrontFaceWinding = winding;
}

void Renderer_Base::SetSwapInterval(int32 interval)
{
    m_SwapInterval = interval;
}

void Renderer_Base::SetBlendEnabled(bool enabled)
{
    m_BlendEnabled = enabled;
}

void Renderer_Base::SetBlendFunc(MyRE::BlendFactors srcFactor, MyRE::BlendFactors dstFactor)
{
    m_BlendFunc_SrcFactor = srcFactor;
    m_BlendFunc_DstFactor = dstFactor;
}

void Renderer_Base::SetLineWidth(float width)
{
    m_LineWidth = width;
}

void Renderer_Base::SetPointSize(float size)
{
    m_PointSize = size;
}
