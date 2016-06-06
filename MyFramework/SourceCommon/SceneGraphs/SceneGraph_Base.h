//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
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

enum SceneGraphFlags
{
    SceneGraphFlag_Opaque       = 0x01,
    SceneGraphFlag_Transparent  = 0x02,
};

class SceneGraphObject
{
public:
    // none of these hold references
    // so if the object is free'd without removing the SceneGraphObject, bad things will happen.
    SceneGraphFlags m_Flags;
    MyMatrix* m_pTransform;
    MyMesh* m_pMesh; // used for final bone transforms ATM
    MySubmesh* m_pSubmesh;
    MaterialDefinition* m_pMaterial;
    bool m_Visible;

    int m_GLPrimitiveType;
    int m_PointSize;
};

class SceneGraph_Base
{
public:
    MySimplePool<SceneGraphObject> m_pObjectPool;

public:
    SceneGraph_Base();
    virtual ~SceneGraph_Base();

    virtual SceneGraphObject* AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags) = 0;
    virtual void RemoveObject(SceneGraphObject* pObject) = 0;

    virtual void Draw(SceneGraphFlags flags, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride) = 0;
};

#endif //__SceneGraph_Base_H__