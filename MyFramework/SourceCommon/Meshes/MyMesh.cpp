//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
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

MyMesh::MyMesh()
{
    m_pSourceFile = nullptr;
    m_LoadDefaultMaterials = true;
    m_ForceCheckForAnimationFile = false;
    m_MeshReady = false;

    m_AABounds.Set( Vector3(0), Vector3(0) );

    m_InitialScale = 1.0f; // TODO: Make this changable through interface somehow... reload/recreate mesh when changed?

    g_pMeshManager->AddMesh( this );

    m_pAnimations.AllocateObjects( MAX_ANIMATIONS );

    m_pAnimationControlFile = nullptr;

    m_pSetupCustomUniformsCallback = nullptr;
    m_pSetupCustomUniformsObjectPtr = nullptr;
}

MyMesh::~MyMesh()
{
    if( this->Prev )
        this->Remove();

    if( m_pSourceFile && m_pSourceFile->IsFinishedLoading() )
        m_pSourceFile->UnregisterFileFinishedLoadingCallback( this );

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
    m_ForceCheckForAnimationFile = true; // Needed for when a mesh file gets reloaded because of changes, this will reload the anim file.

    while( m_SubmeshList.Count() )
    {
        delete m_SubmeshList.RemoveIndex( 0 );
    }
    m_SubmeshList.FreeAllInList();

    m_MeshReady = false;
}

//============================================================================================================================
// Internal file loading functions
//============================================================================================================================
void MyMesh::CreateFromOBJFile()
{
    m_MeshReady = false;

    if( m_pSourceFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        OnFileFinishedLoadingOBJ( m_pSourceFile );
    }
    else
    {
        m_pSourceFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingOBJ );
    }
}

void MyMesh::OnFileFinishedLoadingOBJ(MyFileObject* pFile)
{
    if( pFile == m_pSourceFile )
    {
        pFile->UnregisterFileFinishedLoadingCallback( this );

        if( pFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            LoadBasicOBJ( pFile->GetBuffer(), &m_SubmeshList, false, 1.0f, &m_AABounds );

            MyAssert( m_SubmeshList.Count() > 0 );

            // TODO: Fix if obj loader ever supports submeshes.
            if( m_SubmeshList[0]->m_pVertexBuffer && m_SubmeshList[0]->m_pIndexBuffer )
            {
                //m_VertexFormat = m_pVertexBuffer->m_VertexFormat;
                uint32 indexBufferSize = m_SubmeshList[0]->m_pIndexBuffer->GetDataSize();
                uint32 bytesPerIndex = m_SubmeshList[0]->m_pIndexBuffer->GetBytesPerIndex();
                m_SubmeshList[0]->m_NumIndicesToDraw = indexBufferSize / bytesPerIndex;

                m_MeshReady = true;
            }
        }
        else
        {
            m_MeshReady = false;
        }
    }
}

void MyMesh::CreateFromMyMeshFile()
{
    m_MeshReady = false;

    if( m_pSourceFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        OnFileFinishedLoadingMyMesh( m_pSourceFile );
    }
    else
    {
        m_pSourceFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingMyMesh );
    }
}

