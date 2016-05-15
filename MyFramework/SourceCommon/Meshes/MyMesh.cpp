//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MyMesh.h"

MySubmesh::MySubmesh()
{
    m_pMaterial = 0;

    m_VertexFormat = -1;

    m_pVertexBuffer = 0;
    m_pIndexBuffer = 0;

    m_NumVertsToDraw = 0;
    m_NumIndicesToDraw = 0;
    m_PrimitiveType = GL_TRIANGLES;
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

void MySubmesh::Draw(MyMesh* pMesh, MyMatrix* matworld, MyMatrix* matviewproj, Vector3* campos, Vector3* camrot, MyLight* lights, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride)
{
    BufferDefinition* pVertexBuffer = m_pVertexBuffer;
    BufferDefinition* pIndexBuffer = m_pIndexBuffer;
    MaterialDefinition* pMaterial = m_pMaterial;
    int NumVertsToDraw = m_NumVertsToDraw;
    int NumIndicesToDraw = m_NumIndicesToDraw;
    int VertexFormat = m_VertexFormat;
    int PrimitiveType = m_PrimitiveType;
    int PointSize = m_PointSize;        

    if( pMaterial == 0 )
        return;

    if( pIndexBuffer )
    {
        if( NumIndicesToDraw == 0 )
            return;
    }
    else if( pVertexBuffer )
    {
        if( NumVertsToDraw == 0 )
            return;
    }

    MyMatrix identity;
    if( matworld == 0 )
    {
        identity.SetIdentity();
        matworld = &identity;
    }

    MyAssert( pVertexBuffer );

    if( pVertexBuffer->m_Dirty )
        pVertexBuffer->Rebuild( 0, NumVertsToDraw*g_VertexFormatSizes[VertexFormat] );
    if( pIndexBuffer && pIndexBuffer->m_Dirty )
        pIndexBuffer->Rebuild( 0, NumIndicesToDraw*pIndexBuffer->m_BytesPerIndex );
    MyAssert( ( pIndexBuffer == 0 || pIndexBuffer->m_Dirty == false ) && pVertexBuffer->m_Dirty == false );

    checkGlError( "Drawing Mesh Rebuild()" );

    if( pShaderOverride )
    {
        int indexbuffertype = GL_UNSIGNED_BYTE;
        if( pIndexBuffer != 0 )
        {
            int bytesperindex = pIndexBuffer->m_BytesPerIndex;
            if( bytesperindex == 2 )
                indexbuffertype = GL_UNSIGNED_SHORT;
            else if( bytesperindex == 4 )
                indexbuffertype = GL_UNSIGNED_INT;
        }

        //int numboneinfluences = 0;
        //if( pVertexBuffer && pVertexBuffer->m_pFormatDesc )
        //    numboneinfluences = pVertexBuffer->m_pFormatDesc->num_bone_influences;

        // if an override for the shader is sent in, it's already active and doesn't want anything other than position set.
        // always use 4 bone version.
        // TODO: this might fail with 1-3 bones,
        //       but should work with 0 bones since bone attribs are set to 100% weight on bone 0
        //       and bone 0 transform uniform is set to identity.
        Shader_Base* pShader = (Shader_Base*)pShaderOverride->GlobalPass( 0, 4 );
        pShader->SetupAttributes( pVertexBuffer, pIndexBuffer, false );
        pShader->ProgramPosition( matviewproj, matworld ); //&m_Transform );

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

        checkGlError( "MyMesh::Draw() - if( pShaderOverride ) - after SetupAttributes" );

        // Enable blending if necessary. TODO: sort draws and only set this once.
        //if( pMaterial->IsTransparent( pShader ) )
        //{
        //    glEnable( GL_BLEND );
        //    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        //}

        if( pIndexBuffer )
            MyDrawElements( PrimitiveType, NumIndicesToDraw, indexbuffertype, 0 );
        else
            MyDrawArrays( PrimitiveType, 0, NumVertsToDraw );
        //pShader->DeactivateShader( pVertexBuffer, false ); // disable attributes

        // always disable blending
        glDisable( GL_BLEND );

        checkGlError( "end of MyMesh::Draw() - if( pShaderOverride )" );
    }
    else
    {
        int numboneinfluences = 0;
        if( pVertexBuffer && pVertexBuffer->m_pFormatDesc )
            numboneinfluences = pVertexBuffer->m_pFormatDesc->num_bone_influences;

        if( pMaterial->GetShader() == 0 )
            return;

        Shader_Base* pShader = (Shader_Base*)pMaterial->GetShader()->GlobalPass( numlights, numboneinfluences );
        if( pShader )
        {
            if( pShader->ActivateAndProgramShader(
                pVertexBuffer, pIndexBuffer, GL_UNSIGNED_SHORT,
                matviewproj, matworld, pMaterial ) ) //&m_Transform, pMaterial ) )
            {
                checkGlError( "Drawing Mesh ActivateAndProgramShader()" );

                MyMatrix invworld = *matworld; //m_Transform;
                invworld.Inverse();
                //bool didinverse = invworld.Inverse();
                //if( didinverse == false )
                //    LOGError( LOGTag, "Matrix inverse failed\n" );

                pShader->ProgramCamera( campos, 0, &invworld );
                checkGlError( "Drawing Mesh ProgramCamera()" );

                pShader->ProgramLights( lights, numlights, &invworld );
                checkGlError( "Drawing Mesh ProgramLights()" );

                if( PrimitiveType == GL_POINTS )
                    pShader->ProgramPointSize( (float)PointSize );

                if( shadowlightVP && pShadowTex != 0 )
                {
                    MyMatrix textureoffsetmat( 0.5f,0,0,0,  0,0.5f,0,0,  0,0,0.5f,0,  0.5f,0.5f,0.5f,1 );
                    MyMatrix shadowWVPT = textureoffsetmat * *shadowlightVP * *matworld; //m_Transform;
                    pShader->ProgramShadowLight( &shadowWVPT, pShadowTex );
                }

                if( pLightmapTex != 0 )
                {
                    pShader->ProgramLightmap( pLightmapTex );
                    checkGlError( "Drawing Mesh ProgramLightmap()" );
                }

                if( pMesh->m_BoneFinalMatrices.Count() > 0 )
                {
                    pShader->ProgramBoneTransforms( &pMesh->m_BoneFinalMatrices[0], pMesh->m_BoneFinalMatrices.Count() );
                }

                int indexbuffertype = GL_UNSIGNED_BYTE;
                if( pIndexBuffer != 0 )
                {
                    int bytesperindex = pIndexBuffer->m_BytesPerIndex;
                    if( bytesperindex == 2 )
                        indexbuffertype = GL_UNSIGNED_SHORT;
                    else if( bytesperindex == 4 )
                        indexbuffertype = GL_UNSIGNED_INT;
                }

                pShader->ProgramFramebufferSize( (float)g_GLStats.m_CurrentFramebufferWidth, (float)g_GLStats.m_CurrentFramebufferHeight );

                // Enable blending if necessary. TODO: sort draws and only set this once.
                if( pMaterial->IsTransparent( pShader ) )
                {
                    glEnable( GL_BLEND );
                    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                }

                if( pIndexBuffer )
                    MyDrawElements( PrimitiveType, NumIndicesToDraw, indexbuffertype, 0 );
                else
                    MyDrawArrays( PrimitiveType, 0, NumVertsToDraw );

                checkGlError( "Drawing Mesh MyDrawElements()" );

                pShader->DeactivateShader( pVertexBuffer, true );
                checkGlError( "Drawing Mesh DeactivateShader()" );

                // always disable blending
                glDisable( GL_BLEND );
            }
        }
    }
}

MyMesh::MyMesh()
{
    m_pSourceFile = 0;
    m_ForceCheckForAnimationFile = false;
    m_MeshReady = false;

    m_AABounds.Set( Vector3(0), Vector3(0) );

    m_InitialScale = 1.0f; // TODO: make this changable through interface somehow... reload/recreate mesh when changed?

    //m_Transform.SetIdentity();

    g_pMeshManager->AddMesh( this );

    m_pAnimations.AllocateObjects( MAX_ANIMATIONS );

    m_pAnimationControlFile = 0;
}

MyMesh::~MyMesh()
{
    if( this->Prev )
        this->Remove();

    SAFE_RELEASE( m_pSourceFile );

    Clear();
}

void MyMesh::Clear()
{
    while( m_BoneNames.Count() )
    {
        delete[] m_BoneNames.RemoveIndex( 0 );
    }
    m_BoneNames.FreeAllInList();

    m_BoneOffsetMatrices.FreeAllInList();
    m_BoneFinalMatrices.FreeAllInList();
    m_pSkeletonNodeTree.FreeAllInList();

    while( m_pAnimationTimelines.Count() )
    {
        delete m_pAnimationTimelines.RemoveIndex( 0 );
    }
    m_pAnimationTimelines.FreeAllInList();

    while( m_pAnimations.Count() )
    {
        delete m_pAnimations.RemoveIndex( 0 );
    }
    //m_pAnimations.FreeAllInList(); // Allocated in contructor to max size.

    SAFE_RELEASE( m_pAnimationControlFile );
    m_ForceCheckForAnimationFile = true; // needed for when a mesh file gets reloaded because of changes, this will reload the anim file.

    while( m_SubmeshList.Count() )
    {
        delete m_SubmeshList.RemoveIndex( 0 );
    }
    m_SubmeshList.FreeAllInList();

    m_MeshReady = false;
}

#if MYFW_USING_WX
void MyMesh::RefreshWatchWindow()
{
    FillPropertiesWindow( true );
}

void MyMesh::FillPropertiesWindow(bool clear)
{
    g_pPanelWatch->SetRefreshCallback( this, MyMesh::StaticRefreshWatchWindow );

    g_pPanelWatch->Freeze();

    if( clear )
        g_pPanelWatch->ClearAllVariables();

    g_pPanelWatch->AddButton( "Save Animations", this, MyMesh::StaticOnSaveAnimationsPressed );

    for( unsigned int i=0; i<m_pAnimations.Count(); i++ )
    {
        // TODO: replace AddPointerWithDescription with a panel watch control for char*s
        m_ControlID_AnimationName[i] = g_pPanelWatch->AddPointerWithDescription( "Name", 0, m_pAnimations[i]->m_Name, this, 0, MyMesh::StaticOnValueChanged );
        g_pPanelWatch->AddFloat( "Start Time", &m_pAnimations[i]->m_StartTime, 0, 0 );
        g_pPanelWatch->AddFloat( "Duration", &m_pAnimations[i]->m_Duration, 0, 0 );
    }

    g_pPanelWatch->AddButton( "Add Animation", this, MyMesh::StaticOnAddAnimationPressed );

    g_pPanelWatch->Thaw();
}

void MyMesh::OnAddAnimationPressed()
{
    MyAnimation* pAnim = MyNew MyAnimation;

    pAnim->SetName( "New" );
    pAnim->m_TimelineIndex = 0;
    pAnim->m_StartTime = 0;
    pAnim->m_Duration = m_pAnimationTimelines[0]->m_Duration;

    m_pAnimations.Add( pAnim );

    g_pPanelWatch->m_NeedsRefresh = true;
    //FillPropertiesWindow( true ); // crashed since in button press callback and button would be recreated.
}

void MyMesh::OnSaveAnimationsPressed()
{
    SaveAnimationControlFile();
}

void MyMesh::OnValueChanged(int controlid, bool finishedchanging)
{
    if( controlid != -1 )
    {
        int animthatchanged = -1;
        for( int i=0; i<MAX_ANIMATIONS; i++ )
        {
            if( controlid == m_ControlID_AnimationName[i] )
                animthatchanged = i;                
        }

        if( animthatchanged != -1 )
        {
            wxString text = g_pPanelWatch->m_pVariables[controlid].m_Handle_TextCtrl->GetValue();
            m_pAnimations[animthatchanged]->SetName( text );
        }
    }
}
#endif

void MyMesh::CreateSubmeshes(int numsubmeshes)
{
    MyAssert( m_SubmeshList.Length() == 0 );
    m_SubmeshList.AllocateObjects( numsubmeshes );
    for( int i=0; i<numsubmeshes; i++ )
        m_SubmeshList.Add( MyNew MySubmesh() );
}

void MyMesh::CreateBuffers(int vertexformat, unsigned short numverts, unsigned int numindices, bool dynamic)
{
    MyAssert( m_SubmeshList.Length() == 0 );

    CreateSubmeshes( 1 );
    MyAssert( m_SubmeshList[0]->m_pVertexBuffer == 0 );
    MyAssert( m_SubmeshList[0]->m_pIndexBuffer == 0 );

    GLenum usage;
    int numbuffers;
    if( dynamic )
    {
        usage = GL_DYNAMIC_DRAW;
        //numbuffers = 1;
        numbuffers = 2;
    }
    else
    {
        usage = GL_STATIC_DRAW;
        numbuffers = 1;
    }

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        //m_NumVerts = numverts;
        m_SubmeshList[0]->m_NumVertsToDraw = numverts;
        m_SubmeshList[0]->m_VertexFormat = vertexformat;

        if( m_SubmeshList[0]->m_VertexFormat == VertexFormat_XYZUV_RGBA )
        {
            Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[numverts];
            m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*numverts, GL_ARRAY_BUFFER, usage, false, numbuffers, VertexFormat_XYZUV_RGBA, "MyMesh", "Verts" );
        }
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
    {
        //m_NumIndices = numindices;
        m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

        unsigned short* pIndices = MyNew unsigned short[numindices];
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*numindices, GL_ELEMENT_ARRAY_BUFFER, usage, false, numbuffers, 2, "MyMesh", "Verts" );
    }
}

