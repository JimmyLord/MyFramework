//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __TextureManager_H__
#define __TextureManager_H__

class TextureDefinition;
class TextureManager;
class RefCount;
class FBODefinition;

extern TextureManager* g_pTextureManager;

class TextureManager
{
protected:
    CPPListHead m_LoadedTextures;
    CPPListHead m_TexturesStillLoading;
    CPPListHead m_InitializedFBOs;
    CPPListHead m_UninitializedFBOs;
    int m_MaxTexturesToLoadInOneTick;

public:
    TextureManager();
    virtual ~TextureManager();

    void Tick();
    
    FBODefinition* CreateFBO(int width, int height, int minfilter, int magfilter, bool needcolor, int depthbits, bool depthreadable, bool onlyfreeonshutdown = false);
    bool ReSetupFBO(FBODefinition* pFBO, int width, int height, int minfilter, int magfilter, bool needcolor, int depthbits, bool depthreadable);
    void InvalidateFBO(FBODefinition* pFBO);

    TextureDefinition* CreateTexture(const char* texturefilename, int minfilter = GL_NEAREST, int magfilter = GL_NEAREST, int wraps = GL_REPEAT, int wrapt = GL_REPEAT);
    TextureDefinition* CreateTexture(MyFileObject* pFile, int minfilter = GL_NEAREST, int magfilter = GL_NEAREST, int wraps = GL_REPEAT, int wrapt = GL_REPEAT);
    GLuint CreateTextureFromBuffer(TextureDefinition* texturedef);
    TextureDefinition* FindTexture(const char* texturefilename); // Does not add to the TextureDefinition ref count.
    TextureDefinition* FindTexture(const MyFileObject* pFile); // Does not add to the TextureDefinition ref count.

    void FreeAllTextures(bool shuttingdown);
    void InvalidateAllTextures(bool cleanglallocs);

    void SetMaxTexturesToLoadInOneTick(int max) { m_MaxTexturesToLoadInOneTick = max; }
};

#endif //__TextureManager_H__
