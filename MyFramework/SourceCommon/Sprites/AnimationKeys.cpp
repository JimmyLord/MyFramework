//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "AnimationKeys.h"

AnimationKeyPool* g_pAnimationKeyPool = 0;

AnimationKeyPool::AnimationKeyPool()
: m_TranslationKeys(100)
, m_ScaleKeys(100)
, m_AlphaKeys(100)
, m_RotationKeys(100)
{
    unsigned int i;
    for( i=0; i<m_TranslationKeys.Length(); i++ )
    {
        m_TranslationKeys.AddInactiveObject( MyNew KeyTranslation );
    }
    for( i=0; i<m_ScaleKeys.Length(); i++ )
    {
        m_ScaleKeys.AddInactiveObject( MyNew KeyScale );
    }
    for( i=0; i<m_AlphaKeys.Length(); i++ )
    {
        m_AlphaKeys.AddInactiveObject( MyNew KeyAlpha );
    }
    for( i=0; i<m_RotationKeys.Length(); i++ )
    {
        m_RotationKeys.AddInactiveObject( MyNew KeyRotation );
    }
}

AnimationKeyPool::~AnimationKeyPool()
{
    m_TranslationKeys.DeleteAllObjectsInPool();
    m_ScaleKeys.DeleteAllObjectsInPool();
    m_AlphaKeys.DeleteAllObjectsInPool();
    m_RotationKeys.DeleteAllObjectsInPool();
}
