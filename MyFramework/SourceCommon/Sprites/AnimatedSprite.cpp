//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "AnimatedSprite.h"
#include "AnimationKeys.h"
#include "MySprite.h"
#include "../DataTypes/MyActivePool.h"

AnimatedSpriteDefinition::AnimatedSpriteDefinition(int numframes, int numanchorsperframe)
: m_KeysTranslation(m_MaxKeys)
, m_KeysScale(m_MaxKeys)
, m_KeysAlpha(m_MaxKeys)
, m_KeysRotation(m_MaxKeys)
{
    for( unsigned int i=0; i<m_MaxFrames; i++ )
    {
        m_pSprites[i] = 0;
        m_TimeToHoldFrame[i] = 0;
    }
    m_NumFrames = 0;

    m_TimeOfLastKey = 0;

    m_Looping = false;

    if( numframes*numanchorsperframe == 0 )
        m_pAnchorPoints = 0;
    else
        m_pAnchorPoints = MyNew AnimatedSpriteAnchorPoint[numframes*numanchorsperframe];

    for( int i=0; i<numframes*numanchorsperframe; i++ )
        m_pAnchorPoints[i].Clear();

    m_NumAnchorFrames = numframes;
    m_NumAnchorsPerFrame = numanchorsperframe;
}

AnimatedSpriteDefinition::~AnimatedSpriteDefinition()
{
    for( unsigned int i=0; i<m_KeysTranslation.Count(); i++ )
    {
        KeyTranslation* key = m_KeysTranslation[i];
        if( g_pAnimationKeyPool )
            g_pAnimationKeyPool->m_TranslationKeys.MakeObjectInactive( key );
    }
    m_KeysTranslation.Clear();

    for( unsigned int i=0; i<m_KeysScale.Count(); i++ )
    {
        KeyScale* key = m_KeysScale[i];
        if( g_pAnimationKeyPool )
            g_pAnimationKeyPool->m_ScaleKeys.MakeObjectInactive( key );
    }
    m_KeysScale.Clear();

    for( unsigned int i=0; i<m_KeysAlpha.Count(); i++ )
    {
        KeyAlpha* key = m_KeysAlpha[i];
        if( g_pAnimationKeyPool )
            g_pAnimationKeyPool->m_AlphaKeys.MakeObjectInactive( key );
    }
    m_KeysAlpha.Clear();

    for( unsigned int i=0; i<m_KeysRotation.Count(); i++ )
    {
        KeyRotation* key = m_KeysRotation[i];
        if( g_pAnimationKeyPool )
            g_pAnimationKeyPool->m_RotationKeys.MakeObjectInactive( key );
    }
    m_KeysRotation.Clear();

    SAFE_DELETE_ARRAY( m_pAnchorPoints );
}

void AnimatedSpriteDefinition::SetSpriteForFrame(unsigned int framenum, MySprite* pSprite, double holdtime)
{
    MyAssert( framenum < m_MaxFrames );
    if( framenum >= m_MaxFrames )
        return;

    m_pSprites[framenum] = pSprite;
    m_TimeToHoldFrame[framenum] = holdtime;

    if( framenum >= m_NumFrames )
        m_NumFrames = framenum+1;

    double totaltime = 0; 
    for( unsigned int i=0; i<m_NumFrames; i++ )
    {
        totaltime += m_TimeToHoldFrame[i];
    }

    IncreaseIfBigger( m_TimeOfLastKey, totaltime );
}

void AnimatedSpriteDefinition::SetAnchorsForFrame(unsigned int frame, AnimatedSpriteAnchorPoint* pAnchors, unsigned int numanchors)
{
    MyAssert( numanchors <= m_NumAnchorsPerFrame );

    for( unsigned int i=0; i<numanchors; i++ )
    {
        m_pAnchorPoints[frame*m_NumAnchorsPerFrame + i] = pAnchors[i];
    }
}

