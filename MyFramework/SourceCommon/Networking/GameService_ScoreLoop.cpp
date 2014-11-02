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

#if USE_SCORELOOP

#include <scoreloop/scoreloopcore.h>
#include "../Networking/LeaderboardStorageScoreLoop.h"

void LeaderboardScoresCallback(void* cookie, SC_Error_t completionStatus);
void LeaderboardRankingCallback(void* cookie, SC_Error_t completionStatus);
void LeaderboardWriteCallback(void* cookie, SC_Error_t completionStatus);

void LeaderboardScoresCallback(void* cookie, SC_Error_t completionStatus)
{
    ((GameService_ScoreLoop*)cookie)->LeaderboardScoresArrived( completionStatus );
}

void LeaderboardRankingCallback(void* cookie, SC_Error_t completionStatus)
{
    ((GameService_ScoreLoop*)cookie)->LeaderboardRankingArrived( completionStatus );
}

void LeaderboardWriteCallback(void* cookie, SC_Error_t completionStatus)
{
    ((GameService_ScoreLoop*)cookie)->LeaderboardWriteArrived( completionStatus );
}

GameService_ScoreLoop::GameService_ScoreLoop()
{
    m_LoggedIn = false;
    m_LoginFailed = false;

    m_pClient = 0;
    m_pUser = 0;

    m_pScore_Controller = 0;
    m_pScores_Controller = 0;
    m_pRanking_Controller = 0;
    m_Score_Controller_Busy = false;
    m_Scores_Controller_Busy = false;
    m_Ranking_Controller_Busy = false;

    m_GameServiceEntryBeingSent = 0;

    m_pLeaderboardStorageScoreLoop = 0;
}

GameService_ScoreLoop::~GameService_ScoreLoop()
{
    Shutdown();
}

void GameService_ScoreLoop::Shutdown()
{
    if( m_pClient )
        SC_Client_Release( m_pClient );

    if( m_pScore_Controller )
        SC_ScoreController_Release( m_pScore_Controller );
    if( m_pScores_Controller )
        SC_ScoresController_Release( m_pScores_Controller );
    if( m_pRanking_Controller )
        SC_RankingController_Release( m_pRanking_Controller );
    // TODO:? clean up SC_Client_CreateScore created objs...

    m_pScore_Controller = 0;
    m_pScores_Controller = 0;
    m_pRanking_Controller = 0;

    m_Score_Controller_Busy = false;
    m_Scores_Controller_Busy = false;
    m_Ranking_Controller_Busy = false;

    m_GameServiceEntryBeingSent = 0;

    m_LoggedIn = false;
    m_LoginFailed = false;
}

void GameService_ScoreLoop::LeaderboardScoresArrived(SC_Error_t completionStatus)
{
    m_Scores_Controller_Busy = false;

    if( completionStatus != SC_OK )
    {
        LOGError( LOGTag, "ScoreLoop - Leaderboard Scores Request failed!\n" );
        return;
    }

    LOGInfo( LOGTag, "ScoreLoop - Leaderboard Scores Request complete!\n" );

    SC_ScoreList_h scores = SC_ScoresController_GetScores( m_pScores_Controller );

#if MYFW_BLACKBERRY10
    unsigned int count = SC_ScoreList_GetCount( scores );
#else
    unsigned int count = SC_ScoreList_GetScoresCount( scores );
#endif

    int blocktouse = m_pLeaderboardStorageScoreLoop->FindDuplicateBlockOrReturnNew( m_RequestedOffset, m_RequestedBoardID );
    LeaderboardStorageBlock* pBlock = &m_pLeaderboardStorageScoreLoop->m_Blocks[blocktouse];

    m_pLeaderboardStorageScoreLoop->m_Busy = false;

    pBlock->m_InUse = true;
    pBlock->m_Offset = m_RequestedOffset;
    pBlock->m_BoardID = m_RequestedBoardID;
    pBlock->m_TimeRetrieved = MyTime_GetSystemTime();
    pBlock->m_NumEntriesInBlock = count;

    for( unsigned int i=0; i<count; i++ )
    {
        LeaderboardBlockEntry* pPlayer = &pBlock->m_Entries[i];

#if MYFW_BLACKBERRY10
        SC_Score_h score = SC_ScoreList_GetAt( scores, i );
#else
        SC_Score_h score = SC_ScoreList_GetScore( scores, i );
#endif

        double result = SC_Score_GetResult( score );
        //SC_Score_GetMinorResult( score );
        //SC_Score_GetLevel( score );
        unsigned int mode = SC_Score_GetMode( score );
        unsigned int rank = SC_Score_GetRank( score );
        SC_Context_h context = SC_Score_GetContext( score );

        SC_User_h user = SC_Score_GetUser( score );
        SC_String_h string = SC_User_GetLogin( user );
        const char* name = SC_String_GetData( string );

        pPlayer->m_Score = (int)result;
        strncpy( pPlayer->m_Username, name, 15 );
        pPlayer->m_Username[15] = 0;
    }

    m_pLeaderboardStorageScoreLoop->m_RequestedOffset = -1;
    m_pLeaderboardStorageScoreLoop->m_RequestedBoardID = -1;

    int bp = 1;
    //SC_ScoreList_Release( scores );
}

