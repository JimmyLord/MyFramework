//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "GameServiceManager.h"

GameServiceManager* g_pGameServiceManager = 0;

#include "GameService_ScoreLoop.h"

GameServiceManager::GameServiceManager()
{
    m_pWebRequestObject = 0;

    m_PlayerStatus = GameServicePlayerStatus_None;
    m_Username[0] = 0;
    m_Password[0] = 0;

    m_SearchingForMatch = false;
    m_LastMessageTypeSent = GameServiceMessageType_None;

#if USE_SCORELOOP
    m_pScoreLoop = MyNew GameService_ScoreLoop;
#endif
}

GameServiceManager::~GameServiceManager()
{
    SAFE_DELETE( m_pWebRequestObject );
#if USE_SCORELOOP
    SAFE_DELETE( m_pScoreLoop );
#endif

    m_Entries.DeleteAllObjectsInPool();
    m_MatchmakingEntries.DeleteAllObjectsInPool();
}

void GameServiceManager::AllocateEntries(unsigned int numentries)
{
    m_Entries.AllocateObjects( numentries );
    m_MatchmakingEntries.AllocateObjects( 10 );

    for( unsigned int i=0; i<m_Entries.Length(); i++ )
    {
        m_Entries.AddInactiveObject( MyNew GameServiceEntry );
    }

    for( unsigned int i=0; i<10; i++ )
    {
        m_MatchmakingEntries.AddInactiveObject( MyNew GameMatchmakingEntry );
    }

    MyAssert( m_pWebRequestObject == 0 );
    m_pWebRequestObject = MyNew WebRequestObject;
#if 1 //MYFW_NACL
    m_pWebRequestObject->Init( "www.flatheadgames.com", 80 );
#else
    m_pWebRequestObject->Init( "208.83.209.13", 80 );
#endif
}

