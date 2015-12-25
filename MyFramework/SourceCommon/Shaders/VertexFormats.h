//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VertexFormats_H__
#define __VertexFormats_H__

enum VertexFormats // ADDING_NEW_VertexFormat
{
    VertexFormat_Invalid = -1,
    VertexFormat_Dynamic,
    VertexFormat_Sprite,
    VertexFormat_XYZ,
    VertexFormat_XYZUV,
    VertexFormat_XYZUV_RGBA,
    VertexFormat_XYZUVNorm,
    VertexFormat_XYZUVNorm_1Bones,
    VertexFormat_XYZUVNorm_2Bones,
    VertexFormat_XYZUVNorm_3Bones,
    VertexFormat_XYZUVNorm_4Bones,
    VertexFormat_XYZNorm,
    VertexFormat_PointSprite,
    VertexFormat_XYZUVNorm_RGBA,
    VertexFormat_XYZUVNorm_RGBA_1Bones,
    VertexFormat_XYZUVNorm_RGBA_2Bones,
    VertexFormat_XYZUVNorm_RGBA_3Bones,
    VertexFormat_XYZUVNorm_RGBA_4Bones,
    VertexFormat_NumFormats,
    VertexFormat_None = VertexFormat_NumFormats,
};

extern unsigned int g_VertexFormatSizes[VertexFormat_NumFormats];

#define MAX_UV_CHANNELS         4
#define MAX_BONE_INFLUENCES     4

struct VertexFormat_Dynamic_Desc
{
    int stride;
    
    int num_position_components;
    int num_uv_channels;
    int num_bone_influences;

    int offset_pos;
    int offset_uv[MAX_UV_CHANNELS];
    int offset_normal;
    int offset_color;
    int offset_tangent;
    int offset_bitangent;
    int offset_boneindex; //[MAX_BONE_INFLUENCES];
    int offset_boneweight; //[MAX_BONE_INFLUENCES];

    VertexFormat_Dynamic_Desc()
    {
        stride = 0;

        num_position_components = 3;
        num_uv_channels = 0;
        num_bone_influences = 0;

        offset_pos = 0;
        for( int i=0; i<MAX_UV_CHANNELS; i++ )
            offset_uv[i] = 0;
        offset_normal = 0;
        offset_color = 0;
        offset_tangent = 0;
        offset_bitangent = 0;
        offset_boneindex = 0;
        offset_boneweight = 0;
    }
};

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

struct Vertex_XYZUV_Alt
{
    Vector3 pos;
    Vector2 uv;
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

struct Vertex_XYZUVNorm_1Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    unsigned char boneindex[1];
    float weight[1];
};

struct Vertex_XYZUVNorm_2Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    unsigned char boneindex[2];
    float weight[2];
};

struct Vertex_XYZUVNorm_3Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    unsigned char boneindex[3];
    float weight[3];
};

struct Vertex_XYZUVNorm_4Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    unsigned char boneindex[4];
    float weight[4];
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

struct Vertex_XYZUVNorm_RGBA
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    ColorByte color;
};

struct Vertex_XYZUVNorm_RGBA_1Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    ColorByte color;
    unsigned char boneindex[1];
    float weight[1];
};

struct Vertex_XYZUVNorm_RGBA_2Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    ColorByte color;
    unsigned char boneindex[2];
    float weight[2];
};

struct Vertex_XYZUVNorm_RGBA_3Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    ColorByte color;
    unsigned char boneindex[3];
    float weight[3];
};

struct Vertex_XYZUVNorm_RGBA_4Bones
{
    Vector3 pos;
    Vector2 uv;
    Vector3 normal;
    ColorByte color;
    unsigned char boneindex[4];
    float weight[4];
};

#endif //__VertexFormats_H__