void GameService_ScoreLoop::LeaderboardRankingArrived(SC_Error_t completionStatus)
{
    m_Ranking_Controller_Busy = false;

    if( completionStatus != SC_OK )
    {
        LOGError( LOGTag, "ScoreLoop - Leaderboard Ranking Request failed!\n" );
        return;
    }

    LOGInfo( LOGTag, "ScoreLoop - Leaderboard Ranking Request complete!\n" );

    // if we requested a block around the player, then we didn't have his rank,
    // now we do, let the storage class ask for a specific block.
    if( m_pLeaderboardStorageScoreLoop->m_RequestedOffset == -1 )
        m_pLeaderboardStorageScoreLoop->m_Busy = false;

    int rank = SC_RankingController_GetRanking( m_pRanking_Controller );
    int total = SC_RankingController_GetTotal( m_pRanking_Controller );

    m_pLeaderboardStorageScoreLoop->m_UserIndices[m_RequestedBoardID] = rank - 1; // scoreloop ranks start at 1
    m_pLeaderboardStorageScoreLoop->m_TotalPlayers[m_RequestedBoardID] = total;

    m_pLeaderboardStorageScoreLoop->m_Time_UserTotal_Retrieved[m_RequestedBoardID] = MyTime_GetSystemTime();
}

void GameService_ScoreLoop::LeaderboardWriteArrived(SC_Error_t completionStatus)
{
    m_Score_Controller_Busy = false;

    GameServiceEntry* pEntry = m_GameServiceEntryBeingSent;

    if( completionStatus != SC_OK )
    {
        LOGError( LOGTag, "ScoreLoop - Leaderboard Submit failed!\n" );
        g_pGameServiceManager->MarkLeaderboardSubmitComplete( false, GameServiceProtocol_ScoreLoop, pEntry->m_ID, pEntry->m_Score );
        return;
    }

    LOGInfo( LOGTag, "ScoreLoop - Leaderboard Submit succeeded!\n" );
    g_pGameServiceManager->MarkLeaderboardSubmitComplete( true, GameServiceProtocol_ScoreLoop, pEntry->m_ID, pEntry->m_Score );
}

void GameService_ScoreLoop::LoginToService()
{
    // only try to log in once,
    //     TODO: make interface to reset this, so I can add a button to login and users can hit it multiple times.
    if( m_LoginFailed )
        return;

    SC_Error_t errCode;

    // Initialize scData
    SC_InitData_Init( &m_InitData );

    m_InitData.runLoopType = SC_RUN_LOOP_TYPE_CUSTOM;

    // Optionally modify the following fields:
    // scData.currentVersion = SC_INIT_CURRENT_VERSION;
    // scData.minimumRequiredVersion = SC_INIT_VERSION_1_0;
    // scData.runLoopType = SC_RUN_LOOP_TYPE_BPS;

    // Create the client.
    // aGameId, aGameSecret and aCurrency are const char strings that you obtain from Scoreloop.
    // aGameVersion should be your current game version.
    // aLanguageCode specifies the language support for localization in awards,
    // for example, "en" for English, which is the default language.
    errCode = SC_Client_New( &m_pClient, &m_InitData, g_ScoreLoop_GameID, g_ScoreLoop_GameSecret, g_ScoreLoop_VersionNumber, g_ScoreLoop_GameCurrencyCode, g_ScoreLoop_Languages );

    if( errCode != SC_OK )
    {
        LOGError( LOGTag, "ScoreLoop - SC_Client_New() failed\n" );

        m_LoginFailed = true;
    }
    else
    {
        LOGInfo( LOGTag, "ScoreLoop - SC_Client_New() ok\n" );

        m_LoggedIn = true;

        // Request a score controller, scores controller and a ranking controller.
        errCode = SC_Client_CreateScoreController( m_pClient, &m_pScore_Controller, LeaderboardWriteCallback, this );
        if( errCode != SC_OK )
        {
            LOGError( LOGTag, "ScoreLoop - SC_Client_CreateScoreController() failed\n" );
        }

        errCode = SC_Client_CreateScoresController( m_pClient, &m_pScores_Controller, LeaderboardScoresCallback, this );
        if( errCode != SC_OK )
        {
            LOGError( LOGTag, "ScoreLoop - SC_Client_CreateScoresController() failed\n" );
        }

        errCode = SC_Client_CreateRankingController( m_pClient, &m_pRanking_Controller, LeaderboardRankingCallback, this );
        if( errCode != SC_OK )
        {
            LOGError( LOGTag, "ScoreLoop - SC_Client_CreateRankingController() failed\n" );
        }

        m_Score_Controller_Busy = false;
        m_Scores_Controller_Busy = false;
        m_Ranking_Controller_Busy = false;

        SC_Session_h session = SC_Client_GetSession( m_pClient );
        m_pUser = SC_Session_GetUser( session );
    }
}

