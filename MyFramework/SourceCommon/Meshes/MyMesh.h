//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyMesh_H__
#define __MyMesh_H__

#include "../DataTypes/ColorStructs.h"
#include "../DataTypes/MyAABounds.h"
#include "../Shaders/VertexFormats.h"

class BufferDefinition;
class MaterialDefinition;
class MyAnimation;
class MyAnimationTimeline;
class MyFileObject;
class MyLight;
class MySubmesh;
class Shader_Base;
class ShaderGroup;
class TextureDefinition;

struct MySkeletonNode
{
    char* m_Name;
    int m_SkeletonNodeIndex;
    int m_BoneIndex;
    MyList<MySkeletonNode*> m_pChildren;
    MyMatrix m_Transform;

    MySkeletonNode()
    {
        m_Name = nullptr;
        m_SkeletonNodeIndex = 0;
        m_BoneIndex = 0;
    }
    ~MySkeletonNode() { delete[] m_Name; }
};

typedef void SetupCustomUniformsCallbackFunc(void* pObjectPtr, Shader_Base* pShader);

class MyMesh : public TCPPListNode<MyMesh*>, public RefCount
{
    friend class MySubmesh;

    static const unsigned int MAX_ANIMATIONS = 10; // TODO: Fix this hardcodedness.

protected:
    float m_InitialScale;

    MyAABounds m_AABounds;

    MyList<char*> m_BoneNames;
    MyList<MyMatrix> m_BoneOffsetMatrices;
    MyList<MyMatrix> m_BoneFinalMatrices;
    MyList<MySkeletonNode> m_pSkeletonNodeTree;
    MyList<MyAnimationTimeline*> m_pAnimationTimelines;
    MyList<MyAnimation*> m_pAnimations;

    MyFileObject* m_pAnimationControlFile; // .myaniminfo file that holds control info for the animation data.

    // Custom uniform setup before final draw is called. TODO: replace this with custom uniform array in material.
    SetupCustomUniformsCallbackFunc* m_pSetupCustomUniformsCallback;
    void* m_pSetupCustomUniformsObjectPtr;

    MyList<MySubmesh*> m_SubmeshList;

    MyFileObject* m_pSourceFile;
    bool m_LoadDefaultMaterials;
    bool m_ForceCheckForAnimationFile;
    bool m_MeshReady;

protected:
    // Internal file loading functions.
    void CreateFromOBJFile();
    static void StaticOnFileFinishedLoadingOBJ(void* pObjectPtr, MyFileObject* pFile) { ((MyMesh*)pObjectPtr)->OnFileFinishedLoadingOBJ( pFile ); }
    void OnFileFinishedLoadingOBJ(MyFileObject* pFile);

    void CreateFromMyMeshFile();
    static void StaticOnFileFinishedLoadingMyMesh(void* pObjectPtr, MyFileObject* pFile) { ((MyMesh*)pObjectPtr)->OnFileFinishedLoadingMyMesh( pFile ); }
    void OnFileFinishedLoadingMyMesh(MyFileObject* pFile);

    static void StaticOnFileFinishedLoadingMyAnim(void* pObjectPtr, MyFileObject* pFile) { ((MyMesh*)pObjectPtr)->OnFileFinishedLoadingMyAnim( pFile ); }
    void OnFileFinishedLoadingMyAnim(MyFileObject* pFile);

    virtual void ParseFile();
    void GuessAndAssignAppropriateShader();

public:
    MyMesh();
    virtual ~MyMesh();

    void Clear();

    // Getters.
    unsigned int GetSubmeshListCount() { return m_SubmeshList.Count(); }
    MySubmesh* GetSubmesh(unsigned int index) { return m_SubmeshList[index]; }

    MyFileObject* GetFile() { return m_pSourceFile; }
    bool IsReady() { return m_MeshReady; }

    unsigned short GetNumVerts();
    unsigned int GetNumIndices();
    Vertex_Base* GetVerts(bool markDirty);
    unsigned short* GetIndices(bool markDirty);
    unsigned int GetStride(unsigned int submeshindex);

