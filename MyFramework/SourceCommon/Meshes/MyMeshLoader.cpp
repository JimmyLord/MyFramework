//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "BufferManager.h"
#include "MeshManager.h"
#include "MyAnimation.h"
#include "MyMesh.h"
#include "MySubmesh.h"
#include "../Core/GameCore.h"
#include "../DataTypes/MyAABounds.h"
#include "../JSON/cJSONHelpers.h"
#include "../Shaders/VertexFormatManager.h"
#include "../Textures/MaterialDefinition.h"
#include "../Textures/MaterialManager.h"

void MyMesh::LoadMyMesh(const char* pBuffer, MyList<MySubmesh*>* pSubmeshList, float scale)
{
    MeshManager* pMeshManager = m_pGameCore->GetManagers()->GetMeshManager();
    MaterialManager* pMaterialManager = pMeshManager->GetMaterialManager();

    MyAssert( pSubmeshList );
    MyAssert( pSubmeshList->Length() == 0 );

    // Get the number of verts/indices/bones.
    unsigned int totalVerts = 0;
    unsigned int totalIndices = 0;
    unsigned int totalBones = 0;
    unsigned int totalNodes = 0;
    unsigned int totalAnimTimelines = 0;

    Vector3 minVert;
    Vector3 maxVert;

    // Skip the comments at the top of mymesh files by searching for '{'.
    const char* jsonStart = pBuffer;
    {
        int i=0;
        while( jsonStart[i] != '{' )
        {
            i++;
        }
        jsonStart += i;
    }

    cJSON* jRoot = cJSON_Parse( jsonStart );
    MyAssert( jRoot );

    cJSONExt_GetFloat( jRoot, "InitialScale", &m_InitialScale );
    scale = m_InitialScale;

    // Find a line that starts with "#RAW" and store it's index.
    unsigned int bufferLen = (int)strlen( pBuffer );
    unsigned int rawByteOffset = 0;
    if( bufferLen > 5 )
    {
        for( unsigned int i=0; i<bufferLen; i++ )
        {
            if( i > 1 && strncmp( &pBuffer[i], "\n#RAW", 5 ) == 0 )
            {
                rawByteOffset = i+5;
                break;
            }
        }
    }

    // Ensure raw data starts on 4-byte boundary.
    MyAssert( rawByteOffset%4 == 0 );

    {
        cJSONExt_GetUnsignedInt( jRoot, "TotalBones", &totalBones );
        cJSONExt_GetUnsignedInt( jRoot, "TotalNodes", &totalNodes );

        // Read in bone info.
        if( totalBones )
        {
            m_BoneNames.AllocateObjects( totalBones );
            m_BoneOffsetMatrices.AllocateObjects( totalBones );
            m_BoneFinalMatrices.AllocateObjects( totalBones );

            cJSON* jBonesArray = cJSON_GetObjectItem( jRoot, "Bones" );
            MyAssert( totalBones == (unsigned int)cJSON_GetArraySize( jBonesArray ) );
            for( unsigned int i=0; i<totalBones; i++ )
            {
                cJSON* jBone = cJSON_GetArrayItem( jBonesArray, i );
                if( jBone )
                {
                    int nameLen = (int)strlen( jBone->valuestring );
                    char* boneName = MyNew char[nameLen+1];
                    strcpy_s( boneName, nameLen+1, jBone->valuestring );

                    m_BoneNames.Add( boneName );
                }
            }
        }

        // Read in skeleton's node tree.
        if( totalNodes > 0 )
        {
            m_pSkeletonNodeTree.AllocateObjects( totalNodes );

            cJSON* jFirstNode = cJSON_GetObjectItem( jRoot, "Nodes" );

            LoadMyMesh_ReadNode( jFirstNode->child, nullptr );
        }

        // Read in the animation timelines.
        cJSON* jAnimArray = cJSON_GetObjectItem( jRoot, "AnimArray" );
        if( jAnimArray )
        {
            totalAnimTimelines = cJSON_GetArraySize( jAnimArray );
            if( totalAnimTimelines > 0 )
            {
                m_pAnimationTimelines.AllocateObjects( totalAnimTimelines );

                for( unsigned int ai=0; ai<totalAnimTimelines; ai++ )
                {
                    MyAnimationTimeline* pAnim = MyNew MyAnimationTimeline;
                    m_pAnimationTimelines.Add( pAnim );

                    cJSON* jAnim = cJSON_GetArrayItem( jAnimArray, ai );
                    pAnim->ImportFromJSON( jAnim );
                }
            }
        }
    }

    cJSON* jMeshArray = cJSON_GetObjectItem( jRoot, "Meshes" );
    if( jMeshArray )
    {
        cJSON* jMesh = nullptr;
        if( jMeshArray )
            jMesh = jMeshArray->child;

        int numMeshes = cJSON_GetArraySize( jMeshArray );
        pSubmeshList->AllocateObjects( numMeshes );
        for( int i=0; i<numMeshes; i++ )
            pSubmeshList->Add( MyNew MySubmesh() );

        int meshCount = 0;
        while( jMesh )
        {
            MyAssert( meshCount < (int)pSubmeshList->Count() );

            MySubmesh* pSubmesh = (*pSubmeshList)[meshCount];

            BufferDefinition** ppVBO = &pSubmesh->m_pVertexBuffer;
            BufferDefinition** ppIBO = &pSubmesh->m_pIndexBuffer;

            MyAssert( ppVBO );
            MyAssert( ppIBO );

            if( m_LoadDefaultMaterials )
            {
                cJSON* jMaterial = cJSON_GetObjectItem( jMesh, "Material" );
                if( jMaterial && jMaterial->valuestring )
                {
                    MaterialDefinition* pMaterial = pMaterialManager->LoadMaterial( jMaterial->valuestring );
                    if( pMaterial )
                        pSubmesh->SetMaterial( pMaterial );
                    pMaterial->Release();
                }
            }

            cJSONExt_GetUnsignedInt( jMesh, "TotalVerts", &totalVerts );
            cJSONExt_GetUnsignedInt( jMesh, "TotalIndices", &totalIndices );

            unsigned int numUVChannels = 0;
            bool hasNormals = false;
            bool hasTangents = false;
            bool hasBitangents = false;
            bool hasColor = false;
            unsigned int mostBonesInfluences = 0;

            cJSONExt_GetUnsignedInt( jMesh, "VF-uv", &numUVChannels );
            cJSONExt_GetBool( jMesh, "VF-normal", &hasNormals );
            cJSONExt_GetBool( jMesh, "VF-tangent", &hasTangents );
            cJSONExt_GetBool( jMesh, "VF-bitangent", &hasBitangents );
            cJSONExt_GetBool( jMesh, "VF-color", &hasColor );
            cJSONExt_GetUnsignedInt( jMesh, "VF-mostweights", &mostBonesInfluences );

            VertexFormat_Dynamic_Desc* pDesc = pMeshManager->GetVertexFormatManager()->GetDynamicVertexFormat( numUVChannels, hasNormals, hasTangents, hasBitangents, hasColor, mostBonesInfluences );

            // read this mesh's raw bytes, verts/indices/etc.
            if( rawByteOffset != 0 )
            {
                int bytesPerIndex = 4;
                if( totalVerts <= 256 )
                    bytesPerIndex = 1;
                else if( totalVerts <= 256*256 )
                    bytesPerIndex = 2;

                unsigned int vertBufferSize = totalVerts * pDesc->stride;
                unsigned int indexBufferSize = totalIndices * bytesPerIndex;
                unsigned char* verts = MyNew unsigned char[vertBufferSize];
                unsigned char* indices = MyNew unsigned char[indexBufferSize];

                // Read the raw data:
                {
                    // Advance the rawbyteoffset to land on next 4-byte boundary.
                    if( rawByteOffset%4 != 0 )
                        rawByteOffset += 4 - rawByteOffset%4;

                    // Read vert buffer bytes. //(Vertex_XYZUVNorm_RGBA_4Bones*)verts,10
                    memcpy( verts, &pBuffer[rawByteOffset], vertBufferSize );
                    rawByteOffset += vertBufferSize;

                    // Scale the verts if requested... should be done at export or not at all.
                    // Assumes position is the first attribute... ugh. TODO: rip this out.
                    if( scale != 1.0f )
                    {
                        for( unsigned int i=0; i<totalVerts; i++ )
                        {
                            ((float*)(&(verts[pDesc->stride * i])))[0] *= scale;
                            ((float*)(&(verts[pDesc->stride * i])))[1] *= scale;
                            ((float*)(&(verts[pDesc->stride * i])))[2] *= scale;
                        }
                    }

                    for( unsigned int i=0; i<totalVerts; i++ )
                    {
                        if( ((float*)(&(verts[pDesc->stride * i])))[0] < minVert.x || i == 0 ) minVert.x = ((float*)(&(verts[pDesc->stride * i])))[0];
                        if( ((float*)(&(verts[pDesc->stride * i])))[1] < minVert.y || i == 0 ) minVert.y = ((float*)(&(verts[pDesc->stride * i])))[1];
                        if( ((float*)(&(verts[pDesc->stride * i])))[2] < minVert.z || i == 0 ) minVert.z = ((float*)(&(verts[pDesc->stride * i])))[2];
                        if( ((float*)(&(verts[pDesc->stride * i])))[0] > maxVert.x || i == 0 ) maxVert.x = ((float*)(&(verts[pDesc->stride * i])))[0];
                        if( ((float*)(&(verts[pDesc->stride * i])))[1] > maxVert.y || i == 0 ) maxVert.y = ((float*)(&(verts[pDesc->stride * i])))[1];
                        if( ((float*)(&(verts[pDesc->stride * i])))[2] > maxVert.z || i == 0 ) maxVert.z = ((float*)(&(verts[pDesc->stride * i])))[2];
                    }

                    // Advance the rawbyteoffset to land on next 4-byte boundary.
                    if( rawByteOffset%4 != 0 )
                        rawByteOffset += 4 - rawByteOffset%4;

                    // Read index buffer bytes.
                    memcpy( indices, &pBuffer[rawByteOffset], indexBufferSize );
                    rawByteOffset += indexBufferSize;
                }

                // Give verts and indices pointers to BufferDefinition objects, which will handle the delete[]'s.
                if( *ppVBO == nullptr )
                {
                    BufferManager* pBufferManager = m_pGameCore->GetManagers()->GetBufferManager();
                    *ppVBO = pBufferManager->CreateBuffer();
                }

                if( *ppIBO == nullptr )
                {
                    BufferManager* pBufferManager = m_pGameCore->GetManagers()->GetBufferManager();
                    *ppIBO = pBufferManager->CreateBuffer();
                }

                // The buffer will delete the allocated arrays of verts/indices.
                (*ppVBO)->InitializeBuffer( verts, vertBufferSize, MyRE::BufferType_Vertex, MyRE::BufferUsage_StaticDraw, true, 1, VertexFormat_Dynamic, pDesc, "MyMeshLoader", "VBO" );
                (*ppIBO)->InitializeBuffer( indices, indexBufferSize, MyRE::BufferType_Index, MyRE::BufferUsage_StaticDraw, true, 1, bytesPerIndex, "MyMeshLoader", "IBO" );

                //delete[] verts;
                //delete[] indices;

                MyAssert( pSubmeshList->Count() > 0 );
                pSubmesh->m_VertexFormat = (*ppVBO)->GetVertexFormat();
                pSubmesh->m_NumIndicesToDraw = (*ppIBO)->GetDataSize() / (*ppIBO)->GetBytesPerIndex();
            }

            // Get the next mesh from the cJSON array.
            jMesh = jMesh->next;
            meshCount++;
        }

        // Read in the rest of the raw data.
        if( rawByteOffset != 0 )
        {
            // Advance the rawbyteoffset to land on next 4-byte boundary.
            if( rawByteOffset%4 != 0 )
                rawByteOffset += 4 - rawByteOffset%4;

            // Read bone offset matrices.
            {
                if( totalBones > 0 )
                {
                    m_BoneOffsetMatrices.BlockFill( &pBuffer[rawByteOffset], sizeof(MyMatrix)*totalBones, totalBones );
                    rawByteOffset += sizeof(MyMatrix)*totalBones;

                    for( unsigned int i=0; i<m_BoneOffsetMatrices.Count(); i++ )
                    {
                        m_BoneOffsetMatrices[i].m41 *= scale;
                        m_BoneOffsetMatrices[i].m42 *= scale;
                        m_BoneOffsetMatrices[i].m43 *= scale;
                    }
                }

                // Initialize all the final bone matrices to identity.
                MyMatrix matidentity;
                matidentity.SetIdentity();
                for( unsigned int i=0; i<totalBones; i++ )
                {
                    m_BoneFinalMatrices.Add( matidentity );
                }
            }

            // Read in the node transforms
            for( unsigned int ni=0; ni<totalNodes; ni++ )
            {
                MyAssert( ni < m_pSkeletonNodeTree.Count() );

                // TODO: This line fails on Android(gcc)... no clue why, so did it with a memcpy.
                //m_pSkeletonNodeTree[ni].m_Transform = *(MyMatrix*)&buffer[rawByteOffset];
                memcpy( &m_pSkeletonNodeTree[ni].m_Transform, &pBuffer[rawByteOffset], sizeof(MyMatrix) );

                rawByteOffset += sizeof(MyMatrix);

                m_pSkeletonNodeTree[ni].m_Transform.m41 *= scale;
                m_pSkeletonNodeTree[ni].m_Transform.m42 *= scale;
                m_pSkeletonNodeTree[ni].m_Transform.m43 *= scale;
            }

            // Read animation channels.
            for( unsigned int ai=0; ai<totalAnimTimelines; ai++ )
            {
                MyAssert( ai < m_pAnimationTimelines.Count() );

                // Ensure animation data starts on 4-byte boundary.
                MyAssert( rawByteOffset%4 == 0 );

                rawByteOffset += m_pAnimationTimelines[ai]->ImportChannelsFromBuffer( &pBuffer[rawByteOffset], scale );
            }
        }
    }

    cJSON_Delete( jRoot );

    // If something failed to load, don't consider the mesh ready.
    // TODO: Add better error handling... maybe.
    if( pSubmeshList->Count() > 0 )
    {
        Vector3 center = (minVert + maxVert) / 2;
        m_AABounds.Set( center, maxVert - center );

        m_MeshReady = true;
    }
}

