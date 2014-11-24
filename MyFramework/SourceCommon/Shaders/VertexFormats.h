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

#ifndef __VertexFormats_H__
#define __VertexFormats_H__

enum VertexFormats
{
    VertexFormat_Invalid = -1,
    VertexFormat_Sprite,
    VertexFormat_XYZ,
    VertexFormat_XYZUV,
    VertexFormat_XYZUV_RGBA,
    VertexFormat_XYZUVNorm,
    VertexFormat_XYZNorm,
    VertexFormat_PointSprite,
    VertexFormat_NumFormats,
    VertexFormat_None = VertexFormat_NumFormats,
};

extern unsigned int g_VertexFormatSizes[VertexFormat_NumFormats];

struct Vertex_Base
{
};

struct Vertex_Sprite
{
    float x;
    float y;
    float u;
    float v;
};

struct Vertex_XYZ
{
    float x;
    float y;
    float z;
};

struct Vertex_XYZUV
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

struct Vertex_XYZUV_RGBA
{
    float x;
    float y;
    float z;
    float u;
    float v;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Vertex_XYZUV_RGBA_Alt
{
    Vector3 pos;
    Vector2 uv;
    ColorByte col;
};

struct Vertex_XYZUVNorm
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
};

struct Vertex_XYZNorm
{
    Vector3 pos;
    Vector3 normal;
};

struct Vertex_PointSprite
{
    float x;
    float y;
    //float rot;
    float u;
    float v;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    float size;
};

#endif //__VertexFormats_H__