void MyMesh::CreateFromOBJFile(MyFileObject* pFile)
{
    pFile->AddRef();
    SAFE_RELEASE( m_pSourceFile );
    m_pSourceFile = pFile;

    m_MeshReady = false;

    if( pFile->m_FileLoadStatus == FileLoadStatus_Success )
    {
        LoadBasicOBJ( pFile->m_pBuffer, &m_SubmeshList, false, 1.0f, &m_AABounds );

        // TODO: fix if obj loader ever supports submeshes.
        if( m_SubmeshList[0]->m_pVertexBuffer && m_SubmeshList[0]->m_pIndexBuffer )
        {
            //m_VertexFormat = m_pVertexBuffer->m_VertexFormat;
            m_SubmeshList[0]->m_NumIndicesToDraw = m_SubmeshList[0]->m_pIndexBuffer->m_DataSize / m_SubmeshList[0]->m_pIndexBuffer->m_BytesPerIndex;

            m_MeshReady = true;
        }
    }
}

void MyMesh::CreateFromMyMeshFile(MyFileObject* pFile)
{
    //LOGInfo( LOGTag, "%d: MyMesh::CreateFromMyMeshFile ( %s ) m_pAnimationControlFile = %d\n", this, pFile->m_FilenameWithoutExtension, m_pAnimationControlFile );

    MyAssert( pFile );

    // if the requested file changed, then ditch the current one and load the new one.
    if( pFile != m_pSourceFile || m_ForceCheckForAnimationFile )
    {
        m_ForceCheckForAnimationFile = false;

        // free the old .mymesh file and store a pointer to the new one.
        pFile->AddRef();
        SAFE_RELEASE( m_pSourceFile );
        m_pSourceFile = pFile;

        // free the old .myaniminfo file and store a pointer to the new one.
        //if( m_pAnimationControlFile )
        //    LOGInfo( LOGTag, "Releasing old animation file ( %s ) file = %d\n", pFile->m_FilenameWithoutExtension, m_pAnimationControlFile );
        SAFE_RELEASE( m_pAnimationControlFile );
        char animfilename[MAX_PATH];
        pFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".myaniminfo", animfilename, MAX_PATH );
#if MYFW_USING_WX
        // only try to open the file if it exists, only in editor builds since file i/o isn't necessarily synchronous otherwise.
        if( g_pFileManager->DoesFileExist( animfilename ) )
        {
            MyFileObject* newfile = g_pFileManager->RequestFile( animfilename ); // adds a ref to the existing file or new one.
            m_pAnimationControlFile = newfile;
            //LOGInfo( LOGTag, "g_pFileManager->DoesFileExist( %s ) returned true file = %d\n", animfilename, m_pAnimationControlFile );
        }
        else
        {
            //LOGInfo( LOGTag, "g_pFileManager->DoesFileExist( %s ) returned false\n", animfilename );
        }
#else
        MyFileObject* newfile = g_pFileManager->RequestFile( animfilename ); // adds a ref to the existing file or new one.
        m_pAnimationControlFile = newfile;
#endif
    }

    m_MeshReady = false;

    // is the mesh ready and the anim file is loaded or failed to load.
    if( pFile->m_FileLoadStatus == FileLoadStatus_Success &&
        (m_pAnimationControlFile == 0 || m_pAnimationControlFile->m_FileLoadStatus >= FileLoadStatus_Success)
      )
    {
        //LOGInfo( LOGTag, "Animation File = %d\n", m_pAnimationControlFile );

        if( m_pAnimationControlFile && m_pAnimationControlFile->m_FileLoadStatus == FileLoadStatus_Error_FileNotFound )
        {
            //LOGInfo( LOGTag, "Animation File - error loading file\n" );
            g_pFileManager->FreeFile( m_pAnimationControlFile );
            m_pAnimationControlFile = 0;
        }

        LoadMyMesh( pFile->m_pBuffer, &m_SubmeshList, m_InitialScale );
        
        if( m_pAnimationControlFile )
        {
            //LOGInfo( LOGTag, "LoadAnimationControlFile = %s\n", m_pAnimationControlFile->m_pBuffer );
            LoadAnimationControlFile( m_pAnimationControlFile->m_pBuffer );
        }
        else
        {
            LoadAnimationControlFile( 0 );
        }
    }
}

