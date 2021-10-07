//
// Copyright (c) 2021 Jimmy Lord
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#include "RandomHelpers.h"

namespace fw::Random {

MTRand g_RNG;

void SetSeed(unsigned long seed)
{
    g_RNG.seed( seed );
}

double Double(double min, double max)
{
    double r = min + g_RNG() * ((double)max-min);
    return r;
}

float Float(float min, float max)
{
    double r = min + g_RNG() * ((double)max-min);
    return (float)r;
}

int Int(int min, int max)
{
    double r = min + g_RNG() * ((double)max-min);
    return (int)r;
}

} // namespace fw::Random