void GameServiceManager::Tick(const char* customuseragentchunk)
{
    if( m_pWebRequestObject == 0 )
        return;

    m_pWebRequestObject->Tick( customuseragentchunk );

    const char* result = m_pWebRequestObject->GetResult();

    if( result != 0 )
    {
        if( m_LastMessageTypeSent == GameServiceMessageType_SubmitScore )
        {
            cJSON* root = cJSON_Parse( result );

            if( root )
            {
                cJSON* obj = cJSON_GetObjectItem( root, "LeaderboardUpdated" );
                if( obj )
                {
                    LOGInfo( LOGTag, "MyService - Leaderboard Submit succeeded!\n" );
                    MarkLeaderboardSubmitComplete( true, m_CopyOfEntryBeingSentToMyServer.m_Protocol,
                                    m_CopyOfEntryBeingSentToMyServer.m_ID,
                                    m_CopyOfEntryBeingSentToMyServer.m_Score );
                }
                else // failed
                {
                    LOGInfo( LOGTag, "MyService - Leaderboard Submit failed!\n" );
                    MarkLeaderboardSubmitComplete( false, m_CopyOfEntryBeingSentToMyServer.m_Protocol,
                                                  m_CopyOfEntryBeingSentToMyServer.m_ID,
                                                  m_CopyOfEntryBeingSentToMyServer.m_Score );
                }

                cJSON_Delete( root );
            }

            m_pWebRequestObject->ClearResult();
            m_LastMessageTypeSent = GameServiceMessageType_None;
        }

        if( m_LastMessageTypeSent == GameServiceMessageType_GetPlayerList )
        {
            cJSON* root = cJSON_Parse( result );

            if( root )
            {
                cJSON* ips = cJSON_GetObjectItem( root, "IPExternals" );
                if( ips )
                {
                    int numips = cJSON_GetArraySize( ips );

                    m_MatchmakingEntries.InactivateAllObjects();

                    for( int i=0; i<numips; i++ )
                    {
                        cJSON* ipaddr = cJSON_GetArrayItem( ips, i );
                        if( ipaddr )
                        {
                            GameMatchmakingEntry* pEntry = m_MatchmakingEntries.MakeObjectActive();
                            if( pEntry )
                            {
                                int ip[4];
                                int port;

                                sscanf_s( ipaddr->valuestring, "%d.%d.%d.%d:%d", &ip[0], &ip[1], &ip[2], &ip[3], &port );
                                pEntry->ip = ip[0] | ip[1]<<8 | ip[2]<<16 | ip[3]<<24;
                                pEntry->port = port;
                            }
                        }
                    }
                }

                cJSON_Delete( root );
            }

            m_pWebRequestObject->ClearResult();
            m_LastMessageTypeSent = GameServiceMessageType_None;
            m_SearchingForMatch = false;
        }
    }

    if( m_SearchingForMatch )
    {
        if( m_pWebRequestObject->IsBusy() == false )
        {
            m_pWebRequestObject->RequestWebPage( "/exgas/getplayerlist.php?game=%s",
                g_pGameCore->GetMatchmakingGameName() );

            m_LastMessageTypeSent = GameServiceMessageType_GetPlayerList;
        }
    }

    for( unsigned int i=0; i<m_Entries.m_ActiveObjects.Count(); i++ )
    {
        GameServiceEntry* pEntry = m_Entries.m_ActiveObjects[i];

        if( pEntry->m_State == GameServiceSubmitState_New )
        {
            if( pEntry->m_Type == GameServiceType_LeaderboardEntry )
            {
                if( pEntry->m_Protocol == GameServiceProtocol_MyServer &&
                    m_pWebRequestObject->IsBusy() == false )
                {
                    m_CopyOfEntryBeingSentToMyServer = *pEntry;

                    m_pWebRequestObject->RequestWebPage( "/mygameservice/leaderboard_post.php?user=%s&pass=%s&gameid=%d&boardid=%d&score=%d",
                                pEntry->m_Username,
                                pEntry->m_Password,
                                pEntry->m_GameID,
                                pEntry->m_ID,
                                pEntry->m_Score );

                    m_LastMessageTypeSent = GameServiceMessageType_SubmitScore;
                    pEntry->m_State = GameServiceSubmitState_Started;
                    pEntry->m_TimeSendLastAttempted = MyTime_GetSystemTime();
                }

#if MYFW_IOS
                if( pEntry->m_Protocol == GameServiceProtocol_GameCenter && g_GameCenter_LoggedIn == true )
                {
                    GameCenter_SubmitScore( pEntry->m_ID, pEntry->m_IDStr, pEntry->m_Score );
                    pEntry->m_State = GameServiceSubmitState_Started;
                    pEntry->m_TimeSendLastAttempted = MyTime_GetSystemTime();
                }
#endif

#if USE_SCORELOOP
                if( pEntry->m_Protocol == GameServiceProtocol_ScoreLoop && m_pScoreLoop->m_LoggedIn == true )
                {
                    m_pScoreLoop->SubmitScore( pEntry, pEntry->m_ID, pEntry->m_IDStr, pEntry->m_Score );
                    pEntry->m_State = GameServiceSubmitState_Started;
                    pEntry->m_TimeSendLastAttempted = MyTime_GetSystemTime();
                }
#endif
            }
            else if( pEntry->m_Type == GameServiceType_Achievement )
            {               
#if MYFW_IOS
                if( pEntry->m_Protocol == GameServiceProtocol_GameCenter && g_GameCenter_LoggedIn == true )
                {
                    GameCenter_SubmitAchievement( pEntry->m_ID, pEntry->m_IDStr, pEntry->m_Percentage );
                    pEntry->m_State = GameServiceSubmitState_Started;
                    pEntry->m_TimeSendLastAttempted = MyTime_GetSystemTime();
                }
#endif

#if USE_SCORELOOP
                if( pEntry->m_Protocol == GameServiceProtocol_ScoreLoop && m_pScoreLoop->m_LoggedIn == true )
                {
                    m_pScoreLoop->SubmitAchievement( pEntry, pEntry->m_ID, pEntry->m_IDStr, pEntry->m_Percentage );
                    pEntry->m_State = GameServiceSubmitState_Started;
                    pEntry->m_TimeSendLastAttempted = MyTime_GetSystemTime();
                }
#endif
            }
        }
        else if( pEntry->m_State == GameServiceSubmitState_Started )
        {
        }
        else if( pEntry->m_State == GameServiceSubmitState_Failed )
        {
            if( MyTime_GetSystemTime() - pEntry->m_TimeSendLastAttempted > 60*10 )
            {
                pEntry->m_State = GameServiceSubmitState_New; // try again.
            }
        }
        else if( pEntry->m_State == GameServiceSubmitState_Done )
        {
            m_Entries.MakeObjectInactiveByIndex( i );
            i--;
            continue;
        }
    }

    //// if our player is online, but not in a game, then update his status once a minute.
    //if( m_PlayerStatus == GameServicePlayerStatus_Online_Idle ||
    //    m_PlayerStatus == GameServicePlayerStatus_Online_Searching )
    //{
    //    if( m_pWebRequestObject->IsBusy() == false )
    //    {
    //        //&pass=%s
    //        //    pButtonPassword->m_Strings[1],
    //        m_pWebRequestObject->RequestWebPage( "/exgas/updateinfo.php?user=%s&game=%s&ipext=%s&ipint=%s",
    //            pButtonUsername->m_Strings[1],
    //            g_pGame->GetMatchmakingGameName(),
    //            "192",
    //            "192" );
    //    }
    //}
}