int MyMesh::FindBoneIndexByName(char* name)
{
    for( unsigned int i=0; i<m_BoneNames.Count(); i++ )
    {
        if( strcmp( m_BoneNames[i], name ) == 0 )
        {
            return i;
        }
    }

    return -1;
}

void MyMesh::LoadMyMesh_ReadNode(cJSON* jNode, MySkeletonNode* pParentSkelNode)
{
    MySkeletonNode skelNodeToAdd;
    int skelNodeIndex = m_pSkeletonNodeTree.Count();
    m_pSkeletonNodeTree.Add( skelNodeToAdd );

    MyAssert( skelNodeIndex < (int)m_pSkeletonNodeTree.Count() );

    MySkeletonNode& skelNode = m_pSkeletonNodeTree[skelNodeIndex];

    // Add this node as a child of the parent.
    if( pParentSkelNode )
        pParentSkelNode->m_pChildren.Add( &m_pSkeletonNodeTree[skelNodeIndex] );

    char* name = jNode->string;
    MyAssert( name );

    skelNode.m_SkeletonNodeIndex = skelNodeIndex;
    skelNode.m_BoneIndex = FindBoneIndexByName( name );

    // Add the name.
    int nameLen = (int)strlen(name);
    skelNode.m_Name = MyNew char[nameLen+1];
    strcpy_s( skelNode.m_Name, nameLen+1, name );

    // Get count of children.
    unsigned int childCount = cJSONExt_GetDirectChildCount( jNode );

    if( childCount > 0 )
    {
        // Allocate enough pointers for each child.
        skelNode.m_pChildren.AllocateObjects( childCount );

        // Recurse through the children.
        cJSON* jChildNode = jNode->child;
        while( jChildNode )
        {
            LoadMyMesh_ReadNode( jChildNode, &m_pSkeletonNodeTree[skelNodeIndex] );
            jChildNode = jChildNode->next;
        }
    }
}