    unsigned int GetAnimationCount() { return m_pAnimations.Count(); }

    virtual MaterialDefinition* GetMaterial(int submeshindex);
    MyAABounds* GetBounds() { return &m_AABounds; }

    // Setters.
    void SetLoadDefaultMaterials(bool shouldLoad) { m_LoadDefaultMaterials = shouldLoad; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);

    // Submeshes and Buffers.
    void CreateOneSubmeshWithBuffers(VertexFormat_Dynamic_Desc* pVertexFormatDesc, unsigned int numVerts, int bytesPerIndex, unsigned int numIndices, bool dynamic = false);
    void CreateSubmeshes(int numSubmeshes);
    void CreateVertexBuffer(int meshIndex, VertexFormat_Dynamic_Desc* pVertexFormatDesc, unsigned int numVerts, bool dynamic = false);
    void CreateIndexBuffer(int meshIndex, int bytesPerIndex, unsigned int numIndices, bool dynamic = false);

    void SetIndexBuffer(BufferDefinition* pBuffer);
    void SetSourceFile(MyFileObject* pFile);

    void RebuildIndices();

    // Draw.
    virtual void PreDraw() {}
    void Draw(MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, Vector3* campos, Vector3* camrot, MyLight** lightptrs, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride);

    // Animation.
    void RebuildAnimationMatrices(unsigned int animindex, double animtime, unsigned int oldanimindex, double oldanimtime, float perc);
    void RebuildNode(MyAnimationTimeline* pTimeline, float animtime, MyAnimationTimeline* pOldTimeline, float oldanimtime, float perc, unsigned int nodeindex, MyMatrix* pParentTransform);

    // These 5 functions are defined in MyMeshLoader.cpp
    int FindBoneIndexByName(char* name);
    void LoadMyMesh(const char* pBuffer, MyList<MySubmesh*>* pSubmeshList, float scale);
    void LoadMyMesh_ReadNode(cJSON* jNode, MySkeletonNode* pParentSkelNode);
    void LoadAnimationControlFile(const char* pBuffer);
#if MYFW_EDITOR
    void SaveAnimationControlFile();
#endif

    void RegisterSetupCustomUniformsCallback(void* pObjectPtr, SetupCustomUniformsCallbackFunc* pCallback);

    // Shape creation functions.  Defined in MyMeshShapes.cpp
    void CreateClipSpaceQuad(Vector2 maxUV);
    void CreateBox(float boxWidth, float boxHeight, float boxDepth, float startU, float endU, float startV, float endV, unsigned char justificationFlags, Vector3 offset);
    void CreateBox_XYZUV_RGBA(float boxWidth, float boxHeight, float boxDepth, float topStartU, float topEndU, float topStartV, float topEndV, float sideStartU, float sideEndU, float sideStartV, float sideEndV, unsigned char justificationFlags);
    void SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR);

    void CreateCylinder(float radius, unsigned short numSegments, float edgeRadius, float height, float topStartU, float topEndU, float topStartV, float topEndV, float sideStartU, float sideEndU, float sideStartV, float sideEndV);
    void CreatePlane(Vector3 topLeftPos, Vector2 size, Vector2Int vertCount, Vector2 uvStart, Vector2 uvRange, bool createTriangles = true);
    void CreatePlaneUVsNotShared(Vector3 topLeftPos, Vector2 size, Vector2Int vertCount, Vector2 uvStart, Vector2 uvRange, bool createTriangles = true);
    void CreateIcosphere(float radius, unsigned int recursionLevel);
    void Create2DCircle(float radius, unsigned int numberOfSegments);
    void Create2DArc(Vector3 origin, float startAngle, float endAngle, float startRadius, float endRadius, unsigned int numberOfSegments);
    void CreateGrass(Vector3 topLeftPos, Vector2 size, Vector2Int bladeCount, Vector2 bladeSize);

    void CreateEditorLineGridXZ(Vector3 center, float spacing, int halfNumBars);
    void CreateEditorTransformGizmoAxis(float length, float thickness);
};

#endif //__MyMesh_H__
