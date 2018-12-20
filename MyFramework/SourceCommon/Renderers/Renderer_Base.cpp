//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
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
// Actions.
//====================================================================================================
void Renderer_Base::SetClearColor(ColorFloat color)
{
    m_ClearColor = color;
}

void Renderer_Base::SetClearDepth(float depth)
{
    m_ClearDepth = depth;
}