void GameServiceManager::AddScoreToList(char* username, char* password, int gameid, int boardid, int score, const char* boardlabel, bool overwrite, GameServiceProtocol protocol)
{
    GameServiceEntry* pEntry;

    if( protocol == GameServiceProtocol_All || protocol == GameServiceProtocol_MyServer )
    {
        if( username[0] != 0 )
        {
            if( FindAndRemoveOldScoreIfLower( GameServiceProtocol_MyServer, boardid, score ) )
            {
                pEntry = m_Entries.MakeObjectActive();
                if( pEntry )
                    pEntry->FillAsLeaderboardEntry( GameServiceProtocol_MyServer, username, password, gameid, boardid, score, boardlabel, overwrite );
            }
        }
    }

#if MYFW_IOS
    if( protocol == GameServiceProtocol_All || protocol == GameServiceProtocol_GameCenter )
    {
        if( FindAndRemoveOldScoreIfLower( GameServiceProtocol_GameCenter, boardid, score ) )
        {
            pEntry = m_Entries.MakeObjectActive();
            if( pEntry )
                pEntry->FillAsLeaderboardEntry( GameServiceProtocol_GameCenter, username, password, gameid, boardid, score, boardlabel, true );
        }
    }
#endif

#if USE_SCORELOOP
    if( protocol == GameServiceProtocol_All || protocol == GameServiceProtocol_ScoreLoop )
    {
        if( FindAndRemoveOldScoreIfLower( GameServiceProtocol_ScoreLoop, boardid, score ) )
        {
            pEntry = m_Entries.MakeObjectActive();
            if( pEntry )
                pEntry->FillAsLeaderboardEntry( GameServiceProtocol_ScoreLoop, username, password, gameid, boardid, score, boardlabel, true );
        }
    }
#endif
}

bool GameServiceManager::FindAndRemoveOldScoreIfLower(GameServiceProtocol protocol, int boardid, int score)
{
    for( unsigned int i=0; i<m_Entries.m_ActiveObjects.Count(); i++ )
    {
        GameServiceEntry* pEntry = m_Entries.m_ActiveObjects[i];
        
        if( pEntry->m_Type == GameServiceType_LeaderboardEntry &&
            pEntry->m_Protocol == protocol &&
            pEntry->m_ID == boardid )
        {
            if( pEntry->m_Score < score )
            {
                m_Entries.MakeObjectInactiveByIndex( i );
                return true; // post the new score
            }
            else
            {
                return false; // score we're posting is higher, ignore the new one
            }
        }
    }
    
    return true; // post the new score
}

void GameServiceManager::AddAchToList(char* username, char* password, int gameid, int achid, float perc, const char* achidstr, GameServiceProtocol protocol)
{
#if MYFW_IOS || USE_SCORELOOP
    GameServiceEntry* pEntry;
#endif
    
    //if( protocol == GameServiceProtocol_All || protocol == GameServiceProtocol_MyServer )
    //{
    //    if( username[0] != 0 )
    //    {
    //        if( FindAndRemoveOldPercIfLower( GameServiceProtocol_MyServer, achid, perc ) )
    //        {
    //            pEntry = m_Entries.MakeObjectActive();
    //            if( pEntry )
    //                pEntry->FillAsAchievement( GameServiceProtocol_MyServer, username, password, gameid, achid, perc, achidstr );
    //        }
    //    }
    //}
    
#if MYFW_IOS
    if( protocol == GameServiceProtocol_All || protocol == GameServiceProtocol_GameCenter )
    {
        if( FindAndRemoveOldPercIfLower( GameServiceProtocol_GameCenter, achid, perc ) )
        {
            pEntry = m_Entries.MakeObjectActive();
            if( pEntry )
                pEntry->FillAsAchievement( GameServiceProtocol_GameCenter, username, password, gameid, achid, perc, achidstr );
        }
    }
#endif

#if USE_SCORELOOP
    if( protocol == GameServiceProtocol_All || protocol == GameServiceProtocol_ScoreLoop )
    {
        if( FindAndRemoveOldPercIfLower( GameServiceProtocol_ScoreLoop, achid, perc ) )
        {
            pEntry = m_Entries.MakeObjectActive();
            if( pEntry )
                pEntry->FillAsAchievement( GameServiceProtocol_ScoreLoop, username, password, gameid, achid, perc, achidstr );
        }
    }
#endif
}