#if MYFW_EDITOR
void AddPaddingToReachNext4ByteBoundary(FILE* file)
{
    int bytesWritten = ftell( file );
    while( bytesWritten % 4 != 0 )
    {
        fwrite( " ", 1, 1, file );
        bytesWritten++;
    }
}

void MyMesh::ExportToFile(const char* filename) const
{
    unsigned int numMeshes = m_SubmeshList.size();

    // Create a json object.
    cJSON* jRoot = cJSON_CreateObject();

    cJSON* jMeshArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "Meshes", jMeshArray );

    // This is currently only used by heightmaps, so only saving 1 submesh for now.
    //for( unsigned int mati=0; mati<nummaterials; mati++ )
    {
        //if( MaterialsInUse[mati] == false )
        //    continue;

        cJSON* jMesh = cJSON_CreateObject();
        cJSON_AddItemToArray( jMeshArray, jMesh );

        unsigned int totalVerts = 0;
        unsigned int totalIndices = 0;

        //for( unsigned int mi=0; mi<numMeshes; mi++ )
        {
            unsigned int mi=0;

            if( m_SubmeshList[mi] )//.m_MaterialIndex == mati )
            {
                totalVerts += m_SubmeshList[mi]->m_pVertexBuffer->GetDataSize() / m_SubmeshList[mi]->m_pVertexBuffer->GetStride();
                totalIndices += m_SubmeshList[mi]->m_pIndexBuffer->GetDataSize() / m_SubmeshList[mi]->m_pIndexBuffer->GetBytesPerIndex();
            }
        }

        // Add the material index.
        char materialRelativePath[260];
        sprintf_s( materialRelativePath, 260, "%s.mymaterial", m_SubmeshList[0]->GetMaterial()->GetName() );
        cJSON_AddStringToObject( jMesh, "Material", materialRelativePath );

        // Add the vert/index/bone count.
        cJSON_AddNumberToObject( jMesh, "TotalVerts", totalVerts );
        cJSON_AddNumberToObject( jMesh, "TotalIndices", totalIndices );

        // Hardcoded vertex format for heightmaps. // VertexFormat_XYZUVNorm
        cJSONExt_AddNumberToObjectIfDiffers( jMesh, "VF-uv", 1, 0 );
        cJSONExt_AddNumberToObjectIfDiffers( jMesh, "VF-normal", true, false );
        //cJSONExt_AddNumberToObjectIfDiffers( jMesh, "VF-tangent", m_HasTangents, false );
        //cJSONExt_AddNumberToObjectIfDiffers( jMesh, "VF-bitangent", m_HasBitangents, false );
        cJSONExt_AddNumberToObjectIfDiffers( jMesh, "VF-color", false, false );
        cJSONExt_AddNumberToObjectIfDiffers( jMesh, "VF-mostweights", 0, 0 );

        //// quick debug, write out first meshchunks verts/indices as readable text.
        //{
        //    unsigned int numvertsinthischunk = m_MeshChunks[0].m_Vertices.size();
        //    unsigned int numindicesinthischunk = m_MeshChunks[0].m_Indices.size();

        //    cJSON* verts = cJSON_CreateArray();
        //    cJSON_AddItemToObject( mesh, "verts", verts );

        //    for( unsigned int i=0; i<numvertsinthischunk; i++ )
        //    {
        //        cJSON* pos = cJSON_CreateObject();
        //        cJSONExt_AddFloatArrayToObject( pos, "pos", &m_MeshChunks[0].m_Vertices[i].pos.x, 3 );
        //        cJSON_AddItemToArray( verts, pos );
        //    }

        //    cJSONExt_AddIntArrayToObject( mesh, "indices", (int*)&m_MeshChunks[0].m_Indices.front(), numindicesinthischunk );
        //}
    }

    // Save the json object to disk.
    //char* jsonstr = cJSON_PrintUnformatted( jRoot );
    char* jsonString = cJSON_Print( jRoot );

    char outputFilename[260];
    size_t filenameLen = strlen(filename);
    if( filenameLen > 7 && strcmp( &filename[filenameLen-7], ".mymesh" ) == 0 )
        sprintf_s( outputFilename, 260, "%s", filename );
    else
        sprintf_s( outputFilename, 260, "%s.mymesh", filename );

    FILE* file;
