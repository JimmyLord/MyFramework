//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __GLHelpers_H__
#define __GLHelpers_H__

void printGLString(const char* name, GLenum s);
bool checkGlErrorFunc(const char* op, const char* file, int line);
#if _DEBUG
#define checkGlError(op) checkGlErrorFunc(op, __FILE__, __LINE__)
#else
#define checkGlError(op)
#endif

GLint GetAttributeLocation(GLuint programhandle, const char* name, ...);
GLint GetUniformLocation(GLuint programhandle, const char* name, ...);

#if MYFW_WINDOWS && MY_SHITTY_LAPTOP
#define VERTEXPREDEFINES "#define VertexShader 1\n#define WIN32 1\n#define MY_SHITTY_LAPTOP 1\n"
#define FRAGMENTPREDEFINES "#define FragmentShader 1\n#define WIN32 1\n#define MY_SHITTY_LAPTOP 1\n"
#elif MY_SHITTY_LAPTOP
#define VERTEXPREDEFINES "#define VertexShader 1\n#define MY_SHITTY_LAPTOP 1\n"
#define FRAGMENTPREDEFINES "#define FragmentShader 1\n#define MY_SHITTY_LAPTOP 1\n"
#elif MYFW_WINDOWS
#define VERTEXPREDEFINES "#define VertexShader 1\n#define WIN32 1\n"
#define FRAGMENTPREDEFINES "#define FragmentShader 1\n#define WIN32 1\n"
#else
#define VERTEXPREDEFINES "#define VertexShader 1\n"
#define FRAGMENTPREDEFINES "#define FragmentShader 1\n"
#endif

GLuint loadShader(GLenum shaderType, int numchunks, const char** ppChunks, int* pLengths);
GLuint loadShader(GLenum shaderType, const char* pPreSource, int presourcelen, const char* pSource, int sourcelen, const char* pPassDefine);
GLuint createProgram(GLuint* vsid, GLuint* fsid, int prevslen, const char* pPreVertexSource, int prefslen, const char* pPreFragmentSource, int numchunks, const char** ppChunks, int* pLengths);
GLuint createProgram(int vslen, const char* pVertexSource, int fslen, const char* pFragmentSource, GLuint* vsid, GLuint* fsid, int prevslen = strlen(VERTEXPREDEFINES), const char* pPreVertexSource = VERTEXPREDEFINES, int prefslen = strlen(FRAGMENTPREDEFINES), const char* pPreFragmentSource = FRAGMENTPREDEFINES, const char* pPassDefine = 0);

#endif //__GLHelpers_H__