bool GameServiceManager::FindAndRemoveOldPercIfLower(GameServiceProtocol protocol, int achid, float perc)
{
    for( unsigned int i=0; i<m_Entries.m_ActiveObjects.Count(); i++ )
    {
        GameServiceEntry* pEntry = m_Entries.m_ActiveObjects[i];
        
        if( pEntry->m_Type == GameServiceType_LeaderboardEntry &&
            pEntry->m_Protocol == protocol &&
            pEntry->m_ID == achid )
        {
            if( pEntry->m_Percentage < perc )
            {
                m_Entries.MakeObjectInactiveByIndex( i );
                return true; // post the new perc/ach
            }
            else
            {
                return false; // new perc is higher, ignore the new entry
            }
        }
    }
    
    return true; // post the new perc/ach
}

void GameServiceManager::MarkLeaderboardSubmitComplete(bool success, GameServiceProtocol protocol, int boardid, int score)
{
    for( unsigned int i=0; i<m_Entries.m_ActiveObjects.Count(); i++ )
    {
        GameServiceEntry* pEntry = m_Entries.m_ActiveObjects[i];

        if( pEntry->m_Type == GameServiceType_LeaderboardEntry &&
            pEntry->m_Protocol == protocol &&
            pEntry->m_ID == boardid &&
            pEntry->m_Score == score )
        {
            if( success )
                pEntry->m_State = GameServiceSubmitState_Done;
            else
                pEntry->m_State = GameServiceSubmitState_Failed;
        }
    }
}

void GameServiceManager::MarkAchSubmitComplete(bool success, GameServiceProtocol protocol, int achid)
{
    for( unsigned int i=0; i<m_Entries.m_ActiveObjects.Count(); i++ )
    {
        GameServiceEntry* pEntry = m_Entries.m_ActiveObjects[i];
        
        if( pEntry->m_Type == GameServiceType_Achievement &&
            pEntry->m_Protocol == protocol &&
            pEntry->m_ID == achid )
        {
            if( success )
                pEntry->m_State = GameServiceSubmitState_Done;
            else
                pEntry->m_State = GameServiceSubmitState_Failed;
        }
    }
}

unsigned int GameServiceManager::GetNumInProgressEntries()
{
    unsigned int numinprogress = 0;

    for( unsigned int i=0; i<m_Entries.m_ActiveObjects.Count(); i++ )
    {
        GameServiceEntry* pEntry = m_Entries.m_ActiveObjects[i];

        if( pEntry->m_State == GameServiceSubmitState_Started )
        {
            numinprogress++;
        }
    }

    return numinprogress;
}

void GameServiceManager::LoginToService()
{
#if MYFW_IOS
    GameCenter_LoginToService();
#endif

#if USE_SCORELOOP
    m_pScoreLoop->LoginToService();
#endif
}

void GameServiceManager::ShowLeaderboard(const char* leaderboardid)
{
#if MYFW_IOS
    GameCenter_ShowLeaderboard( leaderboardid );
#endif

#if USE_SCORELOOP
    m_pScoreLoop->ShowLeaderboard( leaderboardid );
#endif
}

void GameServiceManager::ShowAchievements()
{
#if MYFW_IOS
    GameCenter_ShowAchievements();
#endif

#if USE_SCORELOOP
    m_pScoreLoop->ShowAchievements();
#endif
}

//void GameServiceManager::SubmitScore(int boardid, const char* tableidstr, int score)
//{
//#if MYFW_IOS
//    GameCenter_SubmitScore( boardid, tableidstr, score );
//#endif
//
//#if USE_SCORELOOP
//    m_pScoreLoop->SubmitScore( boardid, tableidstr, score );
//#endif
//}
//void GameServiceManager::SubmitAchievement(int achid, const char* achidstr, float perc)
//{
//#if MYFW_IOS
//    GameCenter_SubmitAchievement( achid, achidstr, perc );
//#endif
//
//#if USE_SCORELOOP
//    m_pScoreLoop->SubmitAchievement( achid, achidstr, perc );
//#endif
//}

void GameServiceManager::RequestLeaderboardEntries(int boardid, int offset, int number)
{
#if USE_SCORELOOP
    m_pScoreLoop->RequestLeaderboardResults( boardid, offset, number );
#endif
}

void GameServiceManager::StartMatchmaking()
{
    m_SearchingForMatch = true;
}
