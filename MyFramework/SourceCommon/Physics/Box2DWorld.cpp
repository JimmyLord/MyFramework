//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "Box2DWorld.h"

Box2DWorld* g_pBox2DWorld = 0;

Box2DWorld::Box2DWorld()
{
    g_pBox2DWorld = this;

    CreateWorld();
}

Box2DWorld::~Box2DWorld()
{
    if( g_pBox2DWorld == this )
        g_pBox2DWorld = 0;

    Cleanup();
}

void Box2DWorld::CreateWorld()
{
    m_pWorld = MyNew b2World( b2Vec2( 0, -10 ) );
}

void Box2DWorld::PhysicsStep()
{
    m_pWorld->Step( 1.0f/60.0f, 10, 1 );
}

void Box2DWorld::Cleanup()
{
    // delete world
    delete m_pWorld;
}
