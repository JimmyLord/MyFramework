//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MyAnimation.h"

MyAnimationTimeline::MyAnimationTimeline()
{
    m_Name = 0;
    m_Duration = 0;
    m_TicksPerSecond = 0;
}

MyAnimationTimeline::~MyAnimationTimeline()
{
    SAFE_DELETE_ARRAY( m_Name );

    SAFE_DELETE_ARRAY( m_pChannelMemoryAllocation );
}

void MyAnimationTimeline::ImportFromJSON(cJSON* pAnimObj)
{
    cJSON* name = cJSON_GetObjectItem( pAnimObj, "Name" );
    int namelen = strlen( name->valuestring );
    m_Name = MyNew char[namelen+1];
    strcpy_s( m_Name, namelen+1, name->valuestring );

    cJSONExt_GetFloat( pAnimObj, "Duration", &m_Duration );
    cJSONExt_GetFloat( pAnimObj, "TicksPerSecond", &m_TicksPerSecond );

    unsigned int numchannels;
    cJSONExt_GetUnsignedInt( pAnimObj, "NumChannels", &numchannels );
    SetNumberOfChannels( numchannels );
}

int MyAnimationTimeline::ImportChannelsFromBuffer(char* pBuffer)
{
    int byteoffset = 0;

    for( unsigned int ci=0; ci<m_pChannels.Count(); ci++ )
    {
        byteoffset += m_pChannels[ci]->ImportFromBuffer( &pBuffer[byteoffset] );
    }

    return byteoffset;
}

void MyAnimationTimeline::SetNumberOfChannels(unsigned int numchannels)
{
    assert( m_pChannels.Length() == 0 );

    m_pChannels.FreeAllInList();
    m_pChannels.AllocateObjects( numchannels );

    m_pChannelMemoryAllocation = MyNew MyChannel[numchannels];

    for( unsigned int ci=0; ci<numchannels; ci++ )
    {
        m_pChannels.Add( &m_pChannelMemoryAllocation[ci] );
    }
}

int MyAnimationTimeline::FindChannelIndexForNode(unsigned int nodeindex)
{
    unsigned int ci;
    for( ci=0; ci<m_pChannels.Count(); ci++ )
    {
        if( m_pChannels[ci]->m_NodeIndex == (int)nodeindex )
            break;
    }

    if( ci == m_pChannels.Count() )
        return -1;

    return ci;
}

Vector3 MyAnimationTimeline::GetInterpolatedTranslation(float time, unsigned int channelindex)
{
    MyChannel* pChannel = m_pChannels[channelindex];

    // we need at least two values to interpolate...
    if( pChannel->m_TranslationTimes.Count() == 1 )
    {
        return m_pChannels[channelindex]->m_TranslationValues[0];
    }

    int startindex = 0;
    while( time > pChannel->m_TranslationTimes[startindex] && startindex < (int)pChannel->m_TranslationTimes.Count()-1 )
        startindex++;
    startindex -= 1;
    int endindex = startindex+1;

    float starttimestamp = pChannel->m_TranslationTimes[startindex];
    float endtimestamp = pChannel->m_TranslationTimes[endindex];

    float timebetweenframes = endtimestamp - starttimestamp;
    float perctimepassed = (time - starttimestamp) / timebetweenframes;
    //assert( perctimepassed >= 0.0f && perctimepassed <= 1.0f );

    Vector3& StartTranslation = pChannel->m_TranslationValues[startindex];
    Vector3& EndTranslation = pChannel->m_TranslationValues[endindex];

    Vector3 result = StartTranslation + (EndTranslation - StartTranslation) * perctimepassed;
    return result;
}

MyQuat MyAnimationTimeline::GetInterpolatedRotation(float time, unsigned int channelindex)
{
    MyChannel* pChannel = m_pChannels[channelindex];

    // we need at least two values to interpolate...
    if( pChannel->m_RotationTimes.Count() == 1 )
    {
        return m_pChannels[channelindex]->m_RotationValues[0];
    }

    // TODO: fix start/end times.  Some channels seem to have different end times(tick counts).
    //       might need to force a final key onto each channel to compensate... not sure.
    //       fmod for time is in MyMesh::RebuildAnimationMatrices.
    int startindex = 0;
    while( time > pChannel->m_RotationTimes[startindex] && startindex < (int)pChannel->m_RotationTimes.Count()-1 )
        startindex++;
    startindex -= 1;
    int endindex = startindex+1;

    float starttimestamp = pChannel->m_RotationTimes[startindex];
    float endtimestamp = pChannel->m_RotationTimes[endindex];

    float timebetweenframes = endtimestamp - starttimestamp;
    float perctimepassed = (time - starttimestamp) / timebetweenframes;
    //assert( perctimepassed >= 0.0f && perctimepassed <= 1.0f );

    MyQuat& StartRotation = pChannel->m_RotationValues[startindex];
    MyQuat& EndRotation = pChannel->m_RotationValues[endindex];
    MyQuat result = MyQuat::Lerp( StartRotation, EndRotation, perctimepassed ).GetNormalized();
    //MyQuat result = MyQuat::Slerp( StartRotation, EndRotation, perctimepassed );
    return result;
}

Vector3 MyAnimationTimeline::GetInterpolatedScaling(float time, unsigned int channelindex)
{
    return Vector3( 1, 1, 1 );
}

//==================================

int MyChannel::ImportFromBuffer(char* pBuffer)
{
    // channel id
    // num pos keys
    //     pos key times
    //     pos key values
    // num rot keys
    //     rot key times
    //     rot key values
    // num scale keys
    //     scale key times
    //     scale key values

    int byteoffset = 0;
    unsigned int numkeys;

    m_NodeIndex = *(int*)&pBuffer[byteoffset];
    byteoffset += sizeof( int );

    // translation keys
    {
        numkeys = *(int*)&pBuffer[byteoffset];
        byteoffset += sizeof( int );

        m_TranslationTimes.AllocateObjects( numkeys );
        m_TranslationValues.AllocateObjects( numkeys );
        m_TranslationTimes.BlockFill( &pBuffer[byteoffset], sizeof(float)*numkeys, numkeys );
        byteoffset += sizeof(float)*numkeys;
        m_TranslationValues.BlockFill( &pBuffer[byteoffset], sizeof(Vector3)*numkeys, numkeys );
        byteoffset += sizeof(Vector3)*numkeys;
    }

    // rotation keys
    {
        numkeys = *(int*)&pBuffer[byteoffset];
        byteoffset += sizeof( int );

        m_RotationTimes.AllocateObjects( numkeys );
        m_RotationValues.AllocateObjects( numkeys );
        m_RotationTimes.BlockFill( &pBuffer[byteoffset], sizeof(float)*numkeys, numkeys );
        byteoffset += sizeof(float)*numkeys;
        m_RotationValues.BlockFill( &pBuffer[byteoffset], sizeof(MyQuat)*numkeys, numkeys );
        byteoffset += sizeof(MyQuat)*numkeys;
    }

    // scale keys
    {
        numkeys = *(int*)&pBuffer[byteoffset];
        byteoffset += sizeof( int );

        m_ScaleTimes.AllocateObjects( numkeys );
        m_ScaleValues.AllocateObjects( numkeys );
        m_ScaleTimes.BlockFill( &pBuffer[byteoffset], sizeof(float)*numkeys, numkeys );
        byteoffset += sizeof(float)*numkeys;
        m_ScaleValues.BlockFill( &pBuffer[byteoffset], sizeof(Vector3)*numkeys, numkeys );
        byteoffset += sizeof(Vector3)*numkeys;
    }

    return byteoffset;
}
