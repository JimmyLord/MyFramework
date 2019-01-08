//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MyMesh.h"
#include "MySubmesh.h"
#include "../Renderers/BaseClasses/Renderer_Enums.h"
#include "../Renderers/BaseClasses/Renderer_Base.h"
#include "../Renderers/BaseClasses/Shader_Base.h"

MySubmesh::MySubmesh()
{
    m_pMaterial = nullptr;

#if _DEBUG && MYFW_WINDOWS
    m_TriggerBreakpointOnNextDraw = false;
#endif

    m_VertexFormat = -1;

    m_pVertexBuffer = nullptr;
    m_pIndexBuffer = nullptr;

    m_NumVertsToDraw = 0;
    m_NumIndicesToDraw = 0;
    m_PrimitiveType = MyRE::PrimitiveType_Triangles;
    m_PointSize = 1;
}

MySubmesh::~MySubmesh()
{
    SAFE_RELEASE( m_pMaterial );

    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );
}

void MySubmesh::SetMaterial(MaterialDefinition* pMaterial)
{
    if( m_pMaterial == pMaterial )
        return;

    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;
}

unsigned int MySubmesh::GetStride()
{
    return m_pVertexBuffer->GetStride();
}

bool MySubmesh::SetupShader(Shader_Base* pShader, MyMesh* pMesh, MyMatrix* pMatWorld, Vector3* pCamPos, Vector3* pCamRot, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex)
{
    MaterialDefinition* pMaterial = m_pMaterial;
    MyRE::PrimitiveTypes primitiveType = m_PrimitiveType;
    int pointSize = m_PointSize;

    if( pShader->Activate() == false )
    {
        return false; // Shader wasn't ready.
    }

    if( pMaterial == 0 )
    {
        return false;
    }

    pShader->ProgramMaterialProperties( pMaterial->GetTextureColor(), pMaterial->m_ColorDiffuse, pMaterial->m_ColorSpecular, pMaterial->m_Shininess );

    pShader->ProgramUVScaleAndOffset( pMaterial->m_UVScale, pMaterial->m_UVOffset );

    pShader->ProgramExposedUniforms( pMaterial->m_UniformValues );

    pShader->ProgramCamera( pCamPos, pCamRot );

    if( primitiveType == MyRE::PrimitiveType_Points )
        pShader->ProgramPointSize( (float)pointSize );

    if( pShadowTex != nullptr )
    {
        pShader->ProgramShadowLightTexture( pShadowTex );
    }

    if( pLightmapTex != nullptr )
    {
        pShader->ProgramLightmap( pLightmapTex );
    }

    if( pMesh->m_BoneFinalMatrices.Count() > 0 )
    {
        pShader->ProgramBoneTransforms( &pMesh->m_BoneFinalMatrices[0], pMesh->m_BoneFinalMatrices.Count() );
    }

    pShader->ProgramFramebufferSize( (float)g_GLStats.m_CurrentFramebufferWidth, (float)g_GLStats.m_CurrentFramebufferHeight );

    // Enable blending if necessary. TODO: sort draws and only set this once.
    if( pMaterial->IsTransparent( pShader ) )
    {
        g_pRenderer->SetBlendEnabled( true );

        MyRE::BlendFactors srcFactor = pShader->GetShaderBlendFactorSrc();
        MyRE::BlendFactors destFactor = pShader->GetShaderBlendFactorDest();
        g_pRenderer->SetBlendFunc( srcFactor, destFactor );
    }

    if( pMesh->m_pSetupCustomUniformsCallback )
    {
        pMesh->m_pSetupCustomUniformsCallback( pMesh->m_pSetupCustomUniformsObjectPtr, pShader );
    }

    return true; // No errors.
}

void MySubmesh::SetupMeshSpecificShaderUniforms(Shader_Base* pShader, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, MyMatrix* matInverseWorld, Vector3* pCamPos, MyLight** pLightPtrs, int numLights, MyMatrix* shadowLightVP)
{
    pShader->ProgramLights( pLightPtrs, numLights, matInverseWorld );
    pShader->ProgramTransforms( pMatProj, pMatView, pMatWorld );
    pShader->ProgramLocalSpaceCamera( pCamPos, matInverseWorld );

    if( shadowLightVP )
    {
        MyMatrix textureoffsetmat( 0.5f,0,0,0,  0,0.5f,0,0,  0,0,0.5f,0,  0.5f,0.5f,0.5f,1 );
        MyMatrix shadowWVPT = textureoffsetmat * *shadowLightVP * *pMatWorld;
        pShader->ProgramShadowLightTransform( &shadowWVPT );
    }
}

void MySubmesh::SetupAttributes(Shader_Base* pShader)
{
    BufferDefinition* pVertexBuffer = m_pVertexBuffer;
    BufferDefinition* pIndexBuffer = m_pIndexBuffer;

    if( m_pVertexBuffer )
    {
        pShader->SetupAttributes( pVertexBuffer, pIndexBuffer, true );
        pShader->SetupDefaultAttributes( pVertexBuffer );
    }
}

