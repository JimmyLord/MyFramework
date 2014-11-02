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

#ifndef __GameServiceManager_H__
#define __GameServiceManager_H__

class GameServiceManager;
class WebRequestObject;
class GameService_ScoreLoop;

extern GameServiceManager* g_pGameServiceManager;

enum GameServicePlayerStatus
{
    GameServicePlayerStatus_None,
    GameServicePlayerStatus_Offline,
    GameServicePlayerStatus_Online_Idle,
    GameServicePlayerStatus_Online_Searching,
    GameServicePlayerStatus_Online_Playing,
};

enum GameServiceMessageType
{
    GameServiceMessageType_None,
    GameServiceMessageType_SubmitScore,
    GameServiceMessageType_GetPlayerList,
};

enum GameServiceType
{
    GameServiceType_LeaderboardEntry,
    GameServiceType_Achievement,
};

enum GameServiceProtocol
{
    GameServiceProtocol_All = -1, // used when adding a new score to the list of scores to be submitted.
    GameServiceProtocol_MyServer,
    GameServiceProtocol_GameCenter,
    GameServiceProtocol_ScoreLoop,
    GameServiceProtocol_NumProtocols,
};

enum GameServiceSubmitState
{
    GameServiceSubmitState_New,
    GameServiceSubmitState_Started,
    GameServiceSubmitState_Failed,
    GameServiceSubmitState_Done,
};

struct GameMatchmakingEntry
{
    int ip;
    int port;
};

struct GameServiceEntry
{
    GameServiceType m_Type;
    GameServiceSubmitState m_State;
    GameServiceProtocol m_Protocol;
    char m_Username[32];
    char m_Password[32];
    int m_GameID;
    int m_ID;
    int m_Score;
    bool m_OverwriteScore;
    float m_Percentage;
    char m_IDStr[32];
    double m_TimeSendLastAttempted; // retry on failure every 10 minutes.

    void FillAsLeaderboardEntry(GameServiceProtocol protocol, char* username, char* password, int gameid, int boardid, int score, const char* boardidstr, bool overwrite)
    {
        m_Type = GameServiceType_LeaderboardEntry;
        m_State = GameServiceSubmitState_New;
        m_Protocol = protocol;

        strcpy_s( m_Username, 32, username );
        strcpy_s( m_Password, 32, password );
        m_GameID = gameid;
        m_ID = boardid;
        m_Score = score;
        strcpy_s( m_IDStr, 32, boardidstr );

        m_OverwriteScore = overwrite;
        
        m_TimeSendLastAttempted = -10000;
    }

    void FillAsAchievement(GameServiceProtocol protocol, char* username, char* password, int gameid, int achid, float perc, const char* achidstr)
    {
        m_Type = GameServiceType_Achievement;
        m_State = GameServiceSubmitState_New;
        m_Protocol = protocol;
        
        strcpy_s( m_Username, 32, username );
        strcpy_s( m_Password, 32, password );
        m_GameID = gameid;
        m_ID = achid;
        m_Percentage = perc;
        strcpy_s( m_IDStr, 32, achidstr );

        m_TimeSendLastAttempted = -10000;
    }
};

class GameServiceManager
{
protected:
    MyActivePool<GameServiceEntry*> m_Entries;
    MyActivePool<GameMatchmakingEntry*> m_MatchmakingEntries;
    WebRequestObject* m_pWebRequestObject;

    GameServiceEntry m_CopyOfEntryBeingSentToMyServer;

    GameServicePlayerStatus m_PlayerStatus;
    char m_Username[32+1]; // MyServer has a limit of MAX_USERPASS_LENGTH(15).
    char m_Password[32+1]; // MyServer has a limit of MAX_USERPASS_LENGTH(15).

    bool m_SearchingForMatch;
    GameServiceMessageType m_LastMessageTypeSent;

public:
#if USE_SCORELOOP
    GameService_ScoreLoop* m_pScoreLoop;
#endif

public:
    GameServiceManager();
    virtual ~GameServiceManager();

    virtual void AllocateEntries(unsigned int numentries);
    virtual void Tick(const char* customuseragentchunk = 0);
    
    virtual void AddScoreToList(char* username, char* password, int gameid, int boardid, int score, const char* boardlabel, bool overwrite, GameServiceProtocol protocol);
    virtual bool FindAndRemoveOldScoreIfLower(GameServiceProtocol protocol, int boardid, int score);
    
    virtual void AddAchToList(char* username, char* password, int gameid, int achid, float perc, const char* achidstr, GameServiceProtocol protocol);
    virtual bool FindAndRemoveOldPercIfLower(GameServiceProtocol protocol, int achid, float perc);

    virtual void MarkLeaderboardSubmitComplete(bool success, GameServiceProtocol protocol, int boardid, int score);
    virtual void MarkAchSubmitComplete(bool success, GameServiceProtocol protocol, int achid);
    
    unsigned int GetActiveEntries() { return m_Entries.m_ActiveObjects.Count(); }
    unsigned int GetNumInProgressEntries();

    unsigned int GetNumMatchmakingEntries() { return m_MatchmakingEntries.m_ActiveObjects.Count(); }
    GameMatchmakingEntry* GetMatchmakingEntry(int i) { return m_MatchmakingEntries.m_ActiveObjects[i]; }

    void LoginToService();

    void ShowLeaderboard(const char* leaderboardid);
    void ShowAchievements();

//    void SubmitScore(int boardid, const char* tableidstr, int score);
//    void SubmitAchievement(int achid, const char* achidstr, float perc);

    void RequestLeaderboardEntries(int boardid, int offset, int number);

    void StartMatchmaking();
};

#endif //__GameServiceManager_H__
