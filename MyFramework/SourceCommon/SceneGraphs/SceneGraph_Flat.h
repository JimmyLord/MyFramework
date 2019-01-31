//
// Copyright (c) 2016-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SceneGraph_Flat_H__
#define __SceneGraph_Flat_H__

#include "SceneGraph_Base.h"
#include "../Renderers/BaseClasses/Renderer_Enums.h"

class MaterialDefinition;
class MyMatrix;
class MyMesh;
class MySubmesh;
class SceneGraphObject;
class ShaderGroup;
class Vector3;

class SceneGraph_Flat : public SceneGraph_Base
{
protected:
    TCPPListHead<SceneGraphObject*> m_Renderables;
    unsigned int m_NumRenderables;

public:
    SceneGraph_Flat();
    virtual ~SceneGraph_Flat();

    virtual SceneGraphObject* AddObjectWithFlagOverride(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, MyRE::PrimitiveTypes primitiveType, int pointSize, SceneGraphFlags flags, unsigned int layers, void* pUserData) override;
    virtual void RemoveObject(SceneGraphObject* pObject) override;

    virtual void ObjectMoved(SceneGraphObject* pObject) override;

    virtual void Draw(bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender, Vector3* camPos, Vector3* camRot, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr* pPreDrawCallbackFunc) override;
};

#endif //__SceneGraph_Flat_H__