//
// Copyright (c) 2015-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

LightManager* g_pLightManager = 0;

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}

MyLight* LightManager::CreateLight()
{
    // TODO: make a pool of lights, so we don't alloc/free all the time.
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
    if( pLight == 0 )
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

int LightManager::FindNearestLights(LightTypes type, int numtofind, Vector3 pos, MyLight** ppLights)
{
    MyAssert( numtofind > 0 );
    MyAssert( ppLights != 0 );
    MyAssert( numtofind < MAX_LIGHTS_TO_FIND );

    if( numtofind > MAX_LIGHTS_TO_FIND )
        numtofind = MAX_LIGHTS_TO_FIND;

    // TODO: store lights in scene graph and cache the nearest lights between frames.

    float distances[MAX_LIGHTS_TO_FIND];
    float furthest = FLT_MAX;
    for( int i=0; i<numtofind; i++ )
    {
        distances[i] = FLT_MAX;
        ppLights[i] = 0;
    }

    // find nearest lights, based purely on distance from center.
    // TODO: take brightness into account.
    for( CPPListNode* pNode = m_LightList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyLight* pLight = (MyLight*)pNode;

        if( pLight->m_LightType != type )
            continue;

        float distance = (pLight->m_Position - pos).LengthSquared();
        if( distance < furthest )
        {
            for( int i=0; i<numtofind; i++ )
            {
                if( distance < distances[i] )
                {
                    for( int j=numtofind-1; j>i; j-- )
                    {
                        distances[j] = distances[j-1];
                        ppLights[j] = ppLights[j-1];
                    }

                    furthest = distances[numtofind-1];

                    distances[i] = distance;
                    ppLights[i] = pLight;
                    
                    break;
                }
            }
        }
    }

    int numfound = 0;
    for( int i=0; i<numtofind; i++ )
    {
        if( ppLights[i] == 0 )
            break;

        numfound++;
    }

    return numfound;
}
