#ifndef __SceneGraph_Flat_H__
#define __SceneGraph_Flat_H__

class SceneGraph_Flat : public SceneGraph_Base
{
protected:
    RenderableObject* m_pRenderables[10000];
    unsigned int m_NumRenderables;

public:
    SceneGraph_Flat();
    virtual ~SceneGraph_Flat();

    virtual void AddRenderableObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial);
    virtual void RemoveRenderableObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial);

    virtual void Draw();
};

#endif //__SceneGraph_Flat_H__