void GameService_ScoreLoop::HandleEvents()
{
    if( m_LoggedIn == false )
        return;

    SC_HandleCustomEvent( &m_InitData, SC_FALSE );
}

void GameService_ScoreLoop::ShowLeaderboard(const char* leaderboardid)
{
}

void GameService_ScoreLoop::ShowAchievements()
{
}

void GameService_ScoreLoop::SubmitScore(GameServiceEntry* pEntry, int boardid, const char* boardidstr, int score)
{
    if( m_LoggedIn == false )
        return;

    if( m_Score_Controller_Busy )
        return;

    SC_Error_t errCode;
    unsigned int aMode, aLevel;
    double aResult;
    SC_Score_h scoreobj;

    // Create a score object.
    errCode = SC_Client_CreateScore( m_pClient, &scoreobj ); // TODO: refactor to just create one obj?
    if( errCode != SC_OK )
    {
        LOGError( LOGTag, "ScoreLoop - SC_Client_CreateScore() failed\n" );
        return;
    }

    m_GameServiceEntryBeingSent = pEntry;

    // set the score(double) in the score object
    SC_Score_SetResult( scoreobj, (double)score );

    SC_Score_SetMode( scoreobj, boardid );
    //SC_Score_SetMinorResult( scoreobj, aMinorResult ); // set a secondary number in the score
    //SC_Score_SetLevel( scoreobj, aLevel ); //aLevel is the level in the game

    SC_ScoreController_SubmitScore( m_pScore_Controller, scoreobj );

    m_Score_Controller_Busy = true;
}

void GameService_ScoreLoop::SubmitAchievement(GameServiceEntry* pEntry, int achid, const char* achidstr, float perc)
{
}

void GameService_ScoreLoop::RequestLeaderboardResults(int boardid, int offset, int number)
{
    if( m_LoggedIn == false )
        return;

    if( m_Scores_Controller_Busy == true )
        return;

    if( m_Ranking_Controller_Busy == true )
        return;

    SC_Error_t errCode;

    // SC_SCORES_SEARCH_LIST_ALL    SC_SCORES_SEARCH_LIST_24H    SC_SCORES_SEARCH_LIST_USER_COUNTRY
#if MYFW_BLACKBERRY10
    //SC_ScoresController_SetSearchList( m_pScores_Controller, SC_SCORES_SEARCH_LIST_ALL );
#else
    SC_ScoresController_SetSearchList( m_pScores_Controller, SC_SCORE_SEARCH_LIST_GLOBAL );
#endif

    SC_ScoresController_SetMode( m_pScores_Controller, boardid );

    m_RequestedOffset = offset;
    m_RequestedNumber = number;
    m_RequestedBoardID = boardid;

    // get the users rank... if we don't have it.
    if( m_pLeaderboardStorageScoreLoop->m_UserIndices[boardid] == -1 )
    {
#if MYFW_BLACKBERRY10
        errCode = SC_RankingController_LoadRankingForUserInMode( m_pRanking_Controller, m_pUser, boardid );
#else
        errCode = SC_RankingController_RequestRankingForUserInGameMode( m_pRanking_Controller, m_pUser, boardid );
#endif
        if( errCode != SC_OK )
        {
            LOGError( LOGTag, "ScoreLoop - SC_RankingController_LoadRankingForUserInMode() failed\n" );
        }
        else
        {
            m_Ranking_Controller_Busy = true;
        }
    }

    if( offset != -1 )
    {
        //errCode = SC_ScoresController_LoadScoresAroundUser(scores_controller, aUser, aRange);
#if MYFW_BLACKBERRY10
        SC_Range_t range;
        range.offset = offset;
        range.length = number;

        errCode = SC_ScoresController_LoadScores( m_pScores_Controller, range );
#else
        errCode = SC_ScoresController_LoadRange( m_pScores_Controller, offset, number );
#endif
        if( errCode != SC_OK )
        {
            LOGError( LOGTag, "ScoreLoop - SC_ScoresController_LoadScores() failed\n" );
        }
        else
        {
            m_Scores_Controller_Busy = true;
        }
    }
}

#endif