void AnimatedSpriteDefinition::SetAnchorsForFrame(unsigned int frame, Vector3* pVectors, unsigned int numanchors)
{
    MyAssert( numanchors <= m_NumAnchorsPerFrame );

    for( unsigned int i=0; i<numanchors; i++ )
    {
        m_pAnchorPoints[frame*m_NumAnchorsPerFrame + i].point = pVectors[i];
    }
}

void AnimatedSpriteDefinition::AddKeyTranslation(double time, Vector3 position)
{
    KeyTranslation* key = g_pAnimationKeyPool->m_TranslationKeys.MakeObjectActive();
    if( key == 0 )
    {
        LOGInfo( LOGTag, "Ran out of KeyTranslation's" );
    }
    else
    {
        key->m_Time = time;
        key->m_Position = position;
        m_KeysTranslation.Add( key );
    }

    IncreaseIfBigger( m_TimeOfLastKey, time );
}

void AnimatedSpriteDefinition::AddKeyScale(double time, Vector3 scale)
{
    KeyScale* key = g_pAnimationKeyPool->m_ScaleKeys.MakeObjectActive();
    if( key == 0 )
    {
        LOGInfo( LOGTag, "Ran out of KeyScale's" );
    }
    else
    {
        key->m_Time = time;
        key->m_Scale = scale;
        m_KeysScale.Add( key );
    }

    IncreaseIfBigger( m_TimeOfLastKey, time );
}

void AnimatedSpriteDefinition::AddKeyAlpha(double time, float alpha)
{
    KeyAlpha* key = g_pAnimationKeyPool->m_AlphaKeys.MakeObjectActive();
    if( key == 0 )
    {
        LOGInfo( LOGTag, "Ran out of KeyAlpha's" );
    }
    else
    {
        key->m_Time = time;
        key->m_Alpha = alpha;
        m_KeysAlpha.Add( key );
    }

    IncreaseIfBigger( m_TimeOfLastKey, time );
}

void AnimatedSpriteDefinition::AddKeyRotation(double time, Vector3 rotation)
{
    KeyRotation* key = g_pAnimationKeyPool->m_RotationKeys.MakeObjectActive();
    if( key == 0 )
    {
        LOGInfo( LOGTag, "Ran out of KeyRotation's" );
    }
    else
    {
        key->m_Time = time;
        key->m_Rotation = rotation;
        m_KeysRotation.Add( key );
    }

    IncreaseIfBigger( m_TimeOfLastKey, time );
}

unsigned int AnimatedSpriteDefinition::GetFrame(double time)
{
    if( m_NumFrames == 0 )
        return 0;
    //if( m_TimeOfLastKey > 0 )
    //    int bp = 1;

    double timepassed = 0;
    unsigned int frame = 0;

    if( m_TimeOfLastKey != 0 )
        timepassed = fmod( time, m_TimeOfLastKey );
    // not sure if hack: if time%m_TimeOfLastKey is 0, then use last frame, not first.(same as timepassed = m_TimeOfLastKey)
    if( timepassed == 0 )
    {
        frame = m_NumFrames-1;
    }
    else
    {    
        while( timepassed > 0 )
        {
            timepassed -= m_TimeToHoldFrame[frame];
            if( timepassed > 0 )
                frame++;
        }
    }

    if( frame > m_NumFrames )
        frame = m_NumFrames-1;

    //if( m_TimeOfLastKey > 0 )
    //    LOGInfo( LOGTag, "time: %f - %d\n", time, frame );

    return frame;
}

MySprite* AnimatedSpriteDefinition::GetSprite(double time)
{
    unsigned int frame = GetFrame(time);

    return m_pSprites[frame];
}

AnimatedSpriteAnchorPoint* AnimatedSpriteDefinition::GetAnchor(double time, unsigned int anchornum)
{
    MyAssert( anchornum < m_NumAnchorsPerFrame );

    unsigned int frame = GetFrame(time);

    MyAssert( frame < m_NumAnchorFrames );

    return &m_pAnchorPoints[frame*m_NumAnchorsPerFrame + anchornum];
}

