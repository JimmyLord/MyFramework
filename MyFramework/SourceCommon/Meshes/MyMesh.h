//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyMesh_H__
#define __MyMesh_H__

class Shader_Base;
class TextureDefinition;
class BufferDefinition;
class VAODefinition;
class MyLight;
class ShaderGroup;

class MyMesh : public CPPListNode, public RefCount
{
protected:
    ShaderGroup* m_pShaderGroup;

    int m_VertexFormat;
    
    BufferDefinition* m_pVertexBuffer;
    BufferDefinition* m_pIndexBuffer;

public:
    MyFileObject* m_pSourceFile;
    bool m_MeshReady;

    unsigned short m_NumVertsToDraw;
    unsigned int m_NumIndicesToDraw;
    int m_PrimitiveType;

    TextureDefinition* m_pTexture;
    ColorByte m_Tint;
    ColorByte m_SpecColor;
    float m_Shininess;

    MyMatrix m_Position;

public:
    MyMesh();
    virtual ~MyMesh();

    void CreateBuffers(int vertexformat, unsigned short numverts, unsigned int numindices, bool dynamic = false);

    void CreateFromOBJFile(MyFileObject* pFile);

    void CreateBox(float boxw, float boxh, float boxd, float startu, float endu, float startv, float endv, unsigned char justificationflags);
    void CreateBox_XYZUV_RGBA(float boxw, float boxh, float boxd, float startutop, float endutop, float startvtop, float endvtop, float startuside, float enduside, float startvside, float endvside, unsigned char justificationflags);
    void SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR);

    void CreateCylinder(float radius, unsigned short numsegments, float edgeradius, float height, float topstartu, float topendu, float topstartv, float topendv, float sidestartu, float sideendu, float sidestartv, float sideendv);

    void CreateEditorLineGridXZ(Vector3 center, float spacing, int halfnumbars);
    void CreateEditorTransformGizmoAxis(float length, float thickness, ColorByte color);

    ShaderGroup* GetShaderGroup() { return m_pShaderGroup; }
    void SetShaderGroup(ShaderGroup* pShaderGroup);
    void SetShaderAndTexture(ShaderGroup* pShaderGroup, TextureDefinition* pTexture);
    void SetTextureProperties(ColorByte tint, ColorByte speccolor, float shininess);
    void SetPosition(float x, float y, float z);
    void Draw(MyMatrix* matviewproj, Vector3* campos, MyLight* lights, int numlights, MyMatrix* shadowlightwvp, int shadowtexid, int lightmaptexid, ShaderGroup* pShaderOverride);

    void RebuildIndices();

    unsigned short GetNumVerts();
    unsigned int GetNumIndices();
    Vertex_Base* GetVerts(bool markdirty);
    unsigned short* GetIndices(bool markdirty);
};

#endif //__MyMesh_H__
