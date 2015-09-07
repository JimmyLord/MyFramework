//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TypeInfo_H__
#define __TypeInfo_H__

// concept from http://www.flipcode.com/archives/Simple_RTTI_For_C.shtml
// Simple RTTI, will compare first 8 chars as a uint64_t
// Also defines ClassnameSanityCheck(), meant to be dropped in the constructor of any defining SetClassNameX
//      this func will check for duplicate classnames or classnames with a length < 8 characters long.

#if MYFW_WINDOWS && _DEBUG

void TypeInfoTestCode();

void RegisterClassname(const char* name);

#define SetClassnameBase(name) \
 virtual void ClassnameSanityCheck() { static bool ClassnameHasBeenRegistered = false; if( ClassnameHasBeenRegistered == false ) RegisterClassname(name); ClassnameHasBeenRegistered = true; } \
 virtual const char* GetClassname() { return name; } \
 virtual bool IsA(const char* classname) { return( *(uint64_t*)classname == *(uint64_t*)name ? true : false ); }

#define SetClassnameWithParent(name,parent) \
 virtual void ClassnameSanityCheck() { static bool ClassnameHasBeenRegistered = false; if( ClassnameHasBeenRegistered == false ) RegisterClassname(name); ClassnameHasBeenRegistered = true; } \
 virtual const char* GetClassname() { return name; } \
 virtual bool IsA(const char* classname) { return( *(uint64_t*)classname == *(uint64_t*)name ? true : parent::IsA(classname) ); }

#define SetClassnameWith2Parents(name,parent1,parent2) \
 virtual void ClassnameSanityCheck() { static bool ClassnameHasBeenRegistered = false; if( ClassnameHasBeenRegistered == false ) RegisterClassname(name); ClassnameHasBeenRegistered = true; } \
 virtual const char* GetClassname() { return name; } \
 virtual bool IsA(const char* classname) { return( *(uint64_t*)classname == *(uint64_t*)name ? true : (parent1::IsA(classname) || parent2::IsA(classname)) ); }

#else

#define ClassnameSanityCheck() ((void)0)

#define SetClassnameBase(name) \
 virtual const char* GetClassname() { return name; } \
 virtual bool IsA(const char* classname) { return( *(uint64_t*)classname == *(uint64_t*)name ? true : false ); }

#define SetClassnameWithParent(name,parent) \
 virtual const char* GetClassname() { return name; } \
 virtual bool IsA(const char* classname) { return( *(uint64_t*)classname == *(uint64_t*)name ? true : parent::IsA(classname) ); }

#define SetClassnameWith2Parents(name,parent1,parent2) \
 virtual const char* GetClassname() { return name; } \
 virtual bool IsA(const char* classname) { return( *(uint64_t*)classname == *(uint64_t*)name ? true : (parent1::IsA(classname) || parent2::IsA(classname)) ); }

#endif

#endif //__TypeInfo_H__