#if MYFW_WINDOWS
    fopen_s( &file, outputFilename, "wb" );
#else
    file = fopen( outputFilename, "wb" );
#endif
    fprintf( file, "//\n" );
    fprintf( file, "//\n" );
    fprintf( file, "// Generated by MeshTool.exe, don't modify by hand... raw block must land of 4-byte boundary\n" );
    fprintf( file, "//\n" );
    fprintf( file, "//\n" );
    fprintf( file, "\n" );
    fprintf( file, "%s", jsonString );
    cJSONExt_free( jsonString );

    // Raw data needs to land on 4-byte boundary.
    int bytesWritten = ftell( file ) + 1; // Add 1 for the \n that will be written before #RAW.
    while( bytesWritten % 4 != 0 )
    {
        fwrite( " ", 1, 1, file );
        bytesWritten++;
    }

    // Write out a marker for start of raw data.
    // Raw data format:
    //    - for each material/mesh chunk
    //      - vert dump(format listed in json struct)       - padded to start on 4-byte boundary
    //      - index dump(format based on number of verts)   - padded to start on 4-byte boundary
    //    - bone offset matrices                            - padded to start on 4-byte boundary
    //    - node transforms
    //    - animation data
    const char rawDelimiter[] = "\n#RAW";
    fwrite( rawDelimiter, 5, 1, file );

    bytesWritten = ftell( file );
    MyAssert( bytesWritten % 4 == 0 );

    //for( unsigned int mati=0; mati<nummaterials; mati++ )
    unsigned int mati=0;
    {
        unsigned int totalVerts = 0;
        unsigned int totalIndices = 0;

        //for( unsigned int mi=0; mi<numMeshes; mi++ )
        {
            unsigned int mi=0;

            if( m_SubmeshList[mi] ) //.m_MaterialIndex == mati )
            {
                totalVerts += m_SubmeshList[mi]->m_pVertexBuffer->GetDataSize() / m_SubmeshList[mi]->m_pVertexBuffer->GetStride();
                totalIndices += m_SubmeshList[mi]->m_pIndexBuffer->GetDataSize() / m_SubmeshList[mi]->m_pIndexBuffer->GetBytesPerIndex();
            }
        }

        // Raw dump of vertex info.
        //for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
        {
            unsigned int mi=0;

            //if( m_MeshChunks[mi].m_MaterialIndex != mati )
            //    continue;

            // Add padding to file.
            AddPaddingToReachNext4ByteBoundary( file );

            unsigned int numVertsInThisChunk = m_SubmeshList[mi]->m_pVertexBuffer->GetDataSize() / m_SubmeshList[mi]->m_pVertexBuffer->GetStride();

            Vertex_XYZUVNorm* pVerts = (Vertex_XYZUVNorm*)m_SubmeshList[mi]->m_pVertexBuffer->GetData( false );

            for( unsigned int vi=0; vi<numVertsInThisChunk; vi++ )
            {
                fwrite( &pVerts[vi].pos, sizeof(Vector3), 1, file );

                //for( unsigned int i=0; i<1; i++ )
                fwrite( &pVerts[vi].uv, sizeof(Vector2), 1, file );

                //if( m_HasNormals )
                fwrite( &pVerts[vi].normal, sizeof(Vector3), 1, file );

                //if( m_HasTangents )
                //    fwrite( &pVerts[vi].tangent, sizeof(Vector3), 1, file );

                //if( m_HasBitangents )
                //    fwrite( &pVerts[vi].bitangent, sizeof(Vector3), 1, file );

                //if( m_HasColor )
                //    fwrite( &pVerts[vi].color, sizeof(unsigned char) * 4, 1, file );

                //for( unsigned int i=0; i<m_MostBonesInfluences; i++ )
                //    fwrite( &pVerts[vi].boneindices[i], sizeof(unsigned char), 1, file );

                //for( unsigned int i=0; i<m_MostBonesInfluences; i++ )
                //    fwrite( &pVerts[vi].weights[i], sizeof(float), 1, file );
            }
        }

        // Raw dump of indices.
        int vertCount = 0;
        //for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
        {
            unsigned int mi=0;

            //if( m_MeshChunks[mi].m_MaterialIndex != mati )
            //    continue;

            // Add padding to file.
            AddPaddingToReachNext4ByteBoundary( file );

            unsigned int numIndicesInThisChunk = m_SubmeshList[mi]->m_pIndexBuffer->GetDataSize() / m_SubmeshList[mi]->m_pIndexBuffer->GetBytesPerIndex();

            unsigned int* pIndices = (unsigned int*)m_SubmeshList[mi]->m_pIndexBuffer->GetData( false );

            for( unsigned int i=0; i<numIndicesInThisChunk; i++ )
            {
                if( totalVerts <= 256 ) // Write indices as unsigned chars.
                {
                    unsigned char index = (unsigned char)(vertCount + pIndices[i]);
                    fwrite( &index, sizeof(unsigned char), 1, file );
                }
                else if( totalVerts <= 256*256 ) // Write indices as unsigned shorts.
                {
                    unsigned short index = (unsigned short)(vertCount + pIndices[i]);
                    fwrite( &index, sizeof(unsigned short), 1, file );
                }
                else // Write indices as unsigned ints.
                {
                    unsigned int index = vertCount + pIndices[i];
                    fwrite( &index, sizeof(unsigned int), 1, file );
                }
            }

            vertCount += numIndicesInThisChunk;
        }
    }

    //// Add padding to file.
    //AddPaddingToReachNext4ByteBoundary( file );

    //// Dump more raw data to our file, these come after all verts/indices for each mesh material chunk.
    //{
    //    // Raw dump of bone matrices.
    //    unsigned int totalbones = m_Bones.size();
    //    for( unsigned int bi=0; bi<totalbones; bi++ )
    //    {
    //        fwrite( &m_Bones[bi].m_OffsetMatrix.m11, sizeof(MyMatrix), 1, file );
    //    }
    //    DumpRawNodeTransformsFromScene( file );
    //    DumpRawAnimationDataFromScene( file );
    //}

    fclose( file );

    cJSON_Delete( jRoot );

    //delete[] MaterialsInUse;
}

