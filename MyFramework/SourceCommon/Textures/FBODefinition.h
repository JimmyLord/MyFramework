//
// Copyright (c) 2012-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __FBODefinition_H__
#define __FBODefinition_H__

class FBODefinition : public CPPListNode, public RefCount
{
    friend class TextureManager;

    static const int MAX_COLOR_TEXTURES = 4;

public:
    enum FBOColorFormat
    {
        FBOColorFormat_None,
        FBOColorFormat_RGBA_UByte,
#if !MYFW_OPENGLES2
        FBOColorFormat_RGBA_Float16,
        FBOColorFormat_RGB_Float16,
#endif
    };

protected:
    bool m_HasValidResources;
    bool m_FullyLoaded;

    bool m_FailedToInit;
    bool m_OnlyFreeOnShutdown;

    unsigned int m_NumColorTextures;
    TextureDefinition* m_pColorTextures[MAX_COLOR_TEXTURES];
    TextureDefinition* m_pDepthTexture;
    GLuint m_FrameBufferID;

    unsigned int m_Width; // size requested, mainly used by glViewport call.
    unsigned int m_Height;

    unsigned int m_TextureWidth; // generally will be power of 2 bigger than requested width/height
    unsigned int m_TextureHeight;

    int m_MinFilter;
    int m_MagFilter;

    FBOColorFormat m_ColorFormats[MAX_COLOR_TEXTURES];
    int m_DepthBits;
    bool m_DepthIsTexture;

    int m_LastFrameBufferID;

protected: // Limiting access of setup, creation and destruction to TextureManager.
    FBODefinition();
    virtual ~FBODefinition();

    // Returns true if new textures need to be created.
    bool Setup(unsigned int width, unsigned int height, int minFilter, int magFilter, FBOColorFormat colorFormat, int depthBits, bool depthReadable);
    bool Setup(unsigned int width, unsigned int height, int minFilter, int magFilter, FBOColorFormat* colorFormats, int numColorFormats, int depthBits, bool depthReadable);

    bool Create();

    void Invalidate(bool cleanGLAllocs);

public:
    bool IsFullyLoaded() { return m_FullyLoaded; }

    void Bind(bool storeFramebufferID);
    void Unbind(bool restoreLastFramebufferID);

    // Getters
    TextureDefinition* GetColorTexture(int index) { return m_pColorTextures[index]; }
    TextureDefinition* GetDepthTexture() { return m_pDepthTexture; }

    GLuint GetFrameBufferID() { return m_FrameBufferID; }

    unsigned int GetWidth() { return m_Width; }
    unsigned int GetHeight() { return m_Height; }

    unsigned int GetTextureWidth() { return m_TextureWidth; }
    unsigned int GetTextureHeight() { return m_TextureHeight; }
};

#endif //__FBODefinition_H__
