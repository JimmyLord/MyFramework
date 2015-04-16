//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include <stdint.h>
#include <stdio.h>

//#include <Foundation/NSPathUtilities.h>
//#include <Corefoundation/CFBundle.h>
#include <GameKit/GameKit.h>

#include "AppDelegate.h"
#include "ViewController.h"

bool g_GameCenter_LoggedIn = false;

void GameCenter_LoginToService()
{
    [[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError* error)
        {
            if( error != nil )
            {
                LOGInfo( LOGTag, "GameCenter - Login failed\n" );
            }
            else
            {
                LOGInfo( LOGTag, "GameCenter - Login succeeded\n" );
                g_GameCenter_LoggedIn = true;
            }
        }
    ];
}

void GameCenter_ShowLeaderboard(const char* leaderboardid)
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    ViewController* viewController = appDelegate.viewController;
    
//    viewController.showGameCenterLeaderboard( leaderboardid );
    
    [viewController showGameCenterLeaderboard:leaderboardid];
}

void GameCenter_ShowAchievements()
{
    AppDelegate* appDelegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    ViewController* viewController = appDelegate.viewController;
    
    [viewController showGameCenterAchievements];
}

void GameCenter_SubmitScore(int boardid, const char* boardidstr, int score)
{
    NSString* nsstr = [ NSString stringWithUTF8String:boardidstr ] ;
    GKScore* scoreReporter = [[[GKScore alloc] initWithCategory: nsstr] autorelease];
//    GKScore* scoreReporter = [[[GKScore alloc] initWithCategory:@"TimedModeID"] autorelease];
    
    if( scoreReporter )
    {
        scoreReporter.value = score;
        
        [scoreReporter reportScoreWithCompletionHandler:^(NSError* error)
            {
                if( error != nil )
                {
                    LOGInfo( LOGTag, "GameCenter - Leaderboard Submit failed!\n" );
                    g_pGameServiceManager->MarkLeaderboardSubmitComplete( false, GameServiceProtocol_GameCenter, boardid, score );
                }
                else
                {
                    LOGInfo( LOGTag, "GameCenter - Leaderboard Submit succeeded!\n" );
                    g_pGameServiceManager->MarkLeaderboardSubmitComplete( true, GameServiceProtocol_GameCenter, boardid, score );
                }
            }
        ];
    }
}

void GameCenter_SubmitAchievement(int achid, const char* achidstr, float perc)
{
    NSString* nsstr = [ NSString stringWithUTF8String:achidstr ] ;
    GKAchievement* achievement = [[[GKAchievement alloc] initWithIdentifier: nsstr] autorelease];

    if( achievement )
    {
        achievement.percentComplete = perc;
        [achievement reportAchievementWithCompletionHandler:^(NSError *error)
            {
                if( error != nil )
                {
                    LOGInfo( LOGTag, "GameCenter - Achievement Submit failed!\n" );
                    g_pGameServiceManager->MarkAchSubmitComplete( false, GameServiceProtocol_GameCenter, achid );
                }
                else
                {
                    LOGInfo( LOGTag, "GameCenter - Achievement Submit succeeded!\n" );
                    g_pGameServiceManager->MarkAchSubmitComplete( true, GameServiceProtocol_GameCenter, achid );
                }
            }
        ];
    }
}
