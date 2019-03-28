//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "LightManager.h"

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}

MyLight* LightManager::CreateLight()
{
    // TODO: Make a pool of lights, so we don't alloc/free all the time.
    MyLight* pLight = MyNew MyLight();

    m_LightList.AddTail( pLight );

    return pLight;
}

void LightManager::DestroyLight(MyLight* pLight)
{
    MyAssert( pLight );

    pLight->Remove();
    delete pLight;
}

void LightManager::SetLightEnabled(MyLight* pLight, bool enabled)
{
    MyAssert( pLight );
    if( pLight == nullptr )
        return;

    if( enabled )
    {
        MyAssert( m_LightList.GetTail() != pLight );
        m_LightList.MoveTail( pLight );
    }
    else
    {
        MyAssert( m_DisabledLightList.GetTail() != pLight );
        m_DisabledLightList.MoveTail( pLight );
    }
}

int LightManager::FindNearestLights(LightTypes type, int numToFind, Vector3 pos, MyLight** ppLights)
{
    MyAssert( numToFind > 0 );
    MyAssert( ppLights != nullptr );
    MyAssert( numToFind < MAX_LIGHTS_TO_FIND );

    if( numToFind > MAX_LIGHTS_TO_FIND )
        numToFind = MAX_LIGHTS_TO_FIND;

    // TODO: Store lights in scene graph and cache the nearest lights between frames.

    float distances[MAX_LIGHTS_TO_FIND];
    float furthest = FLT_MAX;
    for( int i=0; i<numToFind; i++ )
    {
        distances[i] = FLT_MAX;
        ppLights[i] = nullptr;
    }

    // Find nearest lights, based purely on distance from center.
    // TODO: Take brightness into account.
    for( MyLight* pLight = m_LightList.GetHead(); pLight; pLight = pLight->GetNext() )
    {
        if( pLight->m_LightType != type )
            continue;

        float distance = (pLight->m_Position - pos).LengthSquared();
        if( distance < furthest )
        {
            for( int i=0; i<numToFind; i++ )
            {
                if( distance < distances[i] )
                {
                    for( int j=numToFind-1; j>i; j-- )
                    {
                        distances[j] = distances[j-1];
                        ppLights[j] = ppLights[j-1];
                    }

                    furthest = distances[numToFind-1];

                    distances[i] = distance;
                    ppLights[i] = pLight;
                    
                    break;
                }
            }
        }
    }

    int numFound = 0;
    for( int i=0; i<numToFind; i++ )
    {
        if( ppLights[i] == nullptr )
            break;

        numFound++;
    }

    return numFound;
}
