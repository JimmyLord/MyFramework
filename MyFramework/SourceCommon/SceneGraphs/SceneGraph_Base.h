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
};

typedef void (*PreDrawCallbackFunctionPtr)(SceneGraphObject* pObject, ShaderGroup* pShaderOverride);

class SceneGraphObject : public CPPListNode
{
public:
    // none of these hold references
    // so if the object is free'd without removing the SceneGraphObject, bad things will happen.
    SceneGraphFlags m_Flags;
    unsigned int m_Layers;
    MyMatrix* m_pTransform;
    MyMesh* m_pMesh; // used for final bone transforms ATM
    MySubmesh* m_pSubmesh;
    MaterialDefinition* m_pMaterial;
    bool m_Visible;

    int m_GLPrimitiveType;
    int m_PointSize;

    void* m_pUserData;

    void Clear()
    {
        m_Flags = SceneGraphFlag_Opaque;
        m_Layers = 0;
        m_pTransform = 0;
        m_pMesh = 0;
        m_pSubmesh = 0;
        m_pMaterial = 0;
        m_Visible = false;

        m_GLPrimitiveType = 0;
        m_PointSize = 0;

        m_pUserData = 0;
    }
};

class SceneGraph_Base
{
public:
    MySimplePool<SceneGraphObject> m_pObjectPool;

public:
    SceneGraph_Base();
    virtual ~SceneGraph_Base();

    virtual SceneGraphObject* AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitive, int pointsize, SceneGraphFlags flags, unsigned int layers, void* pUserData) = 0;
    virtual void RemoveObject(SceneGraphObject* pObject) = 0;

    virtual void Draw(SceneGraphFlags flags, unsigned int layerstorender, Vector3* campos, Vector3* camrot, MyMatrix* pMatViewProj, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, ShaderGroup* pShaderOverride, PreDrawCallbackFunctionPtr pPreDrawCallbackFunc) = 0;
};

#endif //__SceneGraph_Base_H__