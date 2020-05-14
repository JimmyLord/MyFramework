//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "Box2DDebugDraw.h"
#include "Box2DContactListener.h"
#include "Box2DWorld.h"
#include "../../../Libraries/Box2D/include/Box2D/Box2D.h"

Box2DWorld::Box2DWorld(MaterialDefinition* debugdrawmaterial, MyMatrix* pMatProj, MyMatrix* pMatView, Box2DContactListener* pContactListener)
{
    m_pWorld = 0;
    m_pDebugDraw = 0;
    m_pContactListener = 0;
    m_pGround = 0;

    CreateWorld( debugdrawmaterial, pMatProj, pMatView, pContactListener );
}

Box2DWorld::~Box2DWorld()
{
    Cleanup();
}

void Box2DWorld::CreateWorld(MaterialDefinition* debugdrawmaterial, MyMatrix* pMatProj, MyMatrix* pMatView, Box2DContactListener* pContactListener)
{
    MyAssert( m_pWorld == 0 );
    m_pWorld = MyNew b2World( b2Vec2( 0, -10 ) );

    // Setup debug draw object.
    if( debugdrawmaterial != 0 )
    {
        m_pDebugDraw = MyNew Box2DDebugDraw( debugdrawmaterial, pMatProj, pMatView );
        m_pWorld->SetDebugDraw( m_pDebugDraw );

        uint32 flags = b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_centerOfMassBit | b2Draw::e_aabbBit | b2Draw::e_pairBit;
        m_pDebugDraw->SetFlags( flags );
    }

    // Setup contact listener object.
    {
        if( pContactListener == 0 )
            m_pContactListener = MyNew Box2DContactListener();
        else
            m_pContactListener = pContactListener;

        m_pWorld->SetContactListener( m_pContactListener );
    }

    // Create a static ground object
    {
        b2BodyDef bodydef;
        bodydef.position = b2Vec2( 0, 0 );
        bodydef.type = b2_staticBody;

        m_pGround = m_pWorld->CreateBody( &bodydef );
    }
}

void Box2DWorld::PhysicsStep()
{
    m_pWorld->Step( 1.0f/60.0f, 10, 1 );
}

void Box2DWorld::Cleanup()
{
    m_pGround = 0;

    SAFE_DELETE( m_pWorld );
    SAFE_DELETE( m_pDebugDraw );
    SAFE_DELETE( m_pContactListener );
}
