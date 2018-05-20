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

#ifndef __LeaderboardStorageScoreLoop_H__
#define __LeaderboardStorageScoreLoop_H__

//#include "Core/LeaderboardsConfig.h"

#define Leaderboard_NumBoardsScoreLoop 5

#define LeaderboardStorageBlockSize           50   // can't change without changing php files.
#define NumLeaderboardStorageBlocksCached     5

struct LeaderboardBlockEntry
{
    char m_Username[15+1];
    int m_Score;
};

struct LeaderboardStorageBlock
{
    bool m_InUse;

    int m_Offset;
    int m_BoardID;

    LeaderboardBlockEntry m_Entries[LeaderboardStorageBlockSize];
    double m_TimeRetrieved;
    int m_NumEntriesInBlock;

    int m_TotalEntriesInList;
};

class LeaderboardStorageScoreLoop
{
    friend class GameServiceScoreLoop;

public:
    bool m_Busy;

    int m_RequestedOffset; // set to -1 to find player in list.
    int m_RequestedBoardID;

public:
    LeaderboardStorageBlock m_Blocks[NumLeaderboardStorageBlocksCached];

    int m_UserIndices[Leaderboard_NumBoardsScoreLoop];
    int m_TotalPlayers[Leaderboard_NumBoardsScoreLoop];
    double m_Time_UserTotal_Retrieved[Leaderboard_NumBoardsScoreLoop];

protected:
    void GetScoreBlock();

public:
    int FindDuplicateBlockOrReturnNew(int offset, int boardid);

public:
    LeaderboardStorageScoreLoop();
    ~LeaderboardStorageScoreLoop();

    void Init();
    void Tick(float deltaTime);

    int FindBlockContainingID(int offset, int boardid);
    bool HasANetworkErrorOccured();
};

#endif //__LeaderboardStorageScoreLoop_H__
