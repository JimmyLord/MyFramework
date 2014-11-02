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

#ifndef __GLExtensions_H__
#define __GLExtensions_H__

void OpenGL_InitExtensions();

// TODO: remove these typedefs and hook in the proper functions.
//       just put these here for now to define the func ptrs to 0.

typedef void (*PFNGLBINDVERTEXARRAYOESPROC) (GLuint array);
typedef void (*PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint *arrays);
typedef void (*PFNGLGENVERTEXARRAYSOESPROC) (GLsizei n, GLuint *arrays);
typedef GLboolean (*PFNGLISVERTEXARRAYOESPROC) (GLuint array);

extern PFNGLBINDVERTEXARRAYOESPROC          glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSOESPROC       glDeleteVertexArrays;
extern PFNGLGENVERTEXARRAYSOESPROC          glGenVertexArrays;
extern PFNGLISVERTEXARRAYOESPROC            glIsVertexArray;

#endif //__GLExtensions_H__