void MyMesh::OnFileFinishedLoadingMyMesh(MyFileObject* pFile)
{
    MyAssert( pFile == m_pSourceFile );
    //LOGInfo( LOGTag, "%d: MyMesh::CreateFromMyMeshFile ( %s ) m_pAnimationControlFile = %d\n", this, pFile->GetFilenameWithoutExtension(), m_pAnimationControlFile );

    MyAssert( pFile );

    pFile->UnregisterFileFinishedLoadingCallback( this );

    // Load the animation file.
    if( m_ForceCheckForAnimationFile )
    {
        m_ForceCheckForAnimationFile = false;

        char animfilename[MAX_PATH];
        pFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".myaniminfo", animfilename, MAX_PATH );
#if MYFW_EDITOR
        // Only try to open the file if it exists, only in editor builds since file i/o isn't necessarily synchronous otherwise.
        if( g_pFileManager->DoesFileExist( animfilename ) )
        {
            m_pAnimationControlFile = g_pFileManager->RequestFile( animfilename ); // Adds a ref to the existing file or new one.
            m_pAnimationControlFile->RegisterFileFinishedLoadingCallback( this, StaticOnFileFinishedLoadingMyAnim );
            //LOGInfo( LOGTag, "g_pFileManager->DoesFileExist( %s ) returned true file = %d\n", animfilename, m_pAnimationControlFile );
        }
        else
        {
            //LOGInfo( LOGTag, "g_pFileManager->DoesFileExist( %s ) returned false\n", animfilename );
        }
#else
        m_pAnimationControlFile = g_pFileManager->RequestFile( animfilename ); // adds a ref to the existing file or new one.
#endif
    }

    m_MeshReady = false;

    // Is the mesh ready and the anim file is loaded or failed to load.
    if( pFile->GetFileLoadStatus() == FileLoadStatus_Success )
    {
        LoadMyMesh( pFile->GetBuffer(), &m_SubmeshList, m_InitialScale );

        if( m_pAnimationControlFile == nullptr )
        {
            LoadAnimationControlFile( nullptr );
        }
    }
}

void MyMesh::OnFileFinishedLoadingMyAnim(MyFileObject* pFile)
{
    MyAssert( m_pAnimationControlFile->GetFileLoadStatus() >= FileLoadStatus_Success );
    MyAssert( m_pAnimationControlFile == pFile );

    pFile->UnregisterFileFinishedLoadingCallback( this );

    //LOGInfo( LOGTag, "Animation File = %d\n", m_pAnimationControlFile );

    if( m_pAnimationControlFile->GetFileLoadStatus() == FileLoadStatus_Error_FileNotFound )
    {
        //LOGInfo( LOGTag, "Animation File - error loading file\n" );
        g_pFileManager->FreeFile( m_pAnimationControlFile );
        m_pAnimationControlFile = nullptr;
    }
        
    if( m_pAnimationControlFile )
    {
        //LOGInfo( LOGTag, "LoadAnimationControlFile = %s\n", m_pAnimationControlFile->m_pBuffer );
        LoadAnimationControlFile( m_pAnimationControlFile->GetBuffer() );
    }
    else
    {
        LoadAnimationControlFile( nullptr );
    }
}