Vector3 AnimatedSpriteDefinition::GetPosition(double time)
{
    unsigned int numframes = m_KeysTranslation.Count();
    if( numframes == 0 )
        return Vector3(0,0,0);

    if( m_Looping )
        time = fmod( time, m_TimeOfLastKey );

    unsigned int i;
    for( i=0; i<numframes; i++ )
    {
        double keytime = m_KeysTranslation[i]->m_Time;
        if( time < keytime )
            break;
    }

    if( i == numframes )
    {
        return m_KeysTranslation[i-1]->m_Position;
    }
    else
    {
        MyAssert( i != 0 );

        double prevtime = m_KeysTranslation[i-1]->m_Time;
        double nexttime = m_KeysTranslation[i]->m_Time;

        float ratio = (float)( (time - prevtime) / (nexttime - prevtime) );

        KeyTranslation* prevkey = m_KeysTranslation[i-1];
        KeyTranslation* nextkey = m_KeysTranslation[i];

        Vector3 pos;
        pos.x = prevkey->m_Position.x + ratio * ( nextkey->m_Position.x - prevkey->m_Position.x );
        pos.y = prevkey->m_Position.y + ratio * ( nextkey->m_Position.y - prevkey->m_Position.y );
        pos.z = prevkey->m_Position.z + ratio * ( nextkey->m_Position.z - prevkey->m_Position.z );

        return pos;
    }
}

Vector3 AnimatedSpriteDefinition::GetScale(double time)
{
    unsigned int numframes = m_KeysScale.Count();
    if( numframes == 0 )
        return Vector3(1,1,1);

    if( m_Looping )
        time = fmod( time, m_TimeOfLastKey );

    unsigned int i;
    for( i=0; i<numframes; i++ )
    {
        double keytime = m_KeysScale[i]->m_Time;
        if( time < keytime )
            break;
    }

    if( i == numframes )
    {
        return m_KeysScale[i-1]->m_Scale;
    }
    else
    {
        MyAssert( i != 0 );
        
        double prevtime = m_KeysScale[i-1]->m_Time;
        double nexttime = m_KeysScale[i]->m_Time;

        float ratio = (float)( (time - prevtime) / (nexttime - prevtime) );

        KeyScale* prevkey = m_KeysScale[i-1];
        KeyScale* nextkey = m_KeysScale[i];

        Vector3 scale;
        scale.x = prevkey->m_Scale.x + ratio * ( nextkey->m_Scale.x - prevkey->m_Scale.x );
        scale.y = prevkey->m_Scale.y + ratio * ( nextkey->m_Scale.y - prevkey->m_Scale.y );
        scale.z = prevkey->m_Scale.z + ratio * ( nextkey->m_Scale.z - prevkey->m_Scale.z );

        return scale;
    }
}

float AnimatedSpriteDefinition::GetAlpha(double time)
{
    unsigned int numframes = m_KeysAlpha.Count();
    if( numframes == 0 )
        return 1;

    if( m_Looping )
        time = fmod( time, m_TimeOfLastKey );

    unsigned int i;
    for( i=0; i<numframes; i++ )
    {
        double keytime = m_KeysAlpha[i]->m_Time;
        if( time < keytime )
            break;
    }

    if( i == numframes )
    {
        return m_KeysAlpha[i-1]->m_Alpha;
    }
    else
    {
        MyAssert( i != 0 );

        double prevtime = m_KeysAlpha[i-1]->m_Time;
        double nexttime = m_KeysAlpha[i]->m_Time;

        float ratio = (float)( (time - prevtime) / (nexttime - prevtime) );

        KeyAlpha* prevkey = m_KeysAlpha[i-1];
        KeyAlpha* nextkey = m_KeysAlpha[i];

        float alpha;
        alpha = prevkey->m_Alpha + ratio * ( nextkey->m_Alpha - prevkey->m_Alpha );

        return alpha;
    }
}

