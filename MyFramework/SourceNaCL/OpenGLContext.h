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

#ifndef __OpenGLContext_H__
#define __OpenGLContext_H__

///
/// @file
/// OpenGLContext manages the OpenGL context in the browser that is associated
/// with a @a pp::Instance instance.
///

#include <assert.h>
#include <pthread.h>

#include <algorithm>
#include <string>

#include "ppapi/c/ppb_opengles2.h"
#include "ppapi/cpp/graphics_3d_client.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/size.h"

class MainInstance;

#if !MYFW_PPAPI
#include <tr1/memory>
#endif //!MYFW_PPAPI

class OpenGLContext;
typedef std::tr1::shared_ptr<OpenGLContext> SharedOpenGLContext;

/// OpenGLContext manages an OpenGL rendering context in the browser.
///
class OpenGLContext : public pp::Graphics3DClient
{
public:
    explicit OpenGLContext(pp::Instance* instance);

    /// Release all the in-browser resources used by this context, and make this
    /// context invalid.
    virtual ~OpenGLContext();

    /// The Graphics3DClient interfcace.
    virtual void Graphics3DContextLost()
    {
        assert(!"Unexpectedly lost graphics context");
    }

    /// Make @a this the current 3D context in @a instance.
    /// @param instance The instance of the NaCl module that will receive the
    ///                 the current 3D context.
    /// @return success.
    bool MakeContextCurrent(pp::Instance* instance);

    /// Flush the contents of this context to the browser's 3D device.
    void FlushContext();

    /// Make the underlying 3D device invalid, so that any subsequent rendering
    /// commands will have no effect.  The next call to MakeContextCurrent() will
    /// cause the underlying 3D device to get rebound and start receiving
    /// receiving rendering commands again.  Use InvalidateContext(), for
    /// example, when resizing the context's viewing area.
    void InvalidateContext(pp::Instance* instance);

    /// Resize the context.
    void ResizeContext(const pp::Size& size);

    /// The OpenGL ES 2.0 interface.
    const struct PPB_OpenGLES2* gles2() const
    {
        return m_GLES2Interface;
    }

    /// The PP_Resource needed to make GLES2 calls through the Pepper interface.
    const PP_Resource gl_context() const
    {
        return m_Context.pp_resource();
    }

    /// Indicate whether a flush is pending.  This can only be called from the
    /// main thread; it is not thread safe.
    bool flush_pending() const
    {
        return m_FlushPending;
    }
    void set_flush_pending(bool flag)
    {
        m_FlushPending = flag;
    }

private:
    pp::Size m_Size;
    pp::Graphics3D m_Context;
    bool m_FlushPending;

    const struct PPB_OpenGLES2* m_GLES2Interface;

public:
    MainInstance* m_pInstance;
};

#endif //__OpenGLContext_H__
