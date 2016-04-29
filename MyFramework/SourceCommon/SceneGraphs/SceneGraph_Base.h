#ifndef __SceneGraph_Base_H__
#define __SceneGraph_Base_H__

class MyMesh;
class MaterialDefinition;

struct RenderableObject
{
    MyMatrix* m_pTransform;
    MyMesh* m_pMesh;
    MaterialDefinition* m_pMaterial;
};

class SceneGraph_Base
{
public:
    MySimplePool<RenderableObject> m_pObjectPool;

public:
    SceneGraph_Base();
    virtual ~SceneGraph_Base();

    virtual void AddRenderableObject(MyMatrix* pTransform, MyMesh* pMesh, MaterialDefinition* pMaterial) = 0;
    virtual void RemoveRenderableObject(MyMatrix* pTransform, MyMesh* pMesh, MaterialDefinition* pMaterial) = 0;

    virtual void Draw() = 0;
};

#endif //__SceneGraph_Base_H__