Vector3 AnimatedSpriteDefinition::GetRotation(double time)
{
    unsigned int numframes = m_KeysRotation.Count();
    if( numframes == 0 )
        return Vector3(0,0,0);

    if( m_Looping )
        time = fmod( time, m_TimeOfLastKey );

    unsigned int i;
    for( i=0; i<numframes; i++ )
    {
        double keytime = m_KeysRotation[i]->m_Time;
        if( time < keytime )
            break;
    }

    if( i == numframes )
    {
        return m_KeysRotation[i-1]->m_Rotation;
    }
    else
    {
        MyAssert( i != 0 );

        double prevtime = m_KeysRotation[i-1]->m_Time;
        double nexttime = m_KeysRotation[i]->m_Time;

        float ratio = (float)( (time - prevtime) / (nexttime - prevtime) );

        KeyRotation* prevkey = m_KeysRotation[i-1];
        KeyRotation* nextkey = m_KeysRotation[i];

        Vector3 rot;
        rot.x = prevkey->m_Rotation.x + ratio * ( nextkey->m_Rotation.x - prevkey->m_Rotation.x );
        rot.y = prevkey->m_Rotation.y + ratio * ( nextkey->m_Rotation.y - prevkey->m_Rotation.y );
        rot.z = prevkey->m_Rotation.z + ratio * ( nextkey->m_Rotation.z - prevkey->m_Rotation.z );

        return rot;
    }
}



AnimatedSpriteInstance::AnimatedSpriteInstance()
{
    m_pAnimDef = 0;

    m_TimeFromStart = 0;

    m_BaseTransform.SetIdentity();

    m_pParentMatrix = 0;
}

AnimatedSpriteInstance::~AnimatedSpriteInstance()
{
}

void AnimatedSpriteInstance::Clear()
{
    ResetAnimation();
    m_pAnimDef = 0;
    m_BaseTransform.SetIdentity();
    m_pParentMatrix = 0;
}

void AnimatedSpriteInstance::ResetAnimation()
{
    m_TimeFromStart = 0;
}

void AnimatedSpriteInstance::Tick(float deltaTime)
{
    m_TimeFromStart += deltaTime;
}

MySprite* AnimatedSpriteInstance::SetupSpriteForDrawing(MySprite* overridesprite)
{
    MyAssert( m_pAnimDef );
    if( m_pAnimDef == 0 )
        return 0;
    
    //Vector3 position = m_pAnimDef->GetPosition( m_TimeFromStart );
    //Vector3 scale = m_pAnimDef->GetScale( m_TimeFromStart );
    //float alpha = m_pAnimDef->GetAlpha( m_TimeFromStart );
    //Vector3 rotation = m_pAnimDef->GetRotation( m_TimeFromStart );

    if( m_pParentMatrix )
    {
        m_WorldTransform = *m_pParentMatrix * m_BaseTransform;
    }
    else
    {
        m_WorldTransform = m_BaseTransform;
    }

    MySprite* pSprite;
    if( overridesprite )
        pSprite = overridesprite;
    else
        pSprite = m_pAnimDef->GetSprite( m_TimeFromStart );

    MyAssert( pSprite );
    if( pSprite == 0 )
        return 0;

    //pSprite->SetTransform( temp );
    //pSprite->SetRST( rotation, scale, position, false );
    //pSprite->SetTint( ColorByte(255,255,255,(unsigned char)(alpha*255)) );

    return pSprite;
}

void AnimatedSpriteInstance::Draw(MyMatrix* pMatProj, MyMatrix* pMatView)
{
    MySprite* pSprite = SetupSpriteForDrawing( 0 );
    pSprite->Draw( pMatProj, pMatView, &m_WorldTransform );
}

void AnimatedSpriteInstance::SetTime(double time)
{
    m_TimeFromStart = time;
}

void AnimatedSpriteInstance::SetBaseTransform(MyMatrix& mat)
{
    m_BaseTransform = mat;
}

bool AnimatedSpriteInstance::IsFinished()
{
    MyAssert( m_pAnimDef );

    if( m_TimeFromStart > m_pAnimDef->m_TimeOfLastKey )
        return true;

    return false;
}

AnimatedSpriteAnchorPoint* AnimatedSpriteInstance::GetAnchor(unsigned int anchornum)
{
    MyAssert( m_pAnimDef );

    return m_pAnimDef->GetAnchor( m_TimeFromStart, anchornum );
}
