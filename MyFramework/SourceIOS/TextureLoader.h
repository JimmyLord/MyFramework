//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TEXTURELOADER_H__
#define __TEXTURELOADER_H__

class TextureDefinition;

FILE* IOS_fopen(const char* filepathnameext);

//MyFileObject* RequestFile(const char* filepathnameext);
//
//char* LoadFile(const char* filename, int* length = 0);
//GLuint LoadTexture(const char* filename);
//GLuint LoadTextureFromMemory(TextureDefinition* texturedef);

#endif //__TEXTURELOADER_H__
