//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "MyMeshLoader.h"

void MyMesh::LoadMyMesh(char* buffer, BufferDefinition** ppVBO, BufferDefinition** ppIBO)
{
    assert( ppVBO );
    assert( ppIBO );

    // get the number of verts/indices/bones.
    unsigned int totalverts = 0;
    unsigned int totalindices = 0;
    unsigned int totalbones = 0;

    cJSON* root = cJSON_Parse( buffer );

    cJSONExt_GetUnsignedInt( root, "TotalVerts", &totalverts );
    cJSONExt_GetUnsignedInt( root, "TotalIndices", &totalindices );
    cJSONExt_GetUnsignedInt( root, "TotalBones", &totalbones );

    unsigned int numuvchannels = 0;
    bool hasnormals = false;
    bool hastangents = false;
    bool hasbitangents = false;
    bool hascolor = false;
    unsigned int mostbonesinfluences = 0;

    cJSONExt_GetUnsignedInt( root, "VF-uv", &numuvchannels );
    cJSONExt_GetBool( root, "VF-normal", &hasnormals );
    cJSONExt_GetBool( root, "VF-tangent", &hastangents );
    cJSONExt_GetBool( root, "VF-bitangent", &hasbitangents );
    cJSONExt_GetBool( root, "VF-color", &hascolor );
    cJSONExt_GetUnsignedInt( root, "VF-mostweights", &mostbonesinfluences );

    VertexFormats vertexformat = FindAppropriateVertexFormat( numuvchannels, hasnormals, hastangents, hasbitangents, hascolor, mostbonesinfluences );

    // Read in bone info.
    if( totalbones )
    {
        m_pBoneNames.AllocateObjects( totalbones );
        m_pBoneMatrices.AllocateObjects( totalbones );

        cJSON* bones = cJSON_GetObjectItem( root, "Bones" );
        assert( totalbones == (unsigned int)cJSON_GetArraySize( bones ) );
        for( unsigned int i=0; i<totalbones; i++ )
        {
            cJSON* bone = cJSON_GetArrayItem( bones, i );
            if( bone )
            {
                //cJSON* nameobj = cJSON_GetObjectItem( bone, "Name" );
                //if( nameobj && nameobj->valuestring )
                //{
                //    int namelen = strlen( nameobj->valuestring );
                //    char* bonename = MyNew char[namelen+1];
                //    strcpy_s( bonename, namelen+1, nameobj->valuestring );

                //    m_pBoneNames.Add( bonename );
                //}
                int namelen = strlen( bone->valuestring );
                char* bonename = MyNew char[namelen+1];
                strcpy_s( bonename, namelen+1, bone->valuestring );

                m_pBoneNames.Add( bonename );
            }
        }
    }

    cJSON_Delete( root );

    // find a line that starts with "#raw"
    unsigned int bufferlen = strlen( buffer );
    unsigned int firstrawbyteindex = 0;
    if( bufferlen > 5 )
    {
        for( unsigned int i=0; i<bufferlen; i++ )
        {
            if( i > 1 && strncmp( &buffer[i], "\n#RAW", 5 ) == 0 )
            {
                firstrawbyteindex = i+6;
            }
        }
    }

    if( firstrawbyteindex != 0 )
    {
        int bytesperindex = 4;
        if( totalverts <= 256 )
            bytesperindex = 1;
        else if( totalverts <= 256*256 )
            bytesperindex = 2;

        unsigned int vertbuffersize = totalverts*g_VertexFormatSizes[vertexformat];
        unsigned int indexbuffersize = totalindices * bytesperindex;
        unsigned char* verts = MyNew unsigned char[vertbuffersize];
        unsigned char* indices = MyNew unsigned char[indexbuffersize];

        unsigned int rawbyteoffset = firstrawbyteindex;

        // read the raw data:
        {
            m_pBoneMatrices.BlockFill( &buffer[rawbyteoffset], sizeof(MyMatrix)*totalbones, totalbones );// read bone matrices
            rawbyteoffset += sizeof(MyMatrix)*totalbones;

            //(Vertex_XYZUVNorm_RGBA_4Bones*)verts,10
            memcpy( verts, &buffer[rawbyteoffset], vertbuffersize ); // read vert buffer bytes
            rawbyteoffset += vertbuffersize;

            memcpy( indices, &buffer[rawbyteoffset], indexbuffersize ); // read index buffer bytes
        }

        // give verts and indices pointers to BufferDefinition objects, which will handle the delete[]'s
        if( *ppVBO == 0 )
        {
            *ppVBO = g_pBufferManager->CreateBuffer();
        }

        if( *ppIBO == 0 )
        {
            *ppIBO = g_pBufferManager->CreateBuffer();
        }

        // The buffer will delete the allocated arrays of verts/indices
        (*ppVBO)->InitializeBuffer( verts, vertbuffersize, GL_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, vertexformat, "MyMeshLoader", "VBO" );
        (*ppIBO)->InitializeBuffer( indices, indexbuffersize, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, bytesperindex, "MyMeshLoader", "IBO" );

        //delete[] verts;
        //delete[] indices;
    }
}
