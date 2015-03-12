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

struct MySkeletonNode
{
    char* m_Name;
    int m_SkeletonNodeIndex;
    int m_BoneIndex;    
    MyList<MySkeletonNode*> m_pChildren;
    MyMatrix m_Transform;

    MySkeletonNode()
    {
        m_Name = 0;
        m_SkeletonNodeIndex = 0;
        m_BoneIndex = 0;
    }
    ~MySkeletonNode() { delete[] m_Name; }
};

class MyMesh : public CPPListNode, public RefCount
{
    static const unsigned int MAX_ANIMATIONS = 10; // TODO: fix this hardcodedness

protected:
    ShaderGroup* m_pShaderGroup;

    int m_VertexFormat;
    
    BufferDefinition* m_pVertexBuffer;
    BufferDefinition* m_pIndexBuffer;

    MyList<char*> m_BoneNames;
    MyList<MyMatrix> m_BoneOffsetMatrices;
    MyList<MyMatrix> m_BoneFinalMatrices;
    MyList<MySkeletonNode> m_pSkeletonNodeTree;
    MyList<MyAnimationTimeline*> m_pAnimationTimelines;
    MyList<MyAnimation*> m_pAnimations;

    MyFileObject* m_pAnimationControlFile; // a .myaniminfo file that hold control info for the animation data.

public:
    MyFileObject* m_pSourceFile;
    bool m_MeshReady;

    unsigned short m_NumVertsToDraw;
    unsigned int m_NumIndicesToDraw;
    int m_PrimitiveType;
    int m_PointSize;

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
    void CreateFromMyMeshFile(MyFileObject* pFile);

    void CreateBox(float boxw, float boxh, float boxd, float startu, float endu, float startv, float endv, unsigned char justificationflags);
    void CreateBox_XYZUV_RGBA(float boxw, float boxh, float boxd, float startutop, float endutop, float startvtop, float endvtop, float startuside, float enduside, float startvside, float endvside, unsigned char justificationflags);
    void SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR);

    void CreateCylinder(float radius, unsigned short numsegments, float edgeradius, float height, float topstartu, float topendu, float topstartv, float topendv, float sidestartu, float sideendu, float sidestartv, float sideendv);
    void CreatePlane(Vector3 topleftpos, Vector2 size, Vector2Int vertcount, Vector2 uvstart, Vector2 uvrange, bool createtriangles = true);
    void CreateIcosphere(float radius, unsigned int recursionlevel);

    void CreateEditorLineGridXZ(Vector3 center, float spacing, int halfnumbars);
    void CreateEditorTransformGizmoAxis(float length, float thickness, ColorByte color);

    ShaderGroup* GetShaderGroup() { return m_pShaderGroup; }
    void SetShaderGroup(ShaderGroup* pShaderGroup);
    void SetShaderAndTexture(ShaderGroup* pShaderGroup, TextureDefinition* pTexture);
    void SetTextureProperties(ColorByte tint, ColorByte speccolor, float shininess);
    void SetPosition(float x, float y, float z);
    void Draw(MyMatrix* matviewproj, Vector3* campos, MyLight* lights, int numlights, MyMatrix* shadowlightwvp, int shadowtexid, int lightmaptexid, ShaderGroup* pShaderOverride);

    void RebuildAnimationMatrices(unsigned int animindex, double time);
    void RebuildNode(MyAnimationTimeline* pTimeline, float time, unsigned int nodeindex, MyMatrix* pParentTransform);

    int FindBoneIndexByName(char* name);

    void LoadMyMesh(char* buffer, BufferDefinition** ppVBO, BufferDefinition** ppIBO);
    void LoadMyMesh_ReadNode(cJSON* pNode, MySkeletonNode* pParentSkelNode);
    void LoadAnimationControlFile(char* buffer);
#if MYFW_USING_WX
    int m_ControlID_AnimationName[MAX_ANIMATIONS];

    void SaveAnimationControlFile();
    void FillPropertiesWindow(bool clear);

    static void StaticRefreshWatchWindow(void* pObjectPtr) { ((MyMesh*)pObjectPtr)->RefreshWatchWindow(); }
    void RefreshWatchWindow();

    static void StaticOnAddAnimationPressed(void* pObjectPtr) { ((MyMesh*)pObjectPtr)->OnAddAnimationPressed(); }
    void OnAddAnimationPressed();

    static void StaticOnSaveAnimationsPressed(void* pObjectPtr) { ((MyMesh*)pObjectPtr)->OnSaveAnimationsPressed(); }
    void OnSaveAnimationsPressed();

    static void StaticOnValueChanged(void* pObjectPtr, int id, bool finishedchanging) { ((MyMesh*)pObjectPtr)->OnValueChanged( id, finishedchanging ); }
    void OnValueChanged(int id, bool finishedchanging);
#endif

    void RebuildIndices();

    unsigned short GetNumVerts();
    unsigned int GetNumIndices();
    Vertex_Base* GetVerts(bool markdirty);
    unsigned short* GetIndices(bool markdirty);
    unsigned int GetStride();

    unsigned int GetAnimationCount() { return m_pAnimations.Count(); }
};

#endif //__MyMesh_H__
