//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __RenderGraph_Base_H__
#define __RenderGraph_Base_H__

#include "../DataTypes/MyActivePool.h"
#include "../Renderers/BaseClasses/Renderer_Enums.h"

class GameCore;
class MaterialDefinition;
class MyMesh;
class MySubmesh;
class RenderGraphObject;
class ShaderGroup;
class TextureDefinition;

enum RenderGraphFlags
{
    RenderGraphFlag_Opaque       = 0x01,
    RenderGraphFlag_Transparent  = 0x02,
    RenderGraphFlag_Emissive     = 0x04,
};

enum EmissiveDrawOptions
{
    EmissiveDrawOption_NoEmissives,
    EmissiveDrawOption_EitherEmissiveOrNot,
    EmissiveDrawOption_OnlyEmissives,
};

typedef void PreDrawCallbackFunctionPtr(RenderGraphObject* pObject, ShaderGroup* pShaderOverride);

class RenderGraphObject : public TCPPListNode<RenderGraphObject*>
{
    // No variables hold references.
    // If the object is free'd without removing the RenderGraphObject, bad things will happen.

protected:
    // Changing Opacity/Transparency flags should eventually shift the objects from one bucket to another.
    // Changing materials will likely require changing the Opacity/Transparency RenderGraphFlags.
    RenderGraphFlags m_Flags;
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

    MyRE::PrimitiveTypes m_GLPrimitiveType;
    int m_PointSize;

    void* m_pUserData;

    void Clear();

    void ObjectMoved();

    // Getters.
    RenderGraphFlags GetFlags() { return m_Flags; }
    MaterialDefinition* GetMaterial() { return m_pMaterial; }
    bool IsWaitingForMaterialToFinishLoading() { return m_WaitingForMaterialToFinishLoading; }

    // Setters.
    void SetFlags(RenderGraphFlags newFlags) { m_Flags = newFlags; }
    void SetMaterial(MaterialDefinition* pNewMaterial, bool updateTransparencyFlags);

#if MYFW_EDITOR
    void SetAsEditorObject() { m_EditorObject = true; }
    bool IsEditorObject() { return m_EditorObject; }
#endif
};

class RenderGraph_Base
{
protected:
    GameCore* m_pGameCore;
    MySimplePool<RenderGraphObject> m_pObjectPool;

public:
    RenderGraph_Base(GameCore* pGameCore);
    virtual ~RenderGraph_Base();

    virtual RenderGraphObject* AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, unsigned int layers, void* pUserData);
    virtual RenderGraphObject* AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, RenderGraphFlags flags, unsigned int layers, void* pUserData) = 0;
    virtual void RemoveObject(RenderGraphObject* pObject) = 0;

    virtual void ObjectMoved(RenderGraphObject* pObject) = 0;

    bool ShouldObjectBeDrawn(RenderGraphObject* pObject, bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender);
    virtual void Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc) = 0;
};

#endif //__RenderGraph_Base_H__