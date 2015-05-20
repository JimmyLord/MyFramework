//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
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
    //for( int i=0; i<8; i++ )
    //    m_LightArray[i];
}

LightManager::~LightManager()
{
}

MyLight* LightManager::CreateLight()
{
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

int LightManager::FindNearestLights(int numtofind, Vector3 pos, MyLight** ppLightArray)
{
    MyAssert( numtofind > 0 );

    if( numtofind > MAX_LIGHTS )
        numtofind = MAX_LIGHTS;

    // TODO: store lights in a quad-tree and maybe cache the nearest lights between frames.

    float distances[MAX_LIGHTS];
    MyLight* lights[MAX_LIGHTS];
    float furthest = 10000;
    for( int i=0; i<MAX_LIGHTS; i++ )
    {
        distances[i] = 10000;
        lights[i] = 0;
    }

    // find nearest lights, based purely on distance from center.
    // TODO: take brightness into account.
    for( CPPListNode* pNode = m_LightList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyLight* pLight = (MyLight*)pNode;

        float distance = (pLight->m_Position - pos).LengthSquared();
        if( distance < furthest )
        {
            for( int i=0; i<numtofind; i++ )
            {
                if( distance < distances[i] )
                {
                    for( int j=4-1; j>i; j-- )
                    {
                        distances[j] = distances[j-1];
                        lights[j] = lights[j-1];
                    }

                    furthest = distances[numtofind];

                    distances[i] = distance;
                    lights[i] = pLight;
                    
                    break;
                }
            }
        }
    }

    int numfound = 0;
    for( int i=0; i<numtofind; i++ )
    {
        if( lights[i] == 0 )
            break;

        m_LightArray[numfound] = *lights[i];
        numfound++;
    }

    *ppLightArray = m_LightArray;

    return numfound;
}
