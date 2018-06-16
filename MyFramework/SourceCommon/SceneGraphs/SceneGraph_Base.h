//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SceneGraph_Base_H__
#define __SceneGraph_Base_H__

class MyMesh;
class MaterialDefinition;
class SceneGraphObject;

enum SceneGraphFlags
{
    SceneGraphFlag_Opaque       = 0x01,
    SceneGraphFlag_Transparent  = 0x02,
    SceneGraphFlag_Emissive     = 0x04,
};

enum EmissiveDrawOptions
{
    EmissiveDrawOption_NoEmissives,
    EmissiveDrawOption_EitherEmissiveOrNot,
    EmissiveDrawOption_OnlyEmissives,
};

typedef void (*PreDrawCallbackFunctionPtr)(SceneGraphObject* pObject, ShaderGroup* pShaderOverride);

class SceneGraphObject : public CPPListNode
{
    // No variables hold references.
    // If the object is free'd without removing the SceneGraphObject, bad things will happen.

protected:
    // Changing Opacity/Transparency flags should eventually shift the objects from one bucket to another.
    // Changing materials will likely require changing the Opacity/Transparency SceneGraphFlags.
    SceneGraphFlags m_Flags;
    MaterialDefinition* m_pMaterial;
    bool m_WaitingForMaterialToFinishLoading;

#if MYFW_EDITOR
    bool m_EditorObject;
#endif

public:
    unsigned int m_Layers;
    MyMatrix* m_pTransform;
    MyMesh* m_pMesh; // Used for final bone transforms.
    MySubmesh* m_pSubmesh;
    bool m_Visible;

    int m_GLPrimitiveType;
    int m_PointSize;

    void* m_pUserData;

    void Clear();

    // Getters
    SceneGraphFlags GetFlags() { return m_Flags; }
    MaterialDefinition* GetMaterial() { return m_pMaterial; }
    bool IsWaitingForMaterialToFinishLoading() { return m_WaitingForMaterialToFinishLoading; }

    // Setters
    void SetFlags(SceneGraphFlags newFlags) { m_Flags = newFlags; }
    void SetMaterial(MaterialDefinition* pNewMaterial, bool updateTransparencyFlags);

#if MYFW_EDITOR
    void SetAsEditorObject() { m_EditorObject = true; }
    bool IsEditorObject() { return m_EditorObject; }
#endif
};

class SceneGraph_Base
{
protected:
    MySimplePool<SceneGraphObject> m_pObjectPool;

public:
    SceneGraph_Base();
    virtual ~SceneGraph_Base();

    virtual SceneGraphObject* AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitiveType, int pointSize, unsigned int layers, void* pUserData);
    virtual SceneGraphObject* AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitiveType, int pointSize, SceneGraphFlags flags, unsigned int layers, void* pUserData) = 0;
    virtual void RemoveObject(SceneGraphObject* pObject) = 0;

    bool ShouldObjectBeDrawn(SceneGraphObject* pObject, bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender);
    virtual void Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc) = 0;
};

#endif //__SceneGraph_Base_H__