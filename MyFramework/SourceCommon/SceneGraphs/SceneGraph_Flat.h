//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __SceneGraph_Flat_H__
#define __SceneGraph_Flat_H__

class SceneGraph_Flat : public SceneGraph_Base
{
protected:
    SceneGraphObject* m_pRenderables[10000];
    unsigned int m_NumRenderables;

public:
    SceneGraph_Flat();
    virtual ~SceneGraph_Flat();

    virtual SceneGraphObject* AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers, void* pUserData);
    virtual void RemoveObject(SceneGraphObject* pObject);

    virtual void Draw(SceneGraphFlags flags, unsigned int layerstorender, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride);
};

#endif //__SceneGraph_Flat_H__