void MyMesh::SaveAnimationControlFile()
{
    char filename[MAX_PATH];
    m_pSourceFile->GenerateNewFullPathExtensionWithSameNameInSameFolder( ".myaniminfo", filename, MAX_PATH );

    cJSON* jRoot = cJSON_CreateObject();

    cJSON* jAnimArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jRoot, "Anims", jAnimArray );

    for( unsigned int i=0; i<m_pAnimations.Count(); i++ )
    {
        cJSON* jAnim = cJSON_CreateObject();

        cJSON_AddItemToArray( jAnimArray, jAnim );

        cJSON_AddStringToObject( jAnim, "Name", m_pAnimations[i]->m_Name );
        cJSON_AddNumberToObject( jAnim, "TimelineIndex", m_pAnimations[i]->m_TimelineIndex );
        cJSON_AddNumberToObject( jAnim, "StartTime", m_pAnimations[i]->m_StartTime );
        cJSON_AddNumberToObject( jAnim, "Duration", m_pAnimations[i]->m_Duration );
    }

    // Dump animArray to disk.
    char* jsonString = cJSON_Print( jRoot );
    cJSON_Delete( jRoot );

    FILE* pFile = nullptr;
#if MYFW_WINDOWS
    fopen_s( &pFile, filename, "wb" );
