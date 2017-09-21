//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __NotTheRealB2Settings_H__
#define __NotTheRealB2Settings_H__

// I'm still not using C++11+, so defining override to nothing.
#define override
#define nullptr 0

// Include the real b2Settings.h
#if !_DEBUG && !defined(NDEBUG)
// In release mode temporarily define NDEBUG, the real b2Settings.h has a check for it.
#define NDEBUG
#include "../../../Box2D/Box2D/Box2D/Common/b2Settings.h"
#undef NDEBUG
#else
#include "../../../Box2D/Box2D/Box2D/Common/b2Settings.h"
#endif

#endif //__NotTheRealB2Settings_H__
