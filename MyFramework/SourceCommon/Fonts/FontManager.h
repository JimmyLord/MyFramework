//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __FontManager_H__
#define __FontManager_H__

class BMFont;
class FontManager;
class MyFileObject;
class TextureDefinition;

extern FontManager* g_pFontManager;

class FontDefinition : public TCPPListNode<FontDefinition*>, public RefCount
{
    friend class FontManager;

protected:
    bool m_FullyLoaded;

    char m_FriendlyName[32];
    MyFileObject* m_pFile;
    BMFont* m_pBMFont;

    TextureDefinition* m_pTextureDef;

public:
    FontDefinition();
    virtual ~FontDefinition();

    // Getters.
    bool IsFullyLoaded() { return m_FullyLoaded; }
    MyFileObject* GetFile() { return m_pFile; }
    BMFont* GetBMFont() { return m_pBMFont; }
    TextureDefinition* GetTexture() { return m_pTextureDef; }
};

class FontManager
{
protected:
    TCPPListHead<FontDefinition*> m_FontsLoaded;
    TCPPListHead<FontDefinition*> m_FontsStillLoading;

public:
    FontDefinition* CreateFont(const char* fontfilename);
    FontDefinition* CreateFont(MyFileObject* pFile);
    void Tick();
    FontDefinition* GetFirstFont();
    FontDefinition* FindFont(const char* friendlyname);
    FontDefinition* FindFont(MyFileObject* pFile);
    FontDefinition* FindFontByFilename(const char* fullpath);
    void FreeAllFonts();
};

#endif //__FontManager_H__
