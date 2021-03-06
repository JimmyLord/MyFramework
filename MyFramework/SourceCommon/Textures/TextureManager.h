//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TextureManager_H__
#define __TextureManager_H__

#include "FBODefinition.h"

class FBODefinition;
class GameCore;
class RefCount;
class TextureDefinition;
class TextureManager;

class TextureManager
{
#if MYFW_USING_IMGUI
    friend class EditorMainFrame_ImGui;
#endif

protected:
    GameCore* m_pGameCore;

    TCPPListHead<TextureDefinition*> m_LoadedTextures;
    TCPPListHead<TextureDefinition*> m_TexturesStillLoading;
    TCPPListHead<FBODefinition*> m_InitializedFBOs;
    TCPPListHead<FBODefinition*> m_UninitializedFBOs;
    int m_MaxTexturesToLoadInOneTick;

    TextureDefinition* m_pErrorTexture;

public:
    TextureManager(GameCore* pGameCore);
    virtual ~TextureManager();

    void Tick();
    
    FBODefinition* CreateFBO(int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat colorFormat, int depthBits, bool depthReadable, bool onlyFreeOnShutdown = false);
    FBODefinition* CreateFBO(int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat* colorformats, int numcolorformats, int depthBits, bool depthReadable, bool onlyFreeOnShutdown = false);
    bool ReSetupFBO(FBODefinition* pFBO, int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat colorFormat, int depthBits, bool depthReadable);
    bool ReSetupFBO(FBODefinition* pFBO, int width, int height, MyRE::MinFilters minFilter, MyRE::MagFilters magFilter, FBODefinition::FBOColorFormat* colorFormats, int numColorFormats, int depthBits, bool depthReadable);
    void InvalidateFBO(FBODefinition* pFBO);

    TextureDefinition* CreateTexture(const char* textureFilename, MyRE::MinFilters minFilter = MyRE::MinFilter_Nearest, MyRE::MagFilters magFilter = MyRE::MagFilter_Nearest, MyRE::WrapModes wrapS = MyRE::WrapMode_Repeat, MyRE::WrapModes wrapT = MyRE::WrapMode_Repeat);
    TextureDefinition* CreateTexture(MyFileObject* pFile, MyRE::MinFilters minFilter = MyRE::MinFilter_Nearest, MyRE::MagFilters magFilter = MyRE::MagFilter_Nearest, MyRE::WrapModes wrapS = MyRE::WrapMode_Repeat, MyRE::WrapModes wrapT = MyRE::WrapMode_Repeat);
    TextureDefinition* FindTexture(const char* textureFilename); // Does not add to the TextureDefinition ref count.
    TextureDefinition* FindTexture(const MyFileObject* pFile); // Does not add to the TextureDefinition ref count.

    void FreeAllTextures(bool shuttingDown);
    void InvalidateAllTextures(bool cleanGLAllocs);

    void SetMaxTexturesToLoadInOneTick(int max) { m_MaxTexturesToLoadInOneTick = max; }

    TextureDefinition* GetErrorTexture();
};

#endif //__TextureManager_H__
