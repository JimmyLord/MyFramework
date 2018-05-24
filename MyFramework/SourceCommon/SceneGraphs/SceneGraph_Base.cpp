//
// Copyright (c) 2016-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

#include "SceneGraph_Base.h"

void SceneGraphObject::Clear()
{
    m_Flags = SceneGraphFlag_Opaque;
    m_pMaterial = 0;

#if MYFW_EDITOR
    m_EditorObject = false;
#endif

    m_Layers = 0;
    m_pTransform = 0;
    m_pMesh = 0;
    m_pSubmesh = 0;
    m_Visible = false;

    m_GLPrimitiveType = 0;
    m_PointSize = 0;

    m_pUserData = 0;
}

void SceneGraphObject::SetMaterial(MaterialDefinition* pNewMaterial, bool updateTransparencyFlags)
{
    m_pMaterial = pNewMaterial;

    if( updateTransparencyFlags )
    {
        SceneGraphFlags flags = (SceneGraphFlags)(m_Flags & ~(SceneGraphFlag_Opaque | SceneGraphFlag_Transparent | SceneGraphFlag_Emissive));
        if( pNewMaterial )
        {
            if( pNewMaterial->IsTransparent() )
                flags = (SceneGraphFlags)(flags | SceneGraphFlag_Transparent);
            else
                flags = (SceneGraphFlags)(flags | SceneGraphFlag_Opaque);

            if( pNewMaterial->IsEmissive() )
                flags = (SceneGraphFlags)(flags | SceneGraphFlag_Emissive);
        }
        else
        {
            flags = (SceneGraphFlags)(flags | SceneGraphFlag_Opaque);
        }

        m_Flags = flags;
    }
}

SceneGraph_Base::SceneGraph_Base()
{
    m_pObjectPool.AllocateObjects( 100000 );
}

SceneGraph_Base::~SceneGraph_Base()
{
}

SceneGraphObject* SceneGraph_Base::AddObject(MyMatrix* pTransform, MyMesh* pMesh, MySubmesh* pSubmesh, MaterialDefinition* pMaterial, int primitiveType, int pointSize, unsigned int layers, void* pUserData)
{
    // Add the object with the opaque flag set.
    SceneGraphObject* pSceneGraphObject = AddObjectWithFlagOverride( pTransform, pMesh, pSubmesh, pMaterial, primitiveType, pointSize, SceneGraphFlag_Opaque, layers, pUserData);

    // Set the material again, this time also overwrite the opacity flags with the material setting.
    pSceneGraphObject->SetMaterial( pMaterial, true );

    return pSceneGraphObject;
}

bool SceneGraph_Base::ShouldObjectBeDrawn(SceneGraphObject* pObject, bool drawOpaques, EmissiveDrawOptions emissiveDrawOption, unsigned int layersToRender)
{
    // Forward Passes:
    //   opaque and opaque/emissive
    //   transparent and transparent/emissive

    // Deferred Passes:
    //   deferred -> opaque
    //   forward -> opaque/emissive
    //   forward -> transparent and transparent/emissive

    SceneGraphFlags objectFlags = pObject->GetFlags();

    bool isOpaque      = objectFlags & SceneGraphFlag_Opaque   ? true : false;
    bool isEmissive    = objectFlags & SceneGraphFlag_Emissive ? true : false;

    // If we're drawing opaques and this object is transparent, kick out.
    if( drawOpaques & !isOpaque )
        return false;

    // If we're drawing transparents and this object is opaque, kick out.
    if( !drawOpaques & isOpaque )
        return false;

    // If we don't want to draw emissive objects and this object is emissive, kick out.
    if( emissiveDrawOption == EmissiveDrawOption_NoEmissives && isEmissive )
        return false;

    // If we exclusively want to draw emissive objects and this object is not emissive, kick out.
    if( emissiveDrawOption == EmissiveDrawOption_OnlyEmissives && isEmissive == false )
        return false;

    if( (pObject->m_Layers & layersToRender) == 0 )
        return false;
        
    MyAssert( pObject->m_pSubmesh );

    if( pObject->m_pSubmesh == 0 ) //|| pObject->GetMaterial() == 0 )
        return false;

    if( pObject->m_Visible == false )
        return false;

    return true;
}