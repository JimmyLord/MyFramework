//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __Utility_H__
#define __Utility_H__

const float FEQUALEPSILON = 0.00001f;

void FixSlashesInPath(char* path);
const char* GetRelativePath(char* fullpath); // will replace backslashes with forward slashes in fullpath
void GetFullPath(const char* relativepath, char* fullpath, unsigned int maxcharsinfullpatharray);
void ParseFilename(const char* fullpath, char* outFilename, int sizeFilename, char* outExtension, int sizeExtension);

inline bool fequal(const float a, const float b, const float epsilon = FEQUALEPSILON)
{
    return fabs(a - b) <= epsilon;
}

inline bool fnotequal(const float a, const float b, const float epsilon = FEQUALEPSILON)
{
    return fabs(a - b) > epsilon;
}

inline bool fequal(const double a, const double b, const double epsilon = FEQUALEPSILON)
{
    return fabs(a - b) <= epsilon;
}

inline bool fnotequal(const double a, const double b, const double epsilon = FEQUALEPSILON)
{
    return fabs(a - b) > epsilon;
}

template <class MyType> void MySwap(MyType &v1, MyType &v2)
{
    MyType temp = v1;
    v1 = v2;
    v2 = temp;
}

template <class MyType> void MyClamp(MyType &value, MyType min, MyType max)
{
    if( value < min )
        value = min;

    if( value > max )
        value = max;
}

template <class MyType> MyType MyClamp_Return(MyType value, MyType min, MyType max)
{
    MyType temp = value;

    if( temp < min )
        temp = min;

    if( temp > max )
        temp = max;

    return temp;
}

template <class MyType> void IncreaseIfBigger(MyType &value, MyType newvalue)
{
    if( newvalue > value )
        value = newvalue;
}

template <class MyType> void DecreaseIfLower(MyType &value, MyType newvalue, bool treatzeroasinfinite = false)
{
    if( treatzeroasinfinite && value == 0 )
        value = newvalue;

    if( newvalue < value )
        value = newvalue;
}

inline float MyRoundToMultipleOf(float number, float multiple)
{
    if( number >= 0 )
    {
        number += multiple / 2.0f;
        number -= fmod( number, multiple );
    }
    else
    {
        number *= -1;
        number += multiple / 2.0f;
        number -= fmod( number, multiple );
        number *= -1;
    }

    return number;
}

// a simple runtime version of offsetof, should be safe with non-POD types, I think.
inline size_t MyOffsetOf(void* pObject, void* pMember)
{
    return (char*)pMember - (char*)pObject;
}

#endif //__Utility_H__
