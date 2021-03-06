//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#if MYFW_EMSCRIPTEN
#include <emscripten.h>
#endif

bool MyAssertFunc(const char* file, unsigned long line)
{
    LOGInfo( LOGTag, "======================================================\n" );
    LOGInfo( LOGTag, "%s(%lu) - assert failed\n", file, line );
    LOGInfo( LOGTag, "======================================================\n" );

#if MYFW_WINDOWS
    __debugbreak(); // Trigger a breakpoint.
#endif

#if MYFW_EMSCRIPTEN
    // Dump the stack to the javascript console window.
    EM_ASM(
        stackTrace();
    );
    //emscripten_run_script( "stackTrace();" );

    assert( false ); // Stop execution on asserts.
#endif

    return false;
}