void MySubmesh::Draw(MyMesh* pMesh, MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, Vector3* pCamPos, Vector3* pCamRot, MyLight** pLightPtrs, int numLights, MyMatrix* shadowLightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride, bool hideFromDrawList)
{
#if _DEBUG && MYFW_WINDOWS
    if( m_TriggerBreakpointOnNextDraw )
    {
        __debugbreak();
        m_TriggerBreakpointOnNextDraw = false;
    }
#endif //_DEBUG && MYFW_WINDOWS

    BufferDefinition* pVertexBuffer = m_pVertexBuffer;
    BufferDefinition* pIndexBuffer = m_pIndexBuffer;
    //MaterialDefinition* pMaterial = m_pMaterial;
    int numVertsToDraw = m_NumVertsToDraw;
    int numIndicesToDraw = m_NumIndicesToDraw;
    MyRE::PrimitiveTypes primitiveType = m_PrimitiveType;
    int pointSize = m_PointSize;        

#if MYFW_EDITOR
    MaterialDefinition* pMaterial = m_pMaterial;
    if( pMaterial == nullptr )
    {
        pMaterial = g_pMaterialManager->GetDefaultEditorMaterial();
    }
#else
    MaterialDefinition* pMaterial = m_pMaterial;
    if( pMaterial == nullptr && pShaderOverride == nullptr )
        return;
#endif //MYFW_EDITOR

    if( pIndexBuffer )
    {
        if( numIndicesToDraw == 0 )
            return;
    }
    else if( pVertexBuffer )
    {
        if( numVertsToDraw == 0 )
            return;
    }

    MyMatrix identity;
    if( pMatWorld == nullptr )
    {
        identity.SetIdentity();
        pMatWorld = &identity;
    }

    MyAssert( pVertexBuffer );

    if( pVertexBuffer->IsDirty() )
    {
        MyAssert( numVertsToDraw > 0 );
        pVertexBuffer->Rebuild( 0, numVertsToDraw * GetStride() );
    }
    if( pIndexBuffer && pIndexBuffer->IsDirty() )
    {
        MyAssert( numIndicesToDraw > 0 );
        pIndexBuffer->Rebuild( 0, numIndicesToDraw*pIndexBuffer->GetBytesPerIndex() );
    }
    MyAssert( ( pIndexBuffer == nullptr || pIndexBuffer->IsDirty() == false ) && pVertexBuffer->IsDirty() == false );

    if( pShaderOverride )
    {
        MyRE::IndexTypes IBOType = MyRE::IndexType_Undefined;
        if( pIndexBuffer != nullptr )
        {
            IBOType = pIndexBuffer->GetIBOType();
        }

        //int numboneinfluences = 0;
        //if( pVertexBuffer && pVertexBuffer->m_pFormatDesc )
        //    numboneinfluences = pVertexBuffer->m_pFormatDesc->num_bone_influences;

        // If an override for the shader is sent in, it's already active and doesn't want anything other than position set.
        // Always use 4 bone version.
        // TODO: This might fail with 1-3 bones,
        //       but should work with 0 bones since bone attribs are set to 100% weight on bone 0
        //       and bone 0 transform uniform is set to identity.
        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
        pShader->SetupAttributes( pVertexBuffer, pIndexBuffer, false );
        pShader->ProgramTransforms( pMatProj, pMatView, pMatWorld );

        if( pMesh->m_BoneFinalMatrices.Count() > 0 )
        {
            pShader->ProgramBoneTransforms( &pMesh->m_BoneFinalMatrices[0], pMesh->m_BoneFinalMatrices.Count() );
        }
        else
        {
            MyMatrix identitymat;
            identitymat.SetIdentity();
            pShader->ProgramBoneTransforms( &identitymat, 1 );
        }

        if( pIndexBuffer )
            g_pRenderer->DrawElements( primitiveType, numIndicesToDraw, IBOType, 0, hideFromDrawList );
        else
            g_pRenderer->DrawArrays( primitiveType, 0, numVertsToDraw, hideFromDrawList );

        // Always disable blending.
        g_pRenderer->SetBlendEnabled( false );
    }
    else
    {
        if( pMaterial == nullptr )
            return;

        if( pMaterial->GetShader() == nullptr )
            return;

        int numboneinfluences = 0;
        if( pVertexBuffer && pVertexBuffer->GetFormatDesc() )
            numboneinfluences = pVertexBuffer->GetFormatDesc()->num_bone_influences;

        int numdirlights = 0;
        int numpointlights = 0;
        for( int i=0; i<numLights; i++ )
        {
            switch( pLightPtrs[i]->m_LightType )
            {
            case LightType_Directional: numdirlights++;    break;
            case LightType_Point:       numpointlights++;  break;
            case LightType_Spot:        MyAssert( false ); break;
            case LightType_NumTypes:    MyAssert( false ); break;
            default:                    MyAssert( false ); break;
            }
        }

        Shader_Base* pShader = (Shader_Base*)pMaterial->GetShader()->GlobalPass( numpointlights, numboneinfluences );
        if( pShader )
        {
            bool initialized = SetupShader( pShader, pMesh, pMatWorld, pCamPos, pCamRot, pShadowTex, pLightmapTex );
            if( initialized )
            {
                MyMatrix matInverseWorld = *pMatWorld;
                matInverseWorld.Inverse();
                //bool didinverse = matInverseWorld.Inverse();
                //if( didinverse == false )
                //    LOGError( LOGTag, "Matrix inverse failed\n" );

                SetupAttributes( pShader );
                SetupMeshSpecificShaderUniforms( pShader, pMatProj, pMatView, pMatWorld, &matInverseWorld, pCamPos, pLightPtrs, numLights, shadowLightVP );

                MyRE::IndexTypes IBOType = MyRE::IndexType_Undefined;
                if( pIndexBuffer != nullptr )
                {
                    IBOType = pIndexBuffer->GetIBOType();
                }

                if( pIndexBuffer )
                    g_pRenderer->DrawElements( primitiveType, numIndicesToDraw, IBOType, 0, hideFromDrawList );
                else
                    g_pRenderer->DrawArrays( primitiveType, 0, numVertsToDraw, hideFromDrawList );

                pShader->DeactivateShader( pVertexBuffer, true );

                // Always disable blending.
                g_pRenderer->SetBlendEnabled( false );
            }
        }
    }
}
