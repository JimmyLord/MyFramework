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

#ifndef __FBODefinition_H__
#define __FBODefinition_H__

class FBODefinition : public CPPListNode, public RefCount
{
    friend class TextureManager;

protected:

public:
    bool m_FullyLoaded;
    bool m_FailedToInit;

    GLuint m_ColorTextureID; // might not exist depending on setup.
    GLuint m_DepthBufferID; // could be renderbuffer or texture depending on setup.
    GLuint m_FrameBufferID;

    unsigned int m_Width; // size requested, mainly used by glViewport call.
    unsigned int m_Height;

    unsigned int m_TextureWidth; // generally will be power of 2 bigger than requested width/height
    unsigned int m_TextureHeight;

    int m_MinFilter;
    int m_MagFilter;

    bool m_NeedColorTexture;
    bool m_NeedDepthTexture;
    bool m_DepthIsTexture;

protected:
    bool Create(); // TextureManager will call this.

public:
    FBODefinition();
    virtual ~FBODefinition();

    void Setup(unsigned int width, unsigned int height, int minfilter, int magfilter, bool needcolor, bool needdepth, bool depthreadable);

    void Bind();
    void Unbind();

    void Invalidate(bool cleanglallocs);
};

#endif //__FBODefinition_H__
