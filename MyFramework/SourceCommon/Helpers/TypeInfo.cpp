//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#if MYFW_USING_WX && _DEBUG

std::vector<std::string> g_RegisteredClassnames;

void RegisterClassname(const char* name)
{
    // assert if there's any trouble with this name
    MyAssert( strlen(name) >= 8 );

    // Store this classname in a vector and check for duplicate registered classnames.
    std::vector<std::string>::iterator it;

    it = std::find( g_RegisteredClassnames.begin(), g_RegisteredClassnames.end(), name );
    if( it != g_RegisteredClassnames.end() )
    {
        MyAssert( false ); // classname was used twice.
    }
    else
    {    
        g_RegisteredClassnames.push_back( name );
    }
}

class baseclass
{
public:
    SetClassnameBase( "baseclass" ); // only first 8 chars are registered.

    baseclass()
    {
        ClassnameSanityCheck();
    }
};

class derivedclass : public baseclass
{
public:
    SetClassnameWithParent( "derivedclass", baseclass ); // only first 8 chars are registered.

    derivedclass()
    {
        ClassnameSanityCheck();
    }
};

class subderivedclass : public derivedclass
{
public:
    SetClassnameWithParent( "subderivedclass", derivedclass ); // only first 8 chars are registered.

    subderivedclass()
    {
        ClassnameSanityCheck();
    }
};

void TypeInfoTestCode()
{
    baseclass aaa;
    derivedclass bbb;
    subderivedclass ccc;

    bool aaa_is_baseclass = aaa.IsA( "baseclas1234" ); // only comparing 8 chars, so will succeed
    bool aaa_is_derivedclass = aaa.IsA( "derivedclass" );
    bool aaa_is_subderivedclass = aaa.IsA( "subderivedclass" );
    bool bbb_is_baseclass = bbb.IsA( "baseclass" );
    bool bbb_is_derivedclass = bbb.IsA( "derivedclass" );
    bool bbb_is_subderivedclass = bbb.IsA( "subderivedclass" );
    bool ccc_is_baseclass = ccc.IsA( "baseclass" );
    bool ccc_is_derivedclass = ccc.IsA( "derivedclass" );
    bool ccc_is_subderivedclass = ccc.IsA( "subderivedclass" );

    // speed test, comparing doubles vs strcmp.  first test is generally 2x faster on my laptop.
    double starttime = MyTime_GetSystemTime();
    for( int i=0; i<1000000000; i++ )
        bool aaa = *(double*)"classname" == *(double*)"dlassname";
    double endtime = MyTime_GetSystemTime();
    double timediff = endtime - starttime;

    double starttime2 = MyTime_GetSystemTime();
    for( int i=0; i<1000000000; i++ )
        bool aaa = strcmp( "classname", "dlassname" ) == 0;
    double endtime2 = MyTime_GetSystemTime();
    double timediff2 = endtime2 - starttime2;
}

#endif