void MyMesh::ParseFile()
{
    if( m_MeshReady == false )
    {
        if( m_pSourceFile != 0 )
        {
            if( strcmp( m_pSourceFile->m_ExtensionWithDot, ".obj" ) == 0 )
            {
                CreateFromOBJFile( m_pSourceFile );
            }
            if( strcmp( m_pSourceFile->m_ExtensionWithDot, ".mymesh" ) == 0 )
            {
                CreateFromMyMeshFile( m_pSourceFile );
            }

            if( m_SubmeshList.Count() > 0 )
            {
                if( m_SubmeshList[0]->m_pMaterial && m_SubmeshList[0]->m_pMaterial->GetShader() == 0 )
                {
                    // guess at an appropriate shader for this mesh/material.
                    GuessAndAssignAppropriateShader();
                }
            }
        }
    }
}

void MyMesh::GuessAndAssignAppropriateShader()
{
    for( unsigned int i=0; i<m_SubmeshList.Count(); i++ )
    {
        if( m_SubmeshList[i]->m_pMaterial->GetShader() == 0 )
        {
            // TODO: actually write code here...
            m_SubmeshList[i]->m_pMaterial->SetShader( g_pShaderGroupManager->FindShaderGroupByName( "Shader_TintColor" ) );
        }
    }
}

void MyMesh::CreateBox(float boxw, float boxh, float boxd, float startu, float endu, float startv, float endv, unsigned char justificationflags)
{
    CreateSubmeshes( 1 );
    MyAssert( m_SubmeshList.Count() == 1 );

    MyAssert( m_SubmeshList[0]->m_pVertexBuffer == 0 );
    MyAssert( m_SubmeshList[0]->m_pIndexBuffer == 0 );

    unsigned short numverts = 24;
    unsigned int numindices = 36;
    m_SubmeshList[0]->m_NumVertsToDraw = numverts;
    m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZUVNorm;
        Vertex_XYZUVNorm* pVerts = MyNew Vertex_XYZUVNorm[24];
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUVNorm)*24, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUVNorm, "MyMesh_Box", "Verts" );
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
    {
        unsigned short* pIndices = MyNew unsigned short[36];
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*36, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_Box", "Indices" );
    }

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    unsigned short* pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;

    if( pVerts && pIndices )
    {
        float uleft = startu;
        float uright = endu;
        float vtop = startv;
        float vbottom = endv;

        float xleft = 0;
        float xright = boxw;
        float ytop = 0;
        float ybottom = -boxh;
        float zfront = boxd/2;
        float zback = -boxd/2;

        if( justificationflags & Justify_Bottom )
        {
            ytop += boxh;
            ybottom += boxh;
        }
        else if( justificationflags & Justify_CenterY )
        {
            ytop += boxh / 2.0f;
            ybottom += boxh / 2.0f;
        }

        if( justificationflags & Justify_Right )
        {
            xleft -= boxw;
            xright -= boxw;
        }
        else if( justificationflags & Justify_CenterX )
        {
            xleft -= boxw / 2.0f;
            xright -= boxw / 2.0f;
        }

        GLushort side;

        // front
        side = 0;
        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront;  pVerts[0].uv.x = uleft;  pVerts[0].uv.y = vtop;    // upper left
        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront;  pVerts[1].uv.x = uright; pVerts[1].uv.y = vtop;    // upper right
        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront;  pVerts[2].uv.x = uleft;  pVerts[2].uv.y = vbottom; // lower left
        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront;  pVerts[3].uv.x = uright; pVerts[3].uv.y = vbottom; // lower right
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 0, 1 );

        // back
        side = 1;
        pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 0, -1 );

        // right
        side = 2;
        pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 1, 0, 0 );

        // left
        side = 3;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( -1, 0, 0 );

        // bottom
        side = 4;
        pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, -1, 0 );

        // top
        side = 5;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[0];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[1];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].normal.Set( 0, 1, 0 );

        for( side=0; side<6; side++ )
        {
            pIndices[6*side + 0] = 4*side + 0;
            pIndices[6*side + 1] = 4*side + 2;
            pIndices[6*side + 2] = 4*side + 1;
            pIndices[6*side + 3] = 4*side + 1;
            pIndices[6*side + 4] = 4*side + 2;
            pIndices[6*side + 5] = 4*side + 3;
        }

        Vector3 center( (xleft + xright) / 2, (ytop + ybottom) / 2, (zfront + zback) / 2 );
        m_AABounds.Set( center, Vector3(boxw/2, boxh/2, boxh/2) );
    }

    m_MeshReady = true;
}

void MyMesh::CreateBox_XYZUV_RGBA(float boxw, float boxh, float boxd, float startutop, float endutop, float startvtop, float endvtop, float startuside, float enduside, float startvside, float endvside, unsigned char justificationflags)
{
    CreateSubmeshes( 1 );
    MyAssert( m_SubmeshList.Count() == 1 );

    MyAssert( m_SubmeshList[0]->m_pVertexBuffer == 0 );
    MyAssert( m_SubmeshList[0]->m_pIndexBuffer == 0 );

    unsigned short numverts = 24;
    unsigned int numindices = 36;
    m_SubmeshList[0]->m_NumVertsToDraw = numverts;
    m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZUV_RGBA;
        Vertex_XYZUV_RGBA* pVerts = MyNew Vertex_XYZUV_RGBA[24];
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUV_RGBA)*24, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 2, VertexFormat_XYZUV_RGBA, "MyMesh_BoxXYZUVRGBA", "Verts" );
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
    {
        unsigned short* pIndices = MyNew unsigned short[36];
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*36, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_BoxXYZUVRGBA", "Indices" );
    }

    m_SubmeshList[0]->m_pVertexBuffer->m_Dirty = true;
    m_SubmeshList[0]->m_pIndexBuffer->m_Dirty = true;

    Vertex_XYZUV_RGBA_Alt* pVerts = (Vertex_XYZUV_RGBA_Alt*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    unsigned short* pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;

    if( pVerts && pIndices )
    {
        float ulefttop = startutop;
        float urighttop = endutop;
        float vtoptop = startvtop;
        float vbottomtop = endvtop;

        float uleftside = startuside;
        float urightside = enduside;
        float vtopside = startvside;
        float vbottomside = endvside;

        float xleft = 0;
        float xright = boxw;
        float ytop = 0;
        float ybottom = -boxh;
        float zfront = boxd/2;
        float zback = -boxd/2;

        if( justificationflags & Justify_Bottom )
        {
            ytop += boxh;
            ybottom += boxh;
        }
        else if( justificationflags & Justify_CenterY )
        {
            ytop += boxh / 2.0f;
            ybottom += boxh / 2.0f;
        }

        if( justificationflags & Justify_Right )
        {
            xleft -= boxw;
            xright -= boxw;
        }
        else if( justificationflags & Justify_CenterX )
        {
            xleft -= boxw / 2.0f;
            xright -= boxw / 2.0f;
        }

        GLushort side;

        // front
        side = 0;
        pVerts[0].pos.x = xleft;  pVerts[0].pos.y = ytop;    pVerts[0].pos.z = zfront;  pVerts[0].uv.x = uleftside;  pVerts[0].uv.y = vtopside;    // upper left
        pVerts[1].pos.x = xright; pVerts[1].pos.y = ytop;    pVerts[1].pos.z = zfront;  pVerts[1].uv.x = urightside; pVerts[1].uv.y = vtopside;    // upper right
        pVerts[2].pos.x = xleft;  pVerts[2].pos.y = ybottom; pVerts[2].pos.z = zfront;  pVerts[2].uv.x = uleftside;  pVerts[2].uv.y = vbottomside; // lower left
        pVerts[3].pos.x = xright; pVerts[3].pos.y = ybottom; pVerts[3].pos.z = zfront;  pVerts[3].uv.x = urightside; pVerts[3].uv.y = vbottomside; // lower right
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0,1,1 ); //normal.Set( 0, 0, 1 );

        // back
        side = 1;
        pVerts[4*side + 0] = pVerts[1]; pVerts[4*side + 0].pos.z = zback;  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0]; pVerts[4*side + 1].pos.z = zback;  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3]; pVerts[4*side + 2].pos.z = zback;  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2]; pVerts[4*side + 3].pos.z = zback;  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0,0.2f,1 ); //normal.Set( 0, 0, -1 );

        // right
        side = 2;
        pVerts[4*side + 0] = pVerts[1];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[3];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //1,0,0,1 ); //normal.Set( 1, 0, 0 );

        // left
        side = 3;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[0].uv;
        pVerts[4*side + 1] = pVerts[0];  pVerts[4*side + 1].uv = pVerts[1].uv;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv = pVerts[2].uv;
        pVerts[4*side + 3] = pVerts[2];  pVerts[4*side + 3].uv = pVerts[3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0.2f,0,0,1 ); //normal.Set( -1, 0, 0 );

        // bottom
        side = 4;
        pVerts[4*side + 0] = pVerts[2];  pVerts[4*side + 0].uv.x = ulefttop;  pVerts[4*side + 0].uv.y = vtoptop;
        pVerts[4*side + 1] = pVerts[3];  pVerts[4*side + 1].uv.x = urighttop; pVerts[4*side + 1].uv.y = vtoptop;
        pVerts[4*side + 2] = pVerts[7];  pVerts[4*side + 2].uv.x = ulefttop;  pVerts[4*side + 2].uv.y = vbottomtop;
        pVerts[4*side + 3] = pVerts[6];  pVerts[4*side + 3].uv.x = urighttop; pVerts[4*side + 3].uv.y = vbottomtop;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,0.2f,0,1 ); //normal.Set( 0, -1, 0 );

        // top
        side = 5;
        pVerts[4*side + 0] = pVerts[5];  pVerts[4*side + 0].uv = pVerts[4*4 + 0].uv; // copy UV's from "bottom"
        pVerts[4*side + 1] = pVerts[4];  pVerts[4*side + 1].uv = pVerts[4*4 + 1].uv;
        pVerts[4*side + 2] = pVerts[0];  pVerts[4*side + 2].uv = pVerts[4*4 + 2].uv;
        pVerts[4*side + 3] = pVerts[1];  pVerts[4*side + 3].uv = pVerts[4*4 + 3].uv;
        for( int i=0; i<4; i++ )
            pVerts[4*side + i].col.Set( 1,1,1,1 ); //0,1,0,1 ); //normal.Set( 0, 1, 0 );

        for( side=0; side<6; side++ )
        {
            pIndices[6*side + 0] = 4*side + 0;
            pIndices[6*side + 1] = 4*side + 2;
            pIndices[6*side + 2] = 4*side + 1;
            pIndices[6*side + 3] = 4*side + 1;
            pIndices[6*side + 4] = 4*side + 2;
            pIndices[6*side + 5] = 4*side + 3;
        }

        Vector3 center( (xleft + xright) / 2, (ytop + ybottom) / 2, (zfront + zback) / 2 );
        m_AABounds.Set( center, Vector3(boxw/2, boxh/2, boxh/2) );
    }

    m_MeshReady = true;
}

void MyMesh::SetBoxVertexColors(ColorByte TL, ColorByte TR, ColorByte BL, ColorByte BR)
{
    MyAssert( m_SubmeshList.Count() == 1 );

    m_SubmeshList[0]->m_pVertexBuffer->m_Dirty = true;

    Vertex_XYZUV_RGBA_Alt* pVerts = (Vertex_XYZUV_RGBA_Alt*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;

    int side;

    // front
    side = 0;
    pVerts[0].col = BL;
    pVerts[1].col = BR;
    pVerts[2].col = BL;
    pVerts[3].col = BR;

    // back
    side = 1;
    pVerts[4*side + 0].col = TR;
    pVerts[4*side + 1].col = TL;
    pVerts[4*side + 2].col = TR;
    pVerts[4*side + 3].col = TL;

    // right
    side = 2;
    pVerts[4*side + 0].col = BR;
    pVerts[4*side + 1].col = TR;
    pVerts[4*side + 2].col = BR;
    pVerts[4*side + 3].col = TR;

    // left
    side = 3;
    pVerts[4*side + 0].col = TL;
    pVerts[4*side + 1].col = BL;
    pVerts[4*side + 2].col = TL;
    pVerts[4*side + 3].col = BL;

    // bottom
    side = 4;
    pVerts[4*side + 0].col = BL;
    pVerts[4*side + 1].col = BR;
    pVerts[4*side + 2].col = TL;
    pVerts[4*side + 3].col = TR;

    // top
    side = 5;
    pVerts[4*side + 0].col = TL;
    pVerts[4*side + 1].col = TR;
    pVerts[4*side + 2].col = BL;
    pVerts[4*side + 3].col = BR;

    m_MeshReady = true;
}

void MyMesh::CreateCylinder(float radius, unsigned short numsegments, float edgeradius, float height, float topstartu, float topendu, float topstartv, float topendv, float sidestartu, float sideendu, float sidestartv, float sideendv)
{
    CreateSubmeshes( 1 );
    MyAssert( m_SubmeshList.Count() == 1 );

    MyAssert( m_SubmeshList[0]->m_pVertexBuffer == 0 );
    MyAssert( m_SubmeshList[0]->m_pIndexBuffer == 0 );

    float uperc, vperc;

    unsigned short topverts = numsegments + 1;
    unsigned short edgeverts = numsegments * 2;
    unsigned short sideverts = (numsegments + 1) * 2; // +1 since I'm doubling a column of verts to allow mapping from atlas.
    unsigned short numverts = (topverts + edgeverts)*2 + sideverts;
    int numtris = numsegments * 8;

    unsigned int numindices = numtris*3;
    //m_NumVerts = numverts;
    //m_NumIndices = numtris*3;
    m_SubmeshList[0]->m_NumVertsToDraw = numverts;
    m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZUVNorm;
        Vertex_XYZUVNorm* pVerts = MyNew Vertex_XYZUVNorm[numverts];
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZUVNorm)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUVNorm, "MyMesh_Cylinder", "Verts" );
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
    {
        unsigned short* pIndices = MyNew unsigned short[numtris*3];
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned short)*numtris*3, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_Cylinder", "Indices" );
    }

    Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    unsigned short* pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;

    GLushort vertnum = 0;

    float topsizeu = topendu - topstartu;
    float topsizev = topendv - topstartv;

    // create top center vert
    pVerts[vertnum].pos.Set( 0, height, 0 );
    pVerts[vertnum].uv.Set( topstartu + (topsizeu / 2.0f), topstartv + (topsizev / 2.0f) );
    pVerts[vertnum].normal.Set( 0, 1, 0 );
    vertnum++;

    float uvclamplow = 0.01f;
    float uvclamphigh = 1 - uvclamplow;// 0.98f;

    //               TOP                      BOTTOM - 17+
    //z=1        /----6----\                /----6----\
    //          /           \              /           \
    //         8   /--5--\   4            8   /--5--\   4
    //        /   7       3   \          /   7       3   \
    //       /   /         \   \        /   /         \   \
    //z=0   10  9     0     1   2      10  9     0     1   2
    //       \   \         /   /        \   \         /   /
    //        \   11      15  /          \   11      15  /
    //         12  \--13-/   16           12  \--13-/   16
    //          \           /              \           /
    //z=-1       \----14---/                \----14---/

    int flipz = 1;
#if MYFW_RIGHTHANDED
    flipz = -1;
