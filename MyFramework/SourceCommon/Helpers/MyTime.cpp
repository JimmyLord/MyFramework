//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "MyTime.h"

#if MYFW_LINUX
#include <sys/time.h>
#endif

// Exposed to Lua, change elsewhere if function signature changes.
double MyTime_GetSystemTime(bool realtime)
{
#if MYFW_IOS || MYFW_OSX
    return CFAbsoluteTimeGetCurrent();
#elif MYFW_ANDROID || MYFW_BLACKBERRY //|| MYFW_EMSCRIPTEN
    timespec time;
    if( realtime )
        clock_gettime( CLOCK_REALTIME, &time );
    else
        clock_gettime( CLOCK_MONOTONIC, &time );
    //LOGInfo( LOGTag, "time: sec(%d), nsec(%d)\n", time.tv_sec, time.tv_nsec );
    return time.tv_sec + ((double)time.tv_nsec / 1000000000);
#elif (MYFW_NACL && !MYFW_PPAPI) || MYFW_EMSCRIPTEN || MYFW_LINUX
    struct timeval time;
    gettimeofday( &time, 0 );
    //LOGInfo( LOGTag, "time: sec(%d), usec(%d)\n", time.tv_sec, time.tv_usec );
    return time.tv_sec + ((double)time.tv_usec / 1000000);
#elif MYFW_WINDOWS || MYFW_WP8 || MYFW_PPAPI
    //FILETIME ft;
    //GetSystemTimeAsFileTime(&ft);
    //unsigned long long tt = ((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    //double time = tt / 10000000.0;
    //return time;
    unsigned __int64 freq;
    unsigned __int64 time;

    QueryPerformanceFrequency( (LARGE_INTEGER*)&freq );
    QueryPerformanceCounter( (LARGE_INTEGER*)&time );

    double timeseconds = (double)time / freq;

    return timeseconds;
#elif MYFW_BADA
    long long ticks = 0;
    result res = Osp::System::SystemTime::GetTicks( ticks );
    MyAssert( res == E_SUCCESS );
    return ticks / 1000.0;
#endif

    return 0;
}

static double starttime = MyTime_GetSystemTime();

// Exposed to Lua, change elsewhere if function signature changes.
double MyTime_GetRunningTime()
{
    double currenttime = MyTime_GetSystemTime();

    return currenttime - starttime;
}

double g_UnpausedTime = 0;
// Exposed to Lua, change elsewhere if function signature changes.
double MyTime_GetUnpausedTime()
{
    return g_UnpausedTime;
}
