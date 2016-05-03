//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyMesh_H__
#define __MyMesh_H__

class MaterialDefinition;
class TextureDefinition;
class BufferDefinition;
class MyLight;

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

class MySubmesh
{
    friend class MyMesh;

protected:
    MaterialDefinition* m_pMaterial;

public:
    int m_VertexFormat;

    BufferDefinition* m_pVertexBuffer;
    BufferDefinition* m_pIndexBuffer;

    unsigned short m_NumVertsToDraw;
    unsigned int m_NumIndicesToDraw;
    int m_PrimitiveType;
    int m_PointSize;

    MySubmesh();
    virtual ~MySubmesh();

public:
    MaterialDefinition* GetMaterial() { return m_pMaterial; }
    void SetMaterial(MaterialDefinition* pMaterial);

    virtual void Draw(MyMesh* pMesh, MyMatrix* matworld, MyMatrix* matviewproj, Vector3* campos, Vector3* camrot, MyLight* lights, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride);
};

class MyMesh : public CPPListNode, public RefCount
{
    friend class MySubmesh;

    static const unsigned int MAX_ANIMATIONS = 10; // TODO: fix this hardcodedness

protected:
    float m_InitialScale;

    MyAABounds m_AABounds;

    MyList<char*> m_BoneNames;
    MyList<MyMatrix> m_BoneOffsetMatrices;
    MyList<MyMatrix> m_BoneFinalMatrices;
    MyList<MySkeletonNode> m_pSkeletonNodeTree;
    MyList<MyAnimationTimeline*> m_pAnimationTimelines;
    MyList<MyAnimation*> m_pAnimations;

    MyFileObject* m_pAnimationControlFile; // a .myaniminfo file that hold control info for the animation data.

public:
    MyList<MySubmesh*> m_SubmeshList;

    MyFileObject* m_pSourceFile;
    bool m_ForceCheckForAnimationFile;
    bool m_MeshReady;

protected:
    //MyMatrix m_Transform;

public:
    MyMesh();
    virtual ~MyMesh();

    void Clear();

    void CreateSubmeshes(int numsubmeshes);
    void CreateBuffers(int vertexformat, unsigned short numverts, unsigned int numindices, bool dynamic = false);

    void CreateFromOBJFile(MyFileObject* pFile);
    void CreateFromMyMeshFile(MyFileObject* pFile);
    void ParseFile();
    void GuessAndAssignAppropriateShader();

    void CreateBox(float boxw, float boxh, float boxd, float startu, float endu, float startv, float endv, unsigned char justificationflags);
    void CreateBox_XYZUV_RGBA(float boxw, float boxh, float boxd, float startutop, float endutop, float startvtop, float endvtop, float startuside, float enduside, float startvside, float endvside, unsigned char justificationflags);
    void SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR);

    void CreateCylinder(float radius, unsigned short numsegments, float edgeradius, float height, float topstartu, float topendu, float topstartv, float topendv, float sidestartu, float sideendu, float sidestartv, float sideendv);
    void CreatePlane(Vector3 topleftpos, Vector2 size, Vector2Int vertcount, Vector2 uvstart, Vector2 uvrange, bool createtriangles = true);
    void CreateIcosphere(float radius, unsigned int recursionlevel);
    void Create2DCircle(float radius, unsigned int numberofsegments);

    void CreateEditorLineGridXZ(Vector3 center, float spacing, int halfnumbars);
    void CreateEditorTransformGizmoAxis(float length, float thickness, ColorByte color);

    virtual MaterialDefinition* GetMaterial(int submeshindex);
    MyAABounds* GetBounds() { return &m_AABounds; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);
    //void SetPosition(float x, float y, float z);
    //void SetTransform(MyMatrix& matrix);
    void Draw(MyMatrix* matworld, MyMatrix* matviewproj, Vector3* campos, Vector3* camrot, MyLight* lights, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride);

    void RebuildAnimationMatrices(unsigned int animindex, double animtime, unsigned int oldanimindex, double oldanimtime, float perc);
    void RebuildNode(MyAnimationTimeline* pTimeline, float animtime, MyAnimationTimeline* pOldTimeline, float oldanimtime, float perc, unsigned int nodeindex, MyMatrix* pParentTransform);

    int FindBoneIndexByName(char* name);

    void LoadMyMesh(char* buffer, MyList<MySubmesh*>* pSubmeshList, float scale);
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

    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging, double oldvalue) { ((MyMesh*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);
#endif

    void RebuildIndices();

    unsigned short GetNumVerts();
    unsigned int GetNumIndices();
    Vertex_Base* GetVerts(bool markdirty);
    unsigned short* GetIndices(bool markdirty);
    unsigned int GetStride(unsigned int submeshindex);

    unsigned int GetAnimationCount() { return m_pAnimations.Count(); }
};

#endif //__MyMesh_H__
