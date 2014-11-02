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

#ifndef __GameService_ScoreLoop_H__
#define __GameService_ScoreLoop_H__

#if USE_SCORELOOP

#include <scoreloop/scoreloopcore.h>

extern const char* g_ScoreLoop_GameID;
extern const char* g_ScoreLoop_GameSecret;
extern const char* g_ScoreLoop_VersionNumber;
extern const char* g_ScoreLoop_GameCurrencyCode;
extern const char* g_ScoreLoop_Languages;

class GameServiceEntry;
class LeaderboardStorageScoreLoop;

class GameService_ScoreLoop
{
    friend class GameServiceManager;

public:
    bool m_LoggedIn;
    bool m_LoginFailed;

    SC_Client_h m_pClient;
    SC_InitData_t m_InitData;
    SC_User_h m_pUser;

    SC_ScoreController_h m_pScore_Controller;
    SC_ScoresController_h m_pScores_Controller;
    SC_RankingController_h m_pRanking_Controller;
    bool m_Score_Controller_Busy;
    bool m_Scores_Controller_Busy;
    bool m_Ranking_Controller_Busy;

    int m_RequestedOffset;
    int m_RequestedNumber;
    int m_RequestedBoardID;

    GameServiceEntry* m_GameServiceEntryBeingSent;

    LeaderboardStorageScoreLoop* m_pLeaderboardStorageScoreLoop;

public:
    GameService_ScoreLoop();
    ~GameService_ScoreLoop();

    void LoginToService();
    void Shutdown();
    void HandleEvents();

    void ShowLeaderboard(const char* leaderboardid);
    void ShowAchievements();

    void SubmitScore(GameServiceEntry* pEntry, int boardid, const char* tableidstr, int score);
    void SubmitAchievement(GameServiceEntry* pEntry, int achid, const char* achidstr, float perc);

    void RequestLeaderboardResults(int boardid, int offset, int number);

    void LeaderboardScoresArrived(SC_Error_t completionStatus);
    void LeaderboardRankingArrived(SC_Error_t completionStatus);
    void LeaderboardWriteArrived(SC_Error_t completionStatus);
};

#endif

#endif //__GameService_ScoreLoop_H__
