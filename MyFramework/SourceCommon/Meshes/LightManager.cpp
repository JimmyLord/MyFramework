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
    assert( pLight );

    pLight->Remove();
    delete pLight;
}

int LightManager::FindNearestLights(int numtofind, MyLight** ppLightArray)
{
    assert( numtofind > 0 );

    int numfound = 0;

    for( CPPListNode* pNode = m_LightList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        MyLight* pLight = (MyLight*)pNode;

        // TODO: find nearby lights, return first lights for now.
        m_LightArray[numfound] = *pLight;
        numfound++;

        if( numfound >= numtofind )
            break;
    }

    *ppLightArray = m_LightArray;

    return numfound;
}
