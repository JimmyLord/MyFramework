//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GameCenter_H__
#define __GameCenter_H__

extern bool g_GameCenter_LoggedIn;

void GameCenter_LoginToService();

void GameCenter_ShowLeaderboard(const char* leaderboardid);
void GameCenter_ShowAchievements();

void GameCenter_SubmitScore(int boardid, const char* tableidstr, int score);
void GameCenter_SubmitAchievement(int achid, const char* achidstr, float perc);

#endif //__GameCenter_H__
