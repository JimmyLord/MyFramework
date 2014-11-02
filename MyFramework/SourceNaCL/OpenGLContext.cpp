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

#include "OpenGLContext.h"

#include <pthread.h>
#include "MainInstance.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/gles2/gl2ext_ppapi.h"

// This is called by the brower when the 3D context has been flushed to the
// browser window.
void FlushCallback(void* data, int32_t result)
{
    static_cast<OpenGLContext*>(data)->set_flush_pending(false);
    static_cast<OpenGLContext*>(data)->m_pInstance->DrawSelf();
}

OpenGLContext::OpenGLContext(pp::Instance* instance)
: pp::Graphics3DClient(instance)
, m_FlushPending(false)
{
    m_pInstance = (MainInstance*)instance;
    pp::Module* module = pp::Module::Get();
    assert( module != 0 );
    m_GLES2Interface = static_cast<const struct PPB_OpenGLES2*>(module->GetBrowserInterface(PPB_OPENGLES2_INTERFACE));
    assert( m_GLES2Interface );
}

OpenGLContext::~OpenGLContext()
{
    glSetCurrentContextPPAPI(0);
}

bool OpenGLContext::MakeContextCurrent(pp::Instance* instance)
{
    if( instance == 0 )
    {
        glSetCurrentContextPPAPI( 0 );
        return false;
    }

    // Lazily create the Pepper context.
    if( m_Context.is_null() )
    {
        int32_t attribs[] =
        {
            PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
            PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
            PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
            PP_GRAPHICS3DATTRIB_SAMPLES, 0,
            PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
            PP_GRAPHICS3DATTRIB_WIDTH, m_Size.width(),
            PP_GRAPHICS3DATTRIB_HEIGHT, m_Size.height(),
            PP_GRAPHICS3DATTRIB_NONE
        };

        m_Context = pp::Graphics3D(instance, pp::Graphics3D(), attribs);
        if( m_Context.is_null() )
        {
            glSetCurrentContextPPAPI( 0 );
            return false;
        }
        instance->BindGraphics( m_Context );
    }

    glSetCurrentContextPPAPI( m_Context.pp_resource() );

    return true;
}

void OpenGLContext::InvalidateContext(pp::Instance* instance)
{
    glSetCurrentContextPPAPI(0);
}

void OpenGLContext::ResizeContext(const pp::Size& size)
{
    m_Size = size;
    if( !m_Context.is_null() )
    {
        m_Context.ResizeBuffers( size.width(), size.height() );
    }
}

void OpenGLContext::FlushContext()
{
    if( flush_pending() )
    {
        // A flush is pending so do nothing; just drop this flush on the floor.
        return;
    }
    set_flush_pending( true );
    m_Context.SwapBuffers( pp::CompletionCallback( &FlushCallback, this ) );
}
