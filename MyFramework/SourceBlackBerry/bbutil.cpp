/*
 * Copyright (c) 2011-2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#include "bbutil.h"

#ifdef USING_GL11
#include <GLES/gl.h>
#include <GLES/glext.h>
#elif defined(USING_GL20)
#include <GLES2/gl2.h>
#else
#error bbutil must be compiled with either USING_GL11 or USING_GL20 flags
#endif

//#include <ft2build.h>
//#include FT_FREETYPE_H

#include "png.h"

EGLDisplay egl_disp;
EGLSurface egl_surf;

static EGLConfig egl_conf;
static EGLContext egl_ctx;

static screen_context_t screen_ctx;
static screen_window_t screen_win;
static screen_display_t screen_disp;
static int nbuffers = 2;
static int initialized = 0;

#ifdef USING_GL20
static GLuint text_rendering_program;
static int text_program_initialized = 0;
static GLint positionLoc;
static GLint texcoordLoc;
static GLint textureLoc;
static GLint colorLoc;
#endif

struct font_t
{
    unsigned int font_texture;
    float pt;
    float advance[128];
    float width[128];
    float height[128];
    float tex_x1[128];
    float tex_x2[128];
    float tex_y1[128];
    float tex_y2[128];
    float offset_x[128];
    float offset_y[128];
    int initialized;
};


static void bbutil_egl_perror(const char *msg)
{
    static const char *errmsg[] =
    {
        "function succeeded",
        "EGL is not initialized, or could not be initialized, for the specified display",
        "cannot access a requested resource",
        "failed to allocate resources for the requested operation",
        "an unrecognized attribute or attribute value was passed in an attribute list",
        "an EGLConfig argument does not name a valid EGLConfig",
        "an EGLContext argument does not name a valid EGLContext",
        "the current surface of the calling thread is no longer valid",
        "an EGLDisplay argument does not name a valid EGLDisplay",
        "arguments are inconsistent",
        "an EGLNativePixmapType argument does not refer to a valid native pixmap",
        "an EGLNativeWindowType argument does not refer to a valid native window",
        "one or more argument values are invalid",
        "an EGLSurface argument does not name a valid surface configured for rendering",
        "a power management event has occurred",
        "unknown error code"
    };

    int message_index = eglGetError() - EGL_SUCCESS;

    if( message_index < 0 || message_index > 14 )
        message_index = 15;

    fprintf(stderr, "%s: %s\n", msg, errmsg[message_index]);
}

// Use the PID to set the window group id.
char* get_window_group_id()
{
    static char s_window_group_id[16] = "";
    if( s_window_group_id[0] == '\0' )
    {
        snprintf( s_window_group_id, sizeof(s_window_group_id), "%d", getpid() );
    }
    return s_window_group_id;
}

int bbutil_init_egl(screen_context_t ctx, int interval)
{
    int usage;
    int format = SCREEN_FORMAT_RGBX8888;
    int rc, num_configs;

    EGLint attrib_list[] = { EGL_RED_SIZE,        8,
                             EGL_GREEN_SIZE,      8,
                             EGL_BLUE_SIZE,       8,
                             EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
                             EGL_RENDERABLE_TYPE, 0,
                             EGL_DEPTH_SIZE,      24,
                             EGL_NONE};

#ifdef USING_GL11
    usage = SCREEN_USAGE_OPENGL_ES1 | SCREEN_USAGE_ROTATION;
    attrib_list[9] = EGL_OPENGL_ES_BIT;
#elif defined(USING_GL20)
    usage = SCREEN_USAGE_OPENGL_ES2 | SCREEN_USAGE_ROTATION;
    attrib_list[9] = EGL_OPENGL_ES2_BIT;
    EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
#else
    fprintf( stderr, "bbutil should be compiled with either USING_GL11 or USING_GL20 -D flags\n" );
    return EXIT_FAILURE;
#endif

    // Simple egl initialization
    screen_ctx = ctx;

    egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if( egl_disp == EGL_NO_DISPLAY )
    {
        bbutil_egl_perror( "eglGetDisplay" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglInitialize( egl_disp, NULL, NULL );
    if( rc != EGL_TRUE )
    {
        bbutil_egl_perror( "eglInitialize" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglBindAPI( EGL_OPENGL_ES_API );
    if( rc != EGL_TRUE )
    {
        bbutil_egl_perror( "eglBindApi" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    if( !eglChooseConfig( egl_disp, attrib_list, &egl_conf, 1, &num_configs ) )
    {
        bbutil_terminate();
        return EXIT_FAILURE;
    }

#ifdef USING_GL20
    egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, attributes);
#elif defined(USING_GL11)
    egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, NULL);
#endif

    if( egl_ctx == EGL_NO_CONTEXT )
    {
        bbutil_egl_perror( "eglCreateContext" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_create_window( &screen_win, screen_ctx );
    if( rc )
    {
        perror( "screen_create_window" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_create_window_group( screen_win, get_window_group_id() );
    if( rc )
    {
        perror("screen_create_window_group");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv( screen_win, SCREEN_PROPERTY_FORMAT, &format );
    if( rc )
    {
        perror( "screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv( screen_win, SCREEN_PROPERTY_USAGE, &usage );
    if( rc )
    {
        perror( "screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_pv( screen_win, SCREEN_PROPERTY_DISPLAY, (void **)&screen_disp );
    if( rc )
    {
        perror( "screen_get_window_property_pv" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    int screen_resolution[2];

    rc = screen_get_display_property_iv( screen_disp, SCREEN_PROPERTY_SIZE, screen_resolution );
    if( rc )
    {
        perror( "screen_get_display_property_iv" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    int angle = atoi( getenv("ORIENTATION") );

    screen_display_mode_t screen_mode;
    rc = screen_get_display_property_pv( screen_disp, SCREEN_PROPERTY_MODE, (void**)&screen_mode );
    if( rc )
    {
        perror( "screen_get_display_property_pv" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    int size[2];
    rc = screen_get_window_property_iv( screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size );
    if( rc )
    {
        perror("screen_get_window_property_iv");
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    int buffer_size[2] = {size[0], size[1]};

    if( (angle == 0) || (angle == 180) )
    {
        if( ((screen_mode.width > screen_mode.height) && (size[0] < size[1])) ||
            ((screen_mode.width < screen_mode.height) && (size[0] > size[1])) )
        {
                buffer_size[1] = size[0];
                buffer_size[0] = size[1];
        }
    }
    else if( (angle == 90) || (angle == 270) )
    {
        if( ((screen_mode.width > screen_mode.height) && (size[0] > size[1])) ||
            ((screen_mode.width < screen_mode.height && size[0] < size[1])) )
        {
                buffer_size[1] = size[0];
                buffer_size[0] = size[1];
        }
    }
    else
    {
         fprintf( stderr, "Navigator returned an unexpected orientation angle.\n" );
         bbutil_terminate();
         return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv( screen_win, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size );
    if( rc )
    {
        perror( "screen_set_window_property_iv" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_set_window_property_iv( screen_win, SCREEN_PROPERTY_ROTATION, &angle );
    if( rc )
    {
        perror( "screen_set_window_property_iv" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = screen_create_window_buffers( screen_win, nbuffers );
    if( rc )
    {
        perror( "screen_create_window_buffers" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    egl_surf = eglCreateWindowSurface( egl_disp, egl_conf, screen_win, NULL );
    if( egl_surf == EGL_NO_SURFACE )
    {
        bbutil_egl_perror( "eglCreateWindowSurface" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglMakeCurrent( egl_disp, egl_surf, egl_surf, egl_ctx );
    if( rc != EGL_TRUE )
    {
        bbutil_egl_perror( "eglMakeCurrent" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    rc = eglSwapInterval( egl_disp, interval );
    if( rc != EGL_TRUE )
    {
        bbutil_egl_perror( "eglSwapInterval" );
        bbutil_terminate();
        return EXIT_FAILURE;
    }

    const char* id = "MyOpenGLWindowID";
    rc = screen_set_window_property_cv(screen_win, SCREEN_PROPERTY_ID_STRING, strlen(id), id );
    if( rc )
    {
        perror( "screen_set_window_property_cv" );
        return EXIT_FAILURE;
    }

    //QString group = Application::instance()->mainWindow()->groupId();
    //if( screen_join_window_group( screen_win, group.toAscii() ) != 0  )
    //{
    //    return EXIT_FAILURE;
    //}

    initialized = 1;

    return EXIT_SUCCESS;
}

void bbutil_terminate()
{
    //Typical EGL cleanup
    if (egl_disp != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (egl_surf != EGL_NO_SURFACE)
        {
            eglDestroySurface(egl_disp, egl_surf);
            egl_surf = EGL_NO_SURFACE;
        }
        if (egl_ctx != EGL_NO_CONTEXT)
        {
            eglDestroyContext(egl_disp, egl_ctx);
            egl_ctx = EGL_NO_CONTEXT;
        }
        if (screen_win != NULL)
        {
            screen_destroy_window(screen_win);
            screen_win = NULL;
        }
        eglTerminate(egl_disp);
        egl_disp = EGL_NO_DISPLAY;
    }
    eglReleaseThread();

    initialized = 0;
}

void bbutil_swap()
{
    int rc = eglSwapBuffers( egl_disp, egl_surf );
    if( rc != EGL_TRUE )
    {
        bbutil_egl_perror( "eglSwapBuffers" );
    }
}

int bbutil_rotate_screen_surface(int angle, int interval)
{
    int rc, rotation, skip = 1, temp;;
    int size[2];

    if ((angle != 0) && (angle != 90) && (angle != 180) && (angle != 270)) {
        fprintf(stderr, "Invalid angle\n");
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_ROTATION, &rotation);
    if (rc) {
        perror("screen_set_window_property_iv");
        return EXIT_FAILURE;
    }

    rc = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
    if (rc) {
        perror("screen_set_window_property_iv");
        return EXIT_FAILURE;
    }

    switch (angle - rotation) {
        case -270:
        case -90:
        case 90:
        case 270:
            temp = size[0];
            size[0] = size[1];
            size[1] = temp;
            skip = 0;
            break;
    }

    if (!skip) {
        rc = eglMakeCurrent(egl_disp, NULL, NULL, NULL);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
            return EXIT_FAILURE;
        }

        rc = eglDestroySurface(egl_disp, egl_surf);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
            return EXIT_FAILURE;
        }

        rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_SOURCE_SIZE, size);
        if (rc) {
            perror("screen_set_window_property_iv");
            return EXIT_FAILURE;
        }

        rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
        if (rc) {
            perror("screen_set_window_property_iv");
            return EXIT_FAILURE;
        }
        egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, NULL);
        if (egl_surf == EGL_NO_SURFACE) {
            bbutil_egl_perror("eglCreateWindowSurface");
            return EXIT_FAILURE;
        }

        rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglMakeCurrent");
            return EXIT_FAILURE;
        }

        rc = eglSwapInterval(egl_disp, interval);
        if (rc != EGL_TRUE) {
            bbutil_egl_perror("eglSwapInterval");
            return EXIT_FAILURE;
        }
    }

    rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_ROTATION, &angle);
    if (rc) {
        perror("screen_set_window_property_iv");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

