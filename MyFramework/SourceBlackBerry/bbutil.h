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

#ifndef _UTILITY_H_INCLUDED
#define _UTILITY_H_INCLUDED

#include <EGL/egl.h>
#include <screen/screen.h>
#include <sys/platform.h>

extern EGLDisplay egl_disp;
extern EGLSurface egl_surf;

typedef struct font_t font_t;

#define BBUTIL_DEFAULT_FONT "/usr/fonts/font_repository/monotype/arial.ttf"

#ifdef __cplusplus
extern "C" {
#endif

// Use the PID to set the window group id.
char* get_window_group_id();

/**
 * Initializes EGL
 *
 * @param libscreen context that will be used for EGL setup
 * @return EXIT_SUCCESS if initialization succeeded otherwise EXIT_FAILURE
 */
int bbutil_init_egl(screen_context_t ctx, int interval);

/**
 * Terminates EGL
 */
void bbutil_terminate();

/**
 * Swaps default bbutil window surface to the screen
 */
void bbutil_swap();

int bbutil_rotate_screen_surface(int angle, int interval);

#ifdef __cplusplus
}
#endif

#endif
