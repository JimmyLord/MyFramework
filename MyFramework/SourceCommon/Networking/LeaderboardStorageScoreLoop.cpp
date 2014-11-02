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

#include "LeaderboardStorageScoreLoop.h"
//#include "Core/LeaderboardsConfig.h"
//#include "Core/ProfileManager.h"

LeaderboardStorageScoreLoop::LeaderboardStorageScoreLoop()
{
    m_Busy = false;

    for( int i=0; i<NumLeaderboardStorageBlocksCached; i++ )
    {
        m_Blocks[i].m_InUse = false;
    }

    for( int i=0; i<Leaderboard_NumBoardsScoreLoop; i++ )
    {
        m_UserIndices[i] = -1;
        m_TotalPlayers[i] = -1;
        m_Time_UserTotal_Retrieved[i] = -1000;
    }

    m_RequestedOffset = -1;
    m_RequestedBoardID = -1;
}

LeaderboardStorageScoreLoop::~LeaderboardStorageScoreLoop()
{
}

void LeaderboardStorageScoreLoop::Init()
{
}

bool LeaderboardStorageScoreLoop::HasANetworkErrorOccured()
{
    return false;
}

void LeaderboardStorageScoreLoop::Tick(double TimePassed)
{
    if( m_Busy )
        return;

    if( m_RequestedBoardID >= 0 )
        GetScoreBlock();

    // invalidate all blocks older than 3 minutes.
    if( true )
    {
        for( int i=0; i<NumLeaderboardStorageBlocksCached; i++ )
        {
            if( m_Blocks[i].m_InUse )
            {
                double age = MyTime_GetSystemTime() - m_Blocks[i].m_TimeRetrieved;
                if( age > 3*60 )
                    m_Blocks[i].m_InUse = false;
            }
        }

        for( int i=0; i<Leaderboard_NumBoardsScoreLoop; i++ )
        {
            double age = MyTime_GetSystemTime() - m_Time_UserTotal_Retrieved[i];
            if( age > 3*60 )
            {
                m_UserIndices[i] = -1;
                m_TotalPlayers[i] = -1;
                m_Time_UserTotal_Retrieved[i] = -1000;
            }
        }
    }
}

int LeaderboardStorageScoreLoop::FindBlockContainingID(int offset, int boardid)
{
    int baseindex = (offset/LeaderboardStorageBlockSize) * LeaderboardStorageBlockSize;

    // check for old block containing this index.
    for( int i=0; i<NumLeaderboardStorageBlocksCached; i++ )
    {
        if( m_Blocks[i].m_InUse == true && m_Blocks[i].m_BoardID == boardid )
        {
            if( m_Blocks[i].m_Offset == baseindex )
            {
                if( offset < m_Blocks[i].m_Offset + m_Blocks[i].m_NumEntriesInBlock )
                    return i;
                else
                    return i; // even though the block doesn't have the record we want, return it if we have it.
                    //return -1; // we have the block loaded, but it doesn't contain the entry.
            }
        }
    }

    // didn't find block, request it... assuming no other blocks are requested.
    if( m_Busy == false )
    {
        m_RequestedOffset = baseindex;
        m_RequestedBoardID = boardid;
    }

    return -1;
}

void LeaderboardStorageScoreLoop::GetScoreBlock()
{
#if USE_SCORELOOP
    if( m_Busy == false )
    {
        int offset = m_RequestedOffset;
        int boardid = m_RequestedBoardID;

        if( offset == -1 )
        {
            g_pGameServiceManager->m_pScoreLoop->RequestLeaderboardResults( boardid, -1, 50 );
            m_Busy = true;
        }
        else
        {
            g_pGameServiceManager->m_pScoreLoop->RequestLeaderboardResults( boardid, offset, 50 );
            m_Busy = true;
        }
    }
#endif
}

int LeaderboardStorageScoreLoop::FindDuplicateBlockOrReturnNew(int offset, int boardid)
{
    // check for old block containing this index.
    for( int i=0; i<NumLeaderboardStorageBlocksCached; i++ )
    {
        if( m_Blocks[i].m_InUse == true )
        {
            if( m_Blocks[i].m_Offset == offset && 
                m_Blocks[i].m_BoardID == boardid )
                return i;
        }
    }

    // find an empty block.
    for( int i=0; i<NumLeaderboardStorageBlocksCached; i++ )
    {
        if( m_Blocks[i].m_InUse == false )
        {
            return i;
        }
    }

    // find the oldest block in our list:
    double oldesttime = MyTime_GetSystemTime();
    int oldestblock = 0;
    for( int i=0; i<NumLeaderboardStorageBlocksCached; i++ )
    {
        if( m_Blocks[i].m_TimeRetrieved < oldesttime )
        {
            oldesttime = m_Blocks[i].m_TimeRetrieved;
            oldestblock = i;
        }
    }

    return oldestblock;

    //// shouldn't hit this, but if it does, just use the first block as a failsafe.
    //return 0;
}