void MyMesh::ParseFile()
{
    MyAssert( m_MeshReady == false );
    MyAssert( m_pSourceFile != nullptr );

    if( m_MeshReady == false )
    {
        if( m_pSourceFile != nullptr )
        {
            if( strcmp( m_pSourceFile->GetExtensionWithDot(), ".obj" ) == 0 )
            {
                CreateFromOBJFile();
            }
            if( strcmp( m_pSourceFile->GetExtensionWithDot(), ".mymesh" ) == 0 )
            {
                CreateFromMyMeshFile();
            }

            if( m_SubmeshList.Count() > 0 )
            {
                if( m_SubmeshList[0]->m_pMaterial && m_SubmeshList[0]->m_pMaterial->GetShader() == nullptr )
                {
                    // Guess at an appropriate shader for this mesh/material.
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
        if( m_SubmeshList[i]->m_pMaterial->GetShader() == nullptr )
        {
            // TODO: Actually write code here...
            m_SubmeshList[i]->m_pMaterial->SetShader( g_pShaderGroupManager->FindShaderGroupByName( "Shader_TintColor" ) );
        }
    }
}

//============================================================================================================================
// Getters.
//============================================================================================================================
unsigned short MyMesh::GetNumVerts()
{
    if( m_SubmeshList[0]->m_pVertexBuffer == nullptr )
        return 0;

    return (unsigned short)( m_SubmeshList[0]->m_pVertexBuffer->GetDataSize() / GetStride( 0 ) );
}

unsigned int MyMesh::GetNumIndices()
{
    if( m_SubmeshList[0]->m_pIndexBuffer == nullptr )
        return 0;

    uint32 indexBufferSize = m_SubmeshList[0]->m_pIndexBuffer->GetDataSize();
    uint32 bytesPerIndex = m_SubmeshList[0]->m_pIndexBuffer->GetBytesPerIndex();
    return indexBufferSize / bytesPerIndex;
}

Vertex_Base* MyMesh::GetVerts(bool markDirty)
{
    return (Vertex_Base*)m_SubmeshList[0]->m_pVertexBuffer->GetData( markDirty );
}

unsigned short* MyMesh::GetIndices(bool markDirty)
{
    return (unsigned short*)m_SubmeshList[0]->m_pIndexBuffer->GetData( markDirty );
}

unsigned int MyMesh::GetStride(unsigned int submeshindex)
{
    if( m_SubmeshList[submeshindex]->m_pVertexBuffer->GetVertexFormat() == VertexFormat_Dynamic )
        return m_SubmeshList[submeshindex]->m_pVertexBuffer->GetFormatDesc()->stride;

    return g_VertexFormatSizes[m_SubmeshList[submeshindex]->m_pVertexBuffer->GetVertexFormat()];
}

MaterialDefinition* MyMesh::GetMaterial(int submeshindex)
{
    return m_SubmeshList[submeshindex]->m_pMaterial;
}

//============================================================================================================================
// Setters.
//============================================================================================================================
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

//============================================================================================================================
// Submeshes and Buffers.
//============================================================================================================================
void MyMesh::CreateOneSubmeshWithBuffers(VertexFormat_Dynamic_Desc* pVertexFormatDesc, unsigned int numVerts, int bytesPerIndex, unsigned int numIndices, bool dynamic)
{
    MyAssert( m_SubmeshList.Length() == 0 );

    CreateSubmeshes( 1 );
    CreateVertexBuffer( 0, pVertexFormatDesc, numVerts, dynamic );
    CreateIndexBuffer( 0, bytesPerIndex, numIndices, dynamic );
}

void MyMesh::CreateSubmeshes(int numSubmeshes)
{
    MyAssert( m_SubmeshList.Length() == 0 );

    m_SubmeshList.AllocateObjects( numSubmeshes );
    for( int i=0; i<numSubmeshes; i++ )
        m_SubmeshList.Add( MyNew MySubmesh() );
}

void MyMesh::CreateVertexBuffer(int meshIndex, VertexFormat_Dynamic_Desc* pVertexFormatDesc, unsigned int numVerts, bool dynamic)
{
    MyAssert( meshIndex < (int)m_SubmeshList.Count() );
    MyAssert( m_SubmeshList[meshIndex]->m_pVertexBuffer == nullptr );
    MyAssert( m_SubmeshList[meshIndex]->m_pIndexBuffer == nullptr );

    MyRE::BufferUsages usage = MyRE::BufferUsage_StaticDraw;
    int numbuffers = 1;

    if( dynamic )
    {
        usage = MyRE::BufferUsage_DynamicDraw;
        numbuffers = 2;
    }

    {
        m_SubmeshList[meshIndex]->m_NumVertsToDraw = numVerts;
        m_SubmeshList[meshIndex]->m_VertexFormat = VertexFormat_Dynamic;

        m_SubmeshList[meshIndex]->m_pVertexBuffer = g_pBufferManager->CreateBuffer(
            nullptr, pVertexFormatDesc->stride*numVerts, MyRE::BufferType_Vertex, usage,
            false, numbuffers, VertexFormat_Dynamic, pVertexFormatDesc, "MyMesh", "Verts" );
    }
}

void MyMesh::CreateIndexBuffer(int meshIndex, int bytesPerIndex, unsigned int numIndices, bool dynamic)
{
    MyAssert( meshIndex < (int)m_SubmeshList.Count() );
    MyAssert( m_SubmeshList[meshIndex]->m_pIndexBuffer == nullptr );

    MyRE::BufferUsages usage = MyRE::BufferUsage_StaticDraw;
    int numbuffers = 1;

    if( dynamic )
    {
        usage = MyRE::BufferUsage_DynamicDraw;
        numbuffers = 2;
    }

    {
        m_SubmeshList[meshIndex]->m_NumIndicesToDraw = numIndices;

        m_SubmeshList[meshIndex]->m_pIndexBuffer = g_pBufferManager->CreateBuffer(
            nullptr, bytesPerIndex*numIndices, MyRE::BufferType_Index, usage,
            false, numbuffers, bytesPerIndex, "MyMesh", "Indices" );
    }
}

void MyMesh::SetIndexBuffer(BufferDefinition* pBuffer)
{
    MyAssert( m_SubmeshList.Count() > 0 );
    MyAssert( m_SubmeshList[0] );

    if( m_SubmeshList[0]->m_pIndexBuffer == pBuffer )
        return;

    if( m_SubmeshList[0]->m_pIndexBuffer )
        m_SubmeshList[0]->m_pIndexBuffer->Release();

    m_SubmeshList[0]->m_pIndexBuffer = pBuffer;
    pBuffer->AddRef();
}

void MyMesh::SetSourceFile(MyFileObject* pFile)
{
    if( m_pSourceFile == pFile )
        return;

    pFile->AddRef();

    // Free the current source file and unregister it's "finished loading" callback if one was set.
    if( m_pSourceFile )
        m_pSourceFile->UnregisterFileFinishedLoadingCallback( this );
    SAFE_RELEASE( m_pSourceFile );

    m_pSourceFile = pFile;

    // Free the old .myaniminfo file
    SAFE_RELEASE( m_pAnimationControlFile );

    ParseFile();
}

void MyMesh::RebuildIndices()
{
    for( unsigned int i=0; i<m_SubmeshList.Count(); i++ )
    {
        m_SubmeshList[i]->m_pIndexBuffer->Rebuild( 0, m_SubmeshList[i]->m_pIndexBuffer->GetDataSize() );
    }
}

//============================================================================================================================
// Draw.
//============================================================================================================================
void MyMesh::Draw(MyMatrix* pMatProj, MyMatrix* pMatView, MyMatrix* pMatWorld, Vector3* campos, Vector3* camrot, MyLight** lightptrs, int numlights, MyMatrix* shadowlightVP, TextureDefinition* pShadowTex, TextureDefinition* pLightmapTex, ShaderGroup* pShaderOverride)
{
    for( unsigned int meshindex=0; meshindex<m_SubmeshList.Count(); meshindex++ )
    {
        m_SubmeshList[meshindex]->Draw( this, pMatProj, pMatView, pMatWorld, campos, camrot, lightptrs, numlights, shadowlightVP, pShadowTex, pLightmapTex, pShaderOverride, false );
    }
}

//============================================================================================================================
// Animation.
//============================================================================================================================
void MyMesh::RebuildAnimationMatrices(unsigned int animindex, double animtime, unsigned int oldanimindex, double oldanimtime, float perc)
{
    if( animindex >= m_pAnimations.Count() )
        return;

    // TODO: Find a cleaner solution then checking every tick.
    //       This can/should only happen if a new mesh file replaced on old one with more animations.
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

    if( oldanimindex < m_pAnimations.Count() && perc > 0 )
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
    //MyClamp( perc, 0.0f, 1.0f );
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
            Vector3 oldtranslation = pOldTimeline->GetInterpolatedTranslation( oldanimtime, channelindex );
            MyQuat oldrotation = pOldTimeline->GetInterpolatedRotation( oldanimtime, channelindex );
            //Vector3 oldscale = pOldTimeline->GetInterpolatedScaling( oldanimtime, channelindex );

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

void MyMesh::RegisterSetupCustomUniformsCallback(void* pObjectPtr, SetupCustomUniformsCallbackFunc pCallback)
{
    m_pSetupCustomUniformsCallback = pCallback;
    m_pSetupCustomUniformsObjectPtr = pObjectPtr;
}
