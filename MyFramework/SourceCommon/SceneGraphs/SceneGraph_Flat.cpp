#include "CommonHeader.h"

#include "SceneGraph_Base.h"
#include "SceneGraph_Flat.h"

SceneGraph_Flat::SceneGraph_Flat()
{
    m_NumRenderables = 0;
}

SceneGraph_Flat::~SceneGraph_Flat()
{
    for( unsigned int i=0; i<m_NumRenderables; i++ )
    {
        m_pObjectPool.ReturnObject( m_pRenderables[i] );
    }
}

void SceneGraph_Flat::AddRenderableObject(MyMatrix* pTransform, MyMesh* pMesh, MaterialDefinition* pMaterial)
{
    MyAssert( pTransform != 0 && pMesh != 0 );

    RenderableObject* pObject = m_pObjectPool.GetObject();

    if( pObject )
    {
        MyAssert( m_NumRenderables < 10000 );

        pObject->m_pTransform = pTransform;
        pObject->m_pMesh = pMesh;
        pObject->m_pMaterial = pMaterial;

        m_pRenderables[m_NumRenderables] = pObject;
        m_NumRenderables++;
    }
    else
    {
        LOGInfo( "Scene Graph", "Not enough renderable objects in list\n" );
    }
}

void SceneGraph_Flat::RemoveRenderableObject(MyMatrix* m_pTransform, MyMesh* m_pMesh, MaterialDefinition* m_pMaterial)
{
}

void SceneGraph_Flat::Draw()
{
}
