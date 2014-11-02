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

#ifndef __DXWRAPPER_H__
#define __DXWRAPPER_H__

#define GLvoid void
#define GLboolean bool
#define GLubyte unsigned char
#define GLchar char
#define GLushort unsigned short
#define GLuint unsigned int
#define GLint int
#define GLfloat float
#define GLenum int
#define GLsizei int
#define GLint64EXT int
#define GLbitfield unsigned int
#define GLclampf float
#define GLsizeiptr int // ?!?
#define GLintptr int // ?!?

#define GL_FALSE                0
#define GL_TRUE                 1

#define GL_NEAREST              0x2600
#define GL_LINEAR               0x2601

#define GL_UNSIGNED_BYTE        0x1401
#define GL_UNSIGNED_SHORT       0x1403
#define GL_FLOAT                0x1406

#define GL_FRAGMENT_SHADER      0x8B30
#define GL_VERTEX_SHADER        0x8B31

#define GL_POINTS               0x0000
#define GL_TRIANGLES            0x0004

#define GL_ONE                  1
#define GL_SRC_ALPHA            0x0302
#define GL_ONE_MINUS_SRC_ALPHA  0x0303

#define GL_ARRAY_BUFFER         0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define GL_TEXTURE0             0x84C0

#define GL_TEXTURE_2D           0x0DE1

#define GL_TEXTURE_MIN_FILTER   0x2801
#define GL_TEXTURE_MAG_FILTER   0x2800

#define GL_DEPTH_TEST           0x0B71
#define GL_BLEND                0x0BE2

#define GL_CLAMP                0x2900
#define GL_REPEAT               0x2901
#define GL_CLAMP_TO_BORDER      0x812D
#define GL_CLAMP_TO_EDGE        0x812F

#define GL_DEPTH_BUFFER_BIT     0x00000100
#define GL_COLOR_BUFFER_BIT     0x00004000

#define GL_COMPILE_STATUS       0x8B81
#define GL_LINK_STATUS          0x8B82

#define GL_STATIC_DRAW          0x88E4
#define GL_STATIC_READ          0x88E5
#define GL_STATIC_COPY          0x88E6
#define GL_DYNAMIC_DRAW         0x88E8
#define GL_DYNAMIC_READ         0x88E9
#define GL_DYNAMIC_COPY         0x88EA

#define GL_COLOR_INDEX          0x1900
#define GL_STENCIL_INDEX        0x1901
#define GL_DEPTH_COMPONENT      0x1902
#define GL_RED                  0x1903
#define GL_GREEN                0x1904
#define GL_BLUE                 0x1905
#define GL_ALPHA                0x1906
#define GL_RGB                  0x1907
#define GL_RGBA                 0x1908
#define GL_LUMINANCE            0x1909
#define GL_LUMINANCE_ALPHA      0x190A

#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <memory>

#define MAX_SHADERS             1000
//#define MAX_VERTEX_SHADERS      1000
//#define MAX_FRAGMENT_SHADERS    1000
#define MAX_SHADER_PROGRAMS     1000
#define MAX_BUFFERS             1000
#define MAX_TEXTURES            1000

struct GenericBuffer
{
    bool m_InUse;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
};

struct GenericTextures
{
    bool m_InUse;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ResourceView;
};

extern GenericBuffer g_D3DBufferObjects[MAX_BUFFERS];
extern GenericTextures g_D3DTextures[MAX_TEXTURES];

extern Microsoft::WRL::ComPtr<ID3D11Device1> g_pD3DDevice;
extern Microsoft::WRL::ComPtr<ID3D11DeviceContext1> g_pD3DContext;
extern Microsoft::WRL::ComPtr<ID3D11RenderTargetView> g_pD3DRenderTargetView;
extern Microsoft::WRL::ComPtr<ID3D11DepthStencilView> g_pD3DDepthStencilView;

extern Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pD3DSampleStateNearestWrap;
extern Microsoft::WRL::ComPtr<ID3D11SamplerState> g_pD3DSampleStateLinearWrap;

extern Microsoft::WRL::ComPtr<ID3D11BlendState> g_pD3DBlendStateDisabled;
extern Microsoft::WRL::ComPtr<ID3D11BlendState> g_pD3DBlendStateEnabled;
extern Microsoft::WRL::ComPtr<ID3D11BlendState> g_pD3DBlendStateEnabledAdditive;

extern Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteEnabled;
extern Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteEnabled;
extern Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestEnabled_DepthWriteDisabled;
extern Microsoft::WRL::ComPtr<ID3D11DepthStencilState> g_pD3DDepthStencilState_DepthTestDisabled_DepthWriteDisabled;

void DXWrapper_InitDeviceAndContext( Microsoft::WRL::ComPtr<ID3D11Device1> pDevice,
                                     Microsoft::WRL::ComPtr<ID3D11DeviceContext1> pContext,
                                     Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTargetView,
                                     Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView );
void DXWrapper_InitShadersAndBuffers();

void DXWrapper_CleanupDeviceAndContext();
void DXWrapper_CleanupShadersAndBuffers();

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glClear(GLbitfield mask);

GLuint glCreateShader(GLenum type);
void glShaderSource(GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
void glCompileShader(GLuint shader);

GLuint glCreateProgram();
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);

void glGetShaderiv(GLuint shader, GLenum pname, GLint* param);
void glGetProgramiv(GLuint program, GLenum pname, GLint* param);

void glUseProgram(GLuint program);

void glDeleteProgram(GLuint program);
void glDeleteShader(GLuint shader);
void glDetachShader(GLuint program, GLuint shader);

void glDeleteTextures(GLsizei n, const GLuint* textures);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void glDeleteBuffers(GLsizei n, const GLuint* buffers);

void glBlendFunc(GLenum sfactor, GLenum dfactor);

GLint glGetAttribLocation(GLuint program, const GLchar* name);
GLint glGetUniformLocation(GLuint program, const GLchar* name);

void glGenBuffers(GLsizei n, GLuint* buffers);
void glBindBuffer(GLenum target, GLuint buffer);
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);

void glEnableVertexAttribArray(GLuint index);
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void glUniform1f(GLint location, GLfloat v0);
void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void glUniform1i(GLint location, GLint64EXT x);

void glActiveTexture(GLenum texture);
void glBindTexture(GLenum unit, GLenum value);
void glTexParameteri(GLenum target, GLenum pname, GLint param);

void glDisableVertexAttribArray(GLuint index);

void glEnable(GLuint id);
void glDisable(GLuint id);
void glDepthMask(GLboolean flag);

void glDrawArrays(GLenum mode, GLint first, GLsizei count);

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);

#endif
