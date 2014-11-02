//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __FontManager_H__
#define __FontManager_H__

class FontManager;
class TextureDefinition;
class BMFont;

extern FontManager g_FontManager;

class FontDefinition : public CPPListNode
{
public:
    bool m_FullyLoaded;

    char m_FriendlyName[32];
    char m_Filename[MAX_PATH];
    MyFileObject* m_pFile;
    BMFont* m_pFont;

    TextureDefinition* m_pTextureDef;

    FontDefinition()
    {
        m_FullyLoaded = false;
        m_FriendlyName[0] = 0;
        m_Filename[0] = 0;
        m_pFile = 0;
        m_pFont = 0;

        m_pTextureDef = 0;
    }

    virtual ~FontDefinition()
    {
        this->Remove();
        g_pFileManager->FreeFile( m_pFile );
        SAFE_DELETE( m_pFont );
    }
};

class FontManager
{
protected:
    CPPListHead m_FontsLoaded;
    CPPListHead m_FontsStillLoading;

public:
    FontDefinition* CreateFont( const char* fontfilename );
    void Tick();
    FontDefinition* FindFont( const char* friendlyname );
    void FreeAllFonts();
    void InvalidateAllFonts();
};

#endif //__FontManager_H__