#endif

    // create top inner and outer(edge) circle verts
    for( int i=0; i<numsegments; i++ )
    {
        float radianspersegment = PI*2/numsegments;
        float radians = radianspersegment * i;

        // inner vert
        pVerts[vertnum].pos.x = cos( radians ) * (radius - edgeradius);
        pVerts[vertnum].pos.z = sin( radians ) * (radius - edgeradius) * flipz;
        pVerts[vertnum].pos.y = height;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topstartu + topsizeu * uperc, topstartv + topsizev * vperc );
        pVerts[vertnum].normal.Set( 0, 1, 0 );
        vertnum++;

        // outer vert
        pVerts[vertnum].pos.x = cos( radians ) * (radius);
        pVerts[vertnum].pos.z = sin( radians ) * (radius) * flipz;
        pVerts[vertnum].pos.y = height;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topstartu + topsizeu * uperc, topstartv + topsizev * vperc );
        //pVerts[vertnum].normal.Set( 0, 1, 0 );
        pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 1, pVerts[vertnum].pos.z );
        pVerts[vertnum].normal.Normalize();
        vertnum++;
    }

    // create bottom center vert
    pVerts[vertnum].pos.Set( 0, 0, 0 );
    pVerts[vertnum].uv.Set( topstartu + (topsizeu / 2.0f), topstartv + (topsizev / 2.0f) );
    pVerts[vertnum].normal.Set( 0, -1, 0 );
    vertnum++;

    // create bottom inner and outer(edge) circle verts
    for( int i=0; i<numsegments; i++ )
    {
        float radianspersegment = PI*2/numsegments;
        float radians = radianspersegment * i;

        // inner vert
        pVerts[vertnum].pos.x = cos( radians ) * (radius - edgeradius);
        pVerts[vertnum].pos.z = sin( radians ) * (radius - edgeradius) * flipz;
        pVerts[vertnum].pos.y = 0;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topendu - topsizeu * uperc, topstartv + topsizev * vperc );
        pVerts[vertnum].normal.Set( 0, -1, 0 );
        vertnum++;

        // outer vert
        pVerts[vertnum].pos.x = cos( radians ) * (radius);
        pVerts[vertnum].pos.z = sin( radians ) * (radius) * flipz;
        pVerts[vertnum].pos.y = 0;

        uperc = (pVerts[vertnum].pos.x + radius) / (radius*2);
        MyClamp( uperc, uvclamplow, uvclamphigh );
        vperc = 1 - (pVerts[vertnum].pos.z + radius) / (radius*2);
        MyClamp( vperc, uvclamplow, uvclamphigh );
        pVerts[vertnum].uv.Set( topendu - topsizeu * uperc, topstartv + topsizev * vperc );
        //pVerts[vertnum].normal.Set( 0, -1, 0 );
        pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, -1, pVerts[vertnum].pos.z );
        pVerts[vertnum].normal.Normalize();
        vertnum++;
    }

    // create side verts
    GLushort firstsidevert = vertnum;
    {
        int topbase = 0;
        int bottombase = numsegments * 2 + 1;
        for( int i=0; i<numsegments+1; i++ )
        {
            int oldvertindex = i;
            if( i == numsegments )
                oldvertindex = 0;

            // top vert
            pVerts[vertnum].pos.x = pVerts[topbase + oldvertindex*2 + 2].pos.x;
            pVerts[vertnum].pos.z = pVerts[topbase + oldvertindex*2 + 2].pos.z;
            pVerts[vertnum].pos.y = height;
            pVerts[vertnum].uv.x = sidestartu + (sideendu - sidestartu)*i / numsegments;
            pVerts[vertnum].uv.y = sidestartv;// + (sideendv - sidestartv) / 2;
            pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 0, pVerts[vertnum].pos.z );
            pVerts[vertnum].normal.Normalize();
            vertnum++;

            // bottom vert
            pVerts[vertnum].pos.x = pVerts[bottombase + oldvertindex*2 + 2].pos.x;
            pVerts[vertnum].pos.z = pVerts[bottombase + oldvertindex*2 + 2].pos.z;
            pVerts[vertnum].pos.y = 0;
            pVerts[vertnum].uv.x = sidestartu + (sideendu - sidestartu)*i / numsegments;
            pVerts[vertnum].uv.y = sideendv;// + (sideendv - sidestartv) / 2;
            pVerts[vertnum].normal.Set( pVerts[vertnum].pos.x, 0, pVerts[vertnum].pos.z );
            pVerts[vertnum].normal.Normalize();
            vertnum++;
        }
    }

    int indexnum = 0;

    // setup top center triangles
    if( true )
    {
        GLushort baseindex = 0;
        for( GLushort i=0; i<numsegments; i++ )
        {
            // 0,3,1,   0,5,3,   0,7,5...
            pIndices[indexnum + 0] = 0;
            pIndices[indexnum + 2] = baseindex + i*2 + 1;
            if( i != numsegments-1 )
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 1] = baseindex + 1;

            indexnum += 3;
        }
    }

    // setup top outer edge triangles
    if( true )
    {
        for( GLushort i=0; i<numsegments; i++ )
        {
            // 1,3,2,   2,3,4,   3,5,4...
            pIndices[indexnum + 0] = i*2 + 1;
            pIndices[indexnum + 2] = i*2 + 2;
            if( i != numsegments-1 )
                pIndices[indexnum + 1] = (i+1)*2 + 1;
            else
                pIndices[indexnum + 1] = 0*2 + 1;
            indexnum += 3;

            pIndices[indexnum + 0] = (i)*2 + 2;
            if( i != numsegments-1 )
            {
                pIndices[indexnum + 2] = (i+1)*2 + 2;
                pIndices[indexnum + 1] = (i+1)*2 + 1;
            }
            else
            {
                pIndices[indexnum + 2] = (0)*2 + 2;
                pIndices[indexnum + 1] = (0)*2 + 1;
            }
            indexnum += 3;
        }
    }

    // setup side triangles
    if( true )
    {
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = firstsidevert + i*2;
            pIndices[indexnum + 2] = firstsidevert + i*2 + 1;
            pIndices[indexnum + 1] = firstsidevert + i*2 + 2; //(i*2 + 2)%(numsegments*2);
            indexnum += 3;

            pIndices[indexnum + 0] = firstsidevert + i*2 + 1;
            pIndices[indexnum + 2] = firstsidevert + i*2 + 3; //(i*2 + 3)%(numsegments*2);
            pIndices[indexnum + 1] = firstsidevert + i*2 + 2; //(i*2 + 2)%(numsegments*2);
            indexnum += 3;
        }
    }

    // setup bottom center triangles
    if( true )
    {
        GLushort baseindex = (GLushort)(numsegments*2 + 1);
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = baseindex + 0;
            pIndices[indexnum + 1] = baseindex + i*2 + 1;
            if( i != numsegments-1 )
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 2] = baseindex + 1;

            indexnum += 3;
        }
    }

    // setup bottom outer edge triangles
    if( true )
    {
        GLushort baseindex = (GLushort)(numsegments*2 + 1);
        for( GLushort i=0; i<numsegments; i++ )
        {
            pIndices[indexnum + 0] = baseindex + i*2 + 1;
            pIndices[indexnum + 1] = baseindex + i*2 + 2;
            if( i != numsegments-1 )
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            else
                pIndices[indexnum + 2] = baseindex + 0*2 + 1;
            indexnum += 3;

            pIndices[indexnum + 0] = baseindex + (i)*2 + 2;
            if( i != numsegments-1 )
            {
                pIndices[indexnum + 1] = baseindex + (i+1)*2 + 2;
                pIndices[indexnum + 2] = baseindex + (i+1)*2 + 1;
            }
            else
            {
                pIndices[indexnum + 1] = baseindex + (0)*2 + 2;
                pIndices[indexnum + 2] = baseindex + (0)*2 + 1;
            }
            indexnum += 3;
        }
    }

    Vector3 center( 0, height/2, 0 );
    m_AABounds.Set( center, Vector3(radius, height/2, radius) );

    m_MeshReady = true;
}