#else
    pFile = fopen( filename, "wb" );
#endif
    if( pFile != nullptr )
    {
        fprintf( pFile, "%s", jsonString );
        fclose( pFile );
    }
    else
    {
        LOGError( "File failed to open: %s\n", filename );
    }

    cJSONExt_free( jsonString );
}
#endif

void MyMesh::LoadAnimationControlFile(const char* pBuffer)
{
    // If the file doesn't exist, create a single animation for each timeline.
    if( pBuffer == nullptr )
    {
        MyAssert( m_pAnimationTimelines.Count() < MAX_ANIMATIONS );

        for( unsigned int i=0; i<m_pAnimationTimelines.Count(); i++ )
        {
            MyAnimation* pAnim = MyNew MyAnimation;

            pAnim->SetName( m_pAnimationTimelines[i]->m_Name );
            pAnim->m_TimelineIndex = i;
            pAnim->m_StartTime = 0;
            pAnim->m_Duration = m_pAnimationTimelines[i]->m_Duration;

            //LOGInfo( LOGTag, "Creating animation: %s %f seconds\n", m_pAnimationTimelines[i]->m_Name, m_pAnimationTimelines[i]->m_Duration );

            m_pAnimations.Add( pAnim );
        }

        // Only save if the user adds animations via the editor interface.
        //SaveAnimationControlFile();
    }
    else
    {
        cJSON* jRoot = cJSON_Parse( pBuffer );

        if( jRoot )
        {
            cJSON* jAnimArray = cJSON_GetObjectItem( jRoot, "Anims" );

            int numanims = cJSON_GetArraySize( jAnimArray );
            for( int i=0; i<numanims; i++ )
            {
                cJSON* jAnim = cJSON_GetArrayItem( jAnimArray, i );

                MyAnimation* pAnim = MyNew MyAnimation;

                cJSON* obj = cJSON_GetObjectItem( jAnim, "Name" );
                if( obj )
                    pAnim->SetName( obj->valuestring );
                cJSONExt_GetInt( jAnim, "TimelineIndex", &pAnim->m_TimelineIndex );
                cJSONExt_GetFloat( jAnim, "StartTime", &pAnim->m_StartTime );
                cJSONExt_GetFloat( jAnim, "Duration", &pAnim->m_Duration );

                m_pAnimations.Add( pAnim );
            }
        }

        cJSON_Delete( jRoot );
    }
}