void MyMesh::CreatePlane(Vector3 topleftpos, Vector2 size, Vector2Int vertcount, Vector2 uvstart, Vector2 uvrange, bool createtriangles)
{
    checkGlError( "MyMesh::CreatePlane" );

    int numverts = vertcount.x * vertcount.y;
    if( numverts < 0 || numverts > 65535 )
        return;

    LOGInfo( LOGTag, "MyMesh::CreatePlane\n" );

    unsigned int numtris = (vertcount.x - 1) * (vertcount.y - 1) * 2;
    unsigned int numindices = numtris * 3;
    if( createtriangles == false )
        numindices = numverts;

    if( m_SubmeshList.Length() == 0 )
    {
        CreateSubmeshes( 1 );
    }
    MyAssert( m_SubmeshList.Count() == 1 );

    m_SubmeshList[0]->m_NumVertsToDraw = (unsigned short)numverts;
    m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer();
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )//&& createtriangles )
    {
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer();
    }

    // delete the old buffers, if we want a plane with more.
    if( sizeof(Vertex_XYZUV)*numverts > m_SubmeshList[0]->m_pVertexBuffer->m_DataSize )
    {
        m_SubmeshList[0]->m_pVertexBuffer->FreeBufferedData();
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZUV;
        Vertex_XYZUV* pVerts = MyNew Vertex_XYZUV[numverts];
        m_SubmeshList[0]->m_pVertexBuffer->InitializeBuffer( pVerts, sizeof(Vertex_XYZUV)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUV, 0, "MyMesh_Plane", "Verts" );
    }

    if( sizeof(unsigned short)*numindices > m_SubmeshList[0]->m_pIndexBuffer->m_DataSize )
    {
        m_SubmeshList[0]->m_pIndexBuffer->FreeBufferedData();
        unsigned short* pIndices = MyNew unsigned short[numindices];
        m_SubmeshList[0]->m_pIndexBuffer->InitializeBuffer( pIndices, sizeof(unsigned short)*numindices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 2, "MyMesh_Plane", "Indices" );
    }

    m_SubmeshList[0]->m_pIndexBuffer->m_BytesPerIndex = 2;

    Vertex_XYZUV* pVerts = (Vertex_XYZUV*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    m_SubmeshList[0]->m_pVertexBuffer->m_Dirty = true;

    unsigned short* pIndices = 0;
    //if( createtriangles )
    {
        pIndices = (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;
        m_SubmeshList[0]->m_pIndexBuffer->m_Dirty = true;
    }

    for( int y = 0; y < vertcount.y; y++ )
    {
        for( int x = 0; x < vertcount.x; x++ )
        {
            unsigned short index = (unsigned short)(y * vertcount.x + x);

            pVerts[index].x = topleftpos.x + size.x / (vertcount.x - 1) * x;
            pVerts[index].y = topleftpos.y;
            pVerts[index].z = topleftpos.z + size.y / (vertcount.y - 1) * y;

            pVerts[index].u = uvstart.x + x * uvrange.x / (vertcount.x - 1);
            pVerts[index].v = uvstart.y + y * uvrange.y / (vertcount.y - 1);

            if( createtriangles == false )
                pIndices[index] = index;
        }
    }

    if( createtriangles )
    {
        for( int y = 0; y < vertcount.y - 1; y++ )
        {
            for( int x = 0; x < vertcount.x - 1; x++ )
            {
                int elementindex = (y * (vertcount.x-1) + x) * 6;
                unsigned short vertexindex = (unsigned short)(y * vertcount.x + x);

                pIndices[ elementindex + 0 ] = vertexindex + 0;
                pIndices[ elementindex + 1 ] = vertexindex + (unsigned short)vertcount.x;
                pIndices[ elementindex + 2 ] = vertexindex + 1;

                pIndices[ elementindex + 3 ] = vertexindex + 1;
                pIndices[ elementindex + 4 ] = vertexindex + (unsigned short)vertcount.x;
                pIndices[ elementindex + 5 ] = vertexindex + (unsigned short)vertcount.x + 1;
            }
        }
    }

    Vector3 center( topleftpos.x + size.x/2, topleftpos.y, topleftpos.z + size.y/ 2 );
    m_AABounds.Set( center, Vector3(size.x/2, 0, size.y/2) );

    m_MeshReady = true;
};

void MyMesh::CreateIcosphere(float radius, unsigned int recursionlevel)
{
    // from http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

    int numverts = 12;
    unsigned int numtris = 20;
    unsigned int numindices = numtris * 3;
    int bytesperindex = sizeof(unsigned char);

    if( m_SubmeshList.Length() == 0 )
    {
        CreateSubmeshes( 1 );
    }
    MyAssert( m_SubmeshList.Count() == 1 );

    m_SubmeshList[0]->m_NumVertsToDraw = (unsigned short)numverts;
    m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer();
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
    {
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer();
    }

    // delete the old buffers, if we want an icosphere with more.
    if( sizeof(Vertex_XYZUV)*numverts > m_SubmeshList[0]->m_pVertexBuffer->m_DataSize )
    {
        m_SubmeshList[0]->m_pVertexBuffer->FreeBufferedData();
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZUV;
        Vertex_XYZUV* pVerts = MyNew Vertex_XYZUV[numverts];
        m_SubmeshList[0]->m_pVertexBuffer->InitializeBuffer( pVerts, sizeof(Vertex_XYZUV)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUV, 0, "MyMesh_Icosphere", "Verts" );
    }

    if( bytesperindex*numindices > m_SubmeshList[0]->m_pIndexBuffer->m_DataSize )
    {
        m_SubmeshList[0]->m_pIndexBuffer->FreeBufferedData();
        unsigned char* pIndices = MyNew unsigned char[numindices];
        m_SubmeshList[0]->m_pIndexBuffer->InitializeBuffer( pIndices, bytesperindex*numindices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, bytesperindex, "MyMesh_Icosphere", "Indices" );
    }

    m_SubmeshList[0]->m_pIndexBuffer->m_BytesPerIndex = bytesperindex;

    Vertex_XYZUV_Alt* pVerts = (Vertex_XYZUV_Alt*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    m_SubmeshList[0]->m_pVertexBuffer->m_Dirty = true;

    unsigned char* pIndices = 0;
    pIndices = (unsigned char*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;
    m_SubmeshList[0]->m_pIndexBuffer->m_Dirty = true;

    // create 12 vertices of a icosahedron
    float t = (1.0f + sqrt(5.0f)) / 2.0f;

    pVerts[ 0].pos.Set( -1,  t,  0 );
    pVerts[ 1].pos.Set(  1,  t,  0 );
    pVerts[ 2].pos.Set( -1, -t,  0 );
    pVerts[ 3].pos.Set(  1, -t,  0 );

    pVerts[ 4].pos.Set(  0, -1,  t );
    pVerts[ 5].pos.Set(  0,  1,  t );
    pVerts[ 6].pos.Set(  0, -1, -t );
    pVerts[ 7].pos.Set(  0,  1, -t );

    pVerts[ 8].pos.Set(  t,  0, -1 );
    pVerts[ 9].pos.Set(  t,  0,  1 );
    pVerts[10].pos.Set( -t,  0, -1 );
    pVerts[11].pos.Set( -t,  0,  1 );

    for( int i=0; i<12; i++ )
        pVerts[i].pos *= radius;

    // create 20 triangles of the icosahedron
    unsigned char indexlist[] =
    {
       0, 11,  5,    0,  5,  1,    0,  1,  7,    0,  7, 10,    0, 10, 11,  // 5 faces around point 0
       1,  5,  9,    5, 11,  4,   11, 10,  2,   10,  7,  6,    7,  1,  8,  // 5 adjacent faces 
       3,  9,  4,    3,  4,  2,    3,  2,  6,    3,  6,  8,    3,  8,  9,  // 5 faces around point 3
       4,  9,  5,    2,  4, 11,    6,  2, 10,    8,  6,  7,    9,  8,  1,  // 5 adjacent faces
    };

    for( int i=0; i<60; i++ )
        pIndices[i] = indexlist[i];

    m_AABounds.Set( Vector3(0), Vector3(radius) );

    m_MeshReady = true;

    //public class IcoSphereCreator
    //{
    //    private struct TriangleIndices
    //    {
    //        public int v1;
    //        public int v2;
    //        public int v3;

    //        public TriangleIndices(int v1, int v2, int v3)
    //        {
    //            this.v1 = v1;
    //            this.v2 = v2;
    //            this.v3 = v3;
    //        }
    //    }

    //    private MeshGeometry3D geometry;
    //    private int index;
    //    private Dictionary<Int64, int> middlePointIndexCache;

    //    // add vertex to mesh, fix position to be on unit sphere, return index
    //    private int addVertex(Point3D p)
    //    {
    //        double length = Math.Sqrt(p.X * p.X + p.Y * p.Y + p.Z * p.Z);
    //        geometry.Positions.Add(new Point3D(p.X/length, p.Y/length, p.Z/length));
    //        return index++;
    //    }

    //    // return index of point in the middle of p1 and p2
    //    private int getMiddlePoint(int p1, int p2)
    //    {
    //        // first check if we have it already
    //        bool firstIsSmaller = p1 < p2;
    //        Int64 smallerIndex = firstIsSmaller ? p1 : p2;
    //        Int64 greaterIndex = firstIsSmaller ? p2 : p1;
    //        Int64 key = (smallerIndex << 32) + greaterIndex;

    //        int ret;
    //        if (this.middlePointIndexCache.TryGetValue(key, out ret))
    //        {
    //            return ret;
    //        }

    //        // not in cache, calculate it
    //        Point3D point1 = this.geometry.Positions[p1];
    //        Point3D point2 = this.geometry.Positions[p2];
    //        Point3D middle = new Point3D(
    //            (point1.X + point2.X) / 2.0, 
    //            (point1.Y + point2.Y) / 2.0, 
    //            (point1.Z + point2.Z) / 2.0);

    //        // add vertex makes sure point is on unit sphere
    //        int i = addVertex(middle); 

    //        // store it, return index
    //        this.middlePointIndexCache.Add(key, i);
    //        return i;
    //    }

    //    public MeshGeometry3D Create(int recursionLevel)
    //    {
    //        this.geometry = new MeshGeometry3D();
    //        this.middlePointIndexCache = new Dictionary<long, int>();
    //        this.index = 0;

    //        // create 12 vertices of a icosahedron
    //        var t = (1.0 + Math.Sqrt(5.0)) / 2.0;

    //        addVertex(new Point3D(-1,  t,  0));
    //        addVertex(new Point3D( 1,  t,  0));
    //        addVertex(new Point3D(-1, -t,  0));
    //        addVertex(new Point3D( 1, -t,  0));

    //        addVertex(new Point3D( 0, -1,  t));
    //        addVertex(new Point3D( 0,  1,  t));
    //        addVertex(new Point3D( 0, -1, -t));
    //        addVertex(new Point3D( 0,  1, -t));

    //        addVertex(new Point3D( t,  0, -1));
    //        addVertex(new Point3D( t,  0,  1));
    //        addVertex(new Point3D(-t,  0, -1));
    //        addVertex(new Point3D(-t,  0,  1));


    //        // create 20 triangles of the icosahedron
    //        var faces = new List<TriangleIndices>();

    //        // 5 faces around point 0
    //        faces.Add(new TriangleIndices(0, 11, 5));
    //        faces.Add(new TriangleIndices(0, 5, 1));
    //        faces.Add(new TriangleIndices(0, 1, 7));
    //        faces.Add(new TriangleIndices(0, 7, 10));
    //        faces.Add(new TriangleIndices(0, 10, 11));

    //        // 5 adjacent faces 
    //        faces.Add(new TriangleIndices(1, 5, 9));
    //        faces.Add(new TriangleIndices(5, 11, 4));
    //        faces.Add(new TriangleIndices(11, 10, 2));
    //        faces.Add(new TriangleIndices(10, 7, 6));
    //        faces.Add(new TriangleIndices(7, 1, 8));

    //        // 5 faces around point 3
    //        faces.Add(new TriangleIndices(3, 9, 4));
    //        faces.Add(new TriangleIndices(3, 4, 2));
    //        faces.Add(new TriangleIndices(3, 2, 6));
    //        faces.Add(new TriangleIndices(3, 6, 8));
    //        faces.Add(new TriangleIndices(3, 8, 9));

    //        // 5 adjacent faces 
    //        faces.Add(new TriangleIndices(4, 9, 5));
    //        faces.Add(new TriangleIndices(2, 4, 11));
    //        faces.Add(new TriangleIndices(6, 2, 10));
    //        faces.Add(new TriangleIndices(8, 6, 7));
    //        faces.Add(new TriangleIndices(9, 8, 1));


    //        // refine triangles
    //        for (int i = 0; i < recursionLevel; i++)
    //        {
    //            var faces2 = new List<TriangleIndices>();
    //            foreach (var tri in faces)
    //            {
    //                // replace triangle by 4 triangles
    //                int a = getMiddlePoint(tri.v1, tri.v2);
    //                int b = getMiddlePoint(tri.v2, tri.v3);
    //                int c = getMiddlePoint(tri.v3, tri.v1);

    //                faces2.Add(new TriangleIndices(tri.v1, a, c));
    //                faces2.Add(new TriangleIndices(tri.v2, b, a));
    //                faces2.Add(new TriangleIndices(tri.v3, c, b));
    //                faces2.Add(new TriangleIndices(a, b, c));
    //            }
    //            faces = faces2;
    //        }

    //        // done, now add triangles to mesh
    //        foreach (var tri in faces)
    //        {
    //            this.geometry.TriangleIndices.Add(tri.v1);
    //            this.geometry.TriangleIndices.Add(tri.v2);
    //            this.geometry.TriangleIndices.Add(tri.v3);
    //        }

    //        return this.geometry;        
    //    }
    //}
}

void MyMesh::Create2DCircle(float radius, unsigned int numberofsegments)
{
    int numverts = numberofsegments;

    if( m_SubmeshList.Length() == 0 )
    {
        CreateSubmeshes( 1 );
    }
    MyAssert( m_SubmeshList.Count() == 1 );

    m_SubmeshList[0]->m_NumVertsToDraw = (unsigned short)numverts;
    m_SubmeshList[0]->m_NumIndicesToDraw = 0;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer();
    }

    m_SubmeshList[0]->m_PrimitiveType = GL_TRIANGLE_FAN;

    // delete the old buffers, if we want an circle with more verts.
    if( sizeof(Vertex_XYZUV)*numverts > m_SubmeshList[0]->m_pVertexBuffer->m_DataSize )
    {
        m_SubmeshList[0]->m_pVertexBuffer->FreeBufferedData();
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZUV;
        Vertex_XYZUV* pVerts = MyNew Vertex_XYZUV[numverts];
        m_SubmeshList[0]->m_pVertexBuffer->InitializeBuffer( pVerts, sizeof(Vertex_XYZUV)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZUV, 0, "MyMesh_2dCircle", "Verts" );
    }

    Vertex_XYZUV_Alt* pVerts = (Vertex_XYZUV_Alt*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    m_SubmeshList[0]->m_pVertexBuffer->m_Dirty = true;

    float anglechange = -2.0f * PI / numverts;

    for( int i=0; i<numverts; i++ )
    {
        pVerts[i].pos.x = cos( i*anglechange ) * radius;
        pVerts[i].pos.y = sin( i*anglechange ) * radius;
        pVerts[i].pos.z = 0;

        pVerts[i].uv.x = cos( i*anglechange );
        pVerts[i].uv.y = sin( i*anglechange );
    }

    m_AABounds.Set( Vector3(0), Vector3(radius, radius, 0) );

    m_MeshReady = true;
}

void MyMesh::CreateEditorLineGridXZ(Vector3 center, float spacing, int halfnumbars)
{
    CreateSubmeshes( 1 );
    MyAssert( m_SubmeshList.Count() == 1 );

    MyAssert( m_SubmeshList[0]->m_pVertexBuffer == 0 );
    MyAssert( m_SubmeshList[0]->m_pIndexBuffer == 0 );

    unsigned char numverts = (unsigned char)((halfnumbars*2+1) * 2 * 2);
    unsigned int numindices = (halfnumbars*2+1) * 2 * 2; // halfnumbars*2+1centerline * 2axis * 2indicesperline.
    m_SubmeshList[0]->m_NumVertsToDraw = numverts; // not optimizing reuse of corner verts.
    m_SubmeshList[0]->m_NumIndicesToDraw = numindices;

    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
    {
        m_SubmeshList[0]->m_VertexFormat = VertexFormat_XYZ;
        Vertex_XYZ* pVerts = MyNew Vertex_XYZ[numverts];
        m_SubmeshList[0]->m_pVertexBuffer = g_pBufferManager->CreateBuffer( pVerts, sizeof(Vertex_XYZ)*numverts, GL_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, VertexFormat_XYZ, "MyMesh_GridPlane", "Verts" );
    }

    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
    {
        unsigned char* pIndices = MyNew unsigned char[numindices];
        m_SubmeshList[0]->m_pIndexBuffer = g_pBufferManager->CreateBuffer( pIndices, sizeof(unsigned char)*numindices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, false, 1, 1, "MyMesh_GridPlane", "Indices" );
    }

    m_SubmeshList[0]->m_PrimitiveType = GL_LINES;

    Vertex_XYZ* pVerts = (Vertex_XYZ*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
    unsigned char* pIndices = (unsigned char*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;

    unsigned char vertnum = 0;
    int indexnum = 0;

    // create verts
    for( int i = -halfnumbars; i<=halfnumbars; i++ )
    {
        pVerts[vertnum].x = center.x + i*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + halfnumbars*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + i*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + -halfnumbars*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + halfnumbars*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + i*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;

        pVerts[vertnum].x = center.x + -halfnumbars*spacing;
        pVerts[vertnum].y = center.y;
        pVerts[vertnum].z = center.z + i*spacing;
        pIndices[indexnum] = vertnum;
        vertnum++;
        indexnum++;
    }

    m_AABounds.Set( center, Vector3(halfnumbars*spacing, 0, halfnumbars*spacing) );

    m_MeshReady = true;
}

void MyMesh::CreateEditorTransformGizmoAxis(float length, float thickness, ColorByte color)
{
    CreateCylinder( thickness, 4, 0, length, 0, 1, 0, 1, 0, 1, 0, 1 );
    // TODOMaterials
    LOGError( LOGTag, "TransformGizmo color wasn't set properly... need to make a material for it\n" );
}

MaterialDefinition* MyMesh::GetMaterial(int submeshindex)
{
    return m_SubmeshList[submeshindex]->m_pMaterial;
}

void MyMesh::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    //if( m_SubmeshList.Count() == 0 )
    //    return;

    MyAssert( submeshindex < (int)m_SubmeshList.Count() );

    if( submeshindex == -1 )
    {
        for( unsigned int i=0; i<m_SubmeshList.Count(); i++ )
        {
            m_SubmeshList[i]->SetMaterial( pMaterial );
        }
    }
    else
    {
        m_SubmeshList[submeshindex]->SetMaterial( pMaterial );
    }
}

//void MyMesh::SetPosition(float x, float y, float z)
//{
//    m_Transform.SetTranslation( x, y, z );
//}
//
//void MyMesh::SetTransform(MyMatrix& matrix)
//{
//    m_Transform = matrix;
//}

void MyMesh::RebuildIndices()
{
    for( unsigned int i=0; i<m_SubmeshList.Count(); i++ )
        m_SubmeshList[i]->m_pIndexBuffer->Rebuild( 0, m_SubmeshList[i]->m_pIndexBuffer->m_DataSize );
}

void MyMesh::Draw(MyMatrix* matworld, MyMatrix* matviewproj, Vector3* campos, Vector3* camrot, MyLight* lights, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride)
{
    checkGlError( "start of MyMesh::Draw()" );

    if( m_MeshReady == false )
    {
        ParseFile();
    }

    for( unsigned int meshindex=0; meshindex<m_SubmeshList.Count(); meshindex++ )
    {
        m_SubmeshList[meshindex]->Draw( this, matworld, matviewproj, campos, camrot, lights, numlights, shadowlightVP, pShadowTex, pLightmapTex, pShaderOverride );
    }

    checkGlError( "end of MyMesh::Draw()" );
}

void MyMesh::RebuildAnimationMatrices(unsigned int animindex, double animtime, unsigned int oldanimindex, double oldanimtime, float perc)
{
    if( animindex >= m_pAnimations.Count() )
        return;

    // TODO: find a cleaner solution then checking every tick.
    //       this can/should only happen if a new mesh file replaced on old one with more animations.
    if( oldanimindex >= m_pAnimations.Count() )
        oldanimindex = 0;

    MyAnimationTimeline* pTimeline;
    float AnimationTime;

    MyAnimationTimeline* pTimelineOld = 0;
    float AnimationTimeOld = 0;

    {
        MyAnimation* pAnim = m_pAnimations[animindex];
        int timelineindex = m_pAnimations[animindex]->m_TimelineIndex;
        pTimeline = m_pAnimationTimelines[timelineindex];

        float TicksPerSecond = pTimeline->m_TicksPerSecond;
        double TimeInTicks = animtime * TicksPerSecond;
        double StartTime = pAnim->m_StartTime;
        double Duration = (double)pAnim->m_Duration;
        AnimationTime = (float)(StartTime + (float)fmod( TimeInTicks, Duration ));
    }

    if( oldanimindex && perc > 0 )
    {
        MyAnimation* pAnim = m_pAnimations[oldanimindex];
        int timelineindex = m_pAnimations[oldanimindex]->m_TimelineIndex;
        pTimelineOld = m_pAnimationTimelines[timelineindex];

        float TicksPerSecond = pTimeline->m_TicksPerSecond;
        double TimeInTicks = oldanimtime * TicksPerSecond;
        double StartTime = pAnim->m_StartTime;
        double Duration = (double)pAnim->m_Duration;
        AnimationTimeOld = (float)(StartTime + (float)fmod( TimeInTicks, Duration ));
    }

    MyMatrix matidentity;
    matidentity.SetIdentity();
    RebuildNode( pTimeline, AnimationTime, pTimelineOld, AnimationTimeOld, perc, 0, &matidentity );
}

void MyMesh::RebuildNode(MyAnimationTimeline* pTimeline, float animtime, MyAnimationTimeline* pOldTimeline, float oldanimtime, float perc, unsigned int nodeindex, MyMatrix* pParentTransform)
{
    MyAssert( nodeindex < m_pSkeletonNodeTree.Count() );

    MySkeletonNode* pNode = &m_pSkeletonNodeTree[nodeindex];

    int boneindex = pNode->m_BoneIndex;
    int channelindex = pTimeline->FindChannelIndexForNode( nodeindex );

    MyMatrix localtransform;
    if( channelindex != -1 )
    {
        Vector3 translation = pTimeline->GetInterpolatedTranslation( animtime, channelindex );
        MyQuat rotation = pTimeline->GetInterpolatedRotation( animtime, channelindex );
        Vector3 scale = pTimeline->GetInterpolatedScaling( animtime, channelindex );

        if( pOldTimeline )
        {
            Vector3 oldtranslation = pTimeline->GetInterpolatedTranslation( oldanimtime, channelindex );
            MyQuat oldrotation = pTimeline->GetInterpolatedRotation( oldanimtime, channelindex );
            //Vector3 oldscale = pTimeline->GetInterpolatedScaling( oldanimtime, channelindex );

            translation = translation + (oldtranslation - translation) * perc;
            rotation = MyQuat::Lerp( rotation, oldrotation, perc ).GetNormalized();
            //scale = scale;
        }

        localtransform.CreateSRT( scale, rotation, translation );
    }
    else
    {
        localtransform = pNode->m_Transform;
    }

    MyMatrix fulltransform = *pParentTransform * localtransform;

    if( boneindex != -1 )
        m_BoneFinalMatrices[boneindex] = fulltransform * m_BoneOffsetMatrices[boneindex];

    for( unsigned int cni=0; cni<pNode->m_pChildren.Count(); cni++)
    {
        RebuildNode( pTimeline, animtime, pOldTimeline, oldanimtime, perc, pNode->m_pChildren[cni]->m_SkeletonNodeIndex, &fulltransform );
    }
}

unsigned short MyMesh::GetNumVerts()
{
    if( m_SubmeshList[0]->m_pVertexBuffer == 0 )
        return 0;

    return (unsigned short)( m_SubmeshList[0]->m_pVertexBuffer->m_DataSize / GetStride( 0 ) );
}

unsigned int MyMesh::GetNumIndices()
{
    if( m_SubmeshList[0]->m_pIndexBuffer == 0 )
        return 0;

    return m_SubmeshList[0]->m_pIndexBuffer->m_DataSize / m_SubmeshList[0]->m_pIndexBuffer->m_BytesPerIndex;
}

Vertex_Base* MyMesh::GetVerts(bool markdirty)
{
    if( markdirty )
        m_SubmeshList[0]->m_pVertexBuffer->m_Dirty = true;

    return (Vertex_Base*)m_SubmeshList[0]->m_pVertexBuffer->m_pData;
}

unsigned short* MyMesh::GetIndices(bool markdirty)
{
    if( markdirty )
        m_SubmeshList[0]->m_pIndexBuffer->m_Dirty = true;

    return (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->m_pData;
}

unsigned int MyMesh::GetStride(unsigned int submeshindex)
{
    if( m_SubmeshList[submeshindex]->m_pVertexBuffer->m_VertexFormat == VertexFormat_Dynamic )
        return m_SubmeshList[submeshindex]->m_pVertexBuffer->m_pFormatDesc->stride;

    return g_VertexFormatSizes[m_SubmeshList[submeshindex]->m_pVertexBuffer->m_VertexFormat];
}
