//
// Copyright (c) 2014-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "OBJLoader.h"
#include "../DataTypes/MyAABounds.h"
#include "../Helpers/MyTime.h"
#include "../Meshes/BufferManager.h"
#include "../Meshes/MySubmesh.h"
#include "../Shaders/VertexFormats.h"

// This code is assuming a list of faces longer than 3 is a triangle fan and turns it into a list of tris.
// TODO: OBJs can have concave face lists... so this is a step better, but still broken.

struct FaceInfo
{
    int attributes[4][3];

    int vertindex[4];
};

#if _DEBUG
char* LoadFile(const char* filename, long* length)
{
    char* filecontents = nullptr;

    FILE* filehandle;
#if MYFW_WINDOWS
    errno_t error = fopen_s( &filehandle, filename, "rb" );
#else
    filehandle = fopen( filename, "rb" );//"Data/Scenes/test.scene", "rb" );
#endif

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = MyNew char[size+1];
        fread( filecontents, size, 1, filehandle );
        filecontents[size] = '\0';

        if( length )
            *length = size;

        fclose( filehandle );
    }

    return filecontents;
}
#endif

int FindIndexOfNextSpaceOrTab(const char* buffer, int index)
{
    // Find first non-whitespace.
    while( buffer[index] != '\t' && buffer[index] != ' ' )
    {
        index++;
    }

    return index;
}

int FindIndexOfFirstNonWhitespace(const char* buffer, int index)
{
    // Find first non-whitespace.
    while( buffer[index] == '\r' || buffer[index] == '\n' || buffer[index] == '\t' || buffer[index] == ' ' )
    {
        index++;
    }

    return index;
}

int FindIndexOfFirstNonSpaceOrTab(const char* buffer, int index)
{
    // Find first non-whitespace.
    while( buffer[index] == '\t' || buffer[index] == ' ' )
    {
        index++;
    }

    return index;
}

int FindIndexOfFirstNonWhitespaceOfNextLine(const char* buffer, int index)
{
    // Find end of line.
    while( buffer[index] != '\n' && buffer[index] != '\0' )
    {
        index++;
    }

    return FindIndexOfFirstNonWhitespace( buffer, index );
}

float ReadFloatAndMoveOn(const char* buffer, int* index)
{
    char numberBuffer[100];
    int numberBufferIndex = 0;

    while( (buffer[*index] >= '0' && buffer[*index] <= '9') || buffer[*index] == '.' || buffer[*index] == '-' || buffer[*index] == 'e' || buffer[*index] == '+' )
    {
        numberBuffer[numberBufferIndex] = buffer[*index];
        numberBuffer[numberBufferIndex+1] = '\0';
        numberBufferIndex++;

        (*index)++;
    }

    return (float)atof( numberBuffer );
}

int ReadIntAndMoveOn(const char* buffer, int* index)
{
    char numberBuffer[100];
    int numberBufferIndex = 0;

    while( (buffer[*index] >= '0' && buffer[*index] <= '9') || buffer[*index] == '-' )
    {
        numberBuffer[numberBufferIndex] = buffer[*index];
        numberBuffer[numberBufferIndex+1] = '\0';
        numberBufferIndex++;

        (*index)++;
    }

    return atoi( numberBuffer );
}

Vector3 ParseVertex(const char* buffer, int index)
{
    MyAssert( buffer[index] == 'v' );

    Vector3 outVector(0,0,0);

    // Jump to the first space after the v, vt, or vn.
    index = FindIndexOfNextSpaceOrTab( buffer, index );
    index = FindIndexOfFirstNonWhitespace( buffer, index+1 );

    int count = 0;

    while( buffer[index] != '\r' && buffer[index] != '\n' && buffer[index] != '\0' )
    {
        if( buffer[index] == ' ' || buffer[index] == '\t' )
        {
            count++;
            index = FindIndexOfFirstNonSpaceOrTab( buffer, index );
        }
        else
        {
            float value = ReadFloatAndMoveOn( buffer, &index );

            if( count > 2 )
                MyAssert( value == 1 ); // Seen "v 3.25000 -2.48000 14.0000 1.00000" in a file, not sure why 1 is there.
            else
                outVector[count] = value;
        }
    }

    //if( outvector.LengthSquared() == 0 )
    //    int bp = 1;

    return outVector;
}

int ParseFaceInfo(FaceInfo* faces, const char* buffer, int index)
{
    // Divide the face into a bunch of triangles... assumes face is defined as triangle fan, which it isn't.

    int trianglecount = 0;

    MyAssert( buffer[index] == 'f' );

    // jump to the first number
    index = FindIndexOfFirstNonWhitespace( buffer, index+1 );

    FaceInfo faceInfo;
    memset( &faceInfo, 0, sizeof(FaceInfo) );

    int numVerts = 0;
    int numAttrs = 0;

    int lastNumber = -1;

    while( true )
    {
        if( buffer[index] == ' ' || buffer[index] == '\t' || buffer[index] == '\r' || buffer[index] == '\n' || buffer[index] == '\0' )
        {
            MyAssert( numVerts < 3 );

            if( lastNumber > 0 )
            {
                faceInfo.attributes[numVerts][numAttrs] = lastNumber;
                numVerts++;

                if( numVerts == 3 ) // If we filled a triangle, create a new one.
                {
                    faces[trianglecount] = faceInfo;
                    trianglecount++;

                    // Assuming a triangle fan, so copy vert2 into vert1, vert0 stays the same, then read a new vert2.
                    for( int i=0; i<3; i++ )
                        faceInfo.attributes[1][i] = faceInfo.attributes[2][i];
                    numVerts--;
                }
    
                index = FindIndexOfFirstNonSpaceOrTab( buffer, index );
            }

            lastNumber = -1; // Invalid index.
            numAttrs = 0;

            if( buffer[index] == '\r' || buffer[index] == '\n' || buffer[index] == '\0' )
                break;
        }
        else if( buffer[index] == '/' )
        {
            MyAssert( numVerts < 3 );
            MyAssert( numAttrs < 3 );

            faceInfo.attributes[numVerts][numAttrs] = lastNumber;
            lastNumber = 0; // Invalid index.

            numAttrs++;
            index++;
        }
        else
        {
            lastNumber = ReadIntAndMoveOn( buffer, &index );
        }
    }

    return trianglecount;
}

#if _DEBUG
void LoadBasicOBJFromFile(char* filename, MyList<MySubmesh*>* pSubmeshList, bool removeDuplicateVertices, float scale, MyAABounds* pAABB)
{
    MyAssert( pSubmeshList );

    MyAssert( false ); // Don't use this function, file i/o should be done through a RequestFile call.

    long size;
    char* buffer = LoadFile( filename, &size );

    LoadBasicOBJ( buffer, pSubmeshList, removeDuplicateVertices, scale, pAABB );

    delete[] buffer;
}
#endif

void SetValueOfIndex(unsigned char* indices, int index, unsigned int value, int indexBytes)
{
    if( indexBytes == 1 )
    {
        *(unsigned char*)&indices[index] = (unsigned char)value;
    }
    if( indexBytes == 2 )
    {
        *(unsigned short*)&indices[index] = (unsigned short)value;
    }
    if( indexBytes == 4 )
    {
        *(unsigned int*)&indices[index] = (unsigned int)value;
    }
}

void LoadBasicOBJ(const char* buffer, MyList<MySubmesh*>* pSubmeshList, bool removeDuplicateVertices, float scale, MyAABounds* pAABB)
{
    MyAssert( pSubmeshList );
    MyAssert( pSubmeshList->Length() == 0 );

    pSubmeshList->AllocateObjects( 1 );
    pSubmeshList->Add( MyNew MySubmesh() );

    MyAssert( pSubmeshList->Count() > 0 );

    // TODO: Fix this ugliness from adding submeshes... or relegate obj loading to MeshTool.
    BufferDefinition** ppVBO = &(*pSubmeshList)[0]->m_pVertexBuffer;
    BufferDefinition** ppIBO = &(*pSubmeshList)[0]->m_pIndexBuffer;

    MyAssert( ppVBO );
    MyAssert( ppIBO );

    Vector3 minVert;
    Vector3 maxVert;

    // Count the number of faces and each type of vertex attribute.
    int faceCount = 0;
    int vertPosCount = 0;
    int vertUVCount = 0;
    int vertNormalCount = 0;

    {
        int index = FindIndexOfFirstNonWhitespace( buffer, 0 );
        while( buffer[index] != '\0' )
        {
            if( buffer[index] == 'v' )
            {
                if( buffer[index+1] == ' ' )        vertPosCount++;
                else if( buffer[index+1] == 't' )   vertUVCount++;
                else if( buffer[index+1] == 'n' )   vertNormalCount++;
            }
            else if( buffer[index] == 'f' )
            {
                // Count the number of verices, I'll assume the list produces a convex shape.
                // So, numfaces is 2 less than the number of verts.
                int numVertsInFace = -1;
                while( buffer[index] != '\r' && buffer[index] != '\n' )
                {
                    // Skip all non-whitespace.
                    while( buffer[index] != '\t' && buffer[index] != ' ' && buffer[index] != '\r' && buffer[index] != '\n' )
                        index++;

                    numVertsInFace++;

                    // Skip all whitespace.
                    while( buffer[index] == '\t' || buffer[index] == ' ' )
                        index++;
                }

                faceCount += numVertsInFace-2;
            }

            index = FindIndexOfFirstNonWhitespaceOfNextLine( buffer, index );
        }
    }

    // Allocate individual buffers for pos/uv/norm, then parse again to store values.
    Vector3* VertexPositions = MyNew Vector3[vertPosCount];
    Vector2* VertexUVs = vertUVCount ? MyNew Vector2[vertUVCount] : 0;
    Vector3* VertexNormals = vertNormalCount ? MyNew Vector3[vertNormalCount] : 0;
    FaceInfo* Faces = MyNew FaceInfo[faceCount];

    {
        int index = FindIndexOfFirstNonWhitespace( buffer, 0 );
        faceCount = 0;
        vertPosCount = 0;
        vertUVCount = 0;
        vertNormalCount = 0;

        while( buffer[index] != '\0' )
        {
            if( buffer[index] == 'v' )
            {
                if( buffer[index+1] == ' ' )
                    VertexPositions[vertPosCount++] = ParseVertex( buffer, index );
                else if( buffer[index+1] == 't' )
                    VertexUVs[vertUVCount++] = ParseVertex( buffer, index ).XY();
                else if( buffer[index+1] == 'n' )
                    VertexNormals[vertNormalCount++] = ParseVertex( buffer, index );

                if( VertexPositions[vertPosCount].x < minVert.x || vertPosCount == 0 ) minVert.x = VertexPositions[vertPosCount].x;
                if( VertexPositions[vertPosCount].y < minVert.y || vertPosCount == 0 ) minVert.y = VertexPositions[vertPosCount].y;
                if( VertexPositions[vertPosCount].z < minVert.z || vertPosCount == 0 ) minVert.z = VertexPositions[vertPosCount].z;
                if( VertexPositions[vertPosCount].x > maxVert.x || vertPosCount == 0 ) maxVert.x = VertexPositions[vertPosCount].x;
                if( VertexPositions[vertPosCount].y > maxVert.y || vertPosCount == 0 ) maxVert.y = VertexPositions[vertPosCount].y;
                if( VertexPositions[vertPosCount].z > maxVert.z || vertPosCount == 0 ) maxVert.z = VertexPositions[vertPosCount].z;
            }

            if( buffer[index] == 'f' )
            {
                faceCount += ParseFaceInfo( &Faces[faceCount], buffer, index );
            }

            index = FindIndexOfFirstNonWhitespaceOfNextLine( buffer, index );
        }
    }

    // Check the first face, to see how many attributes we need to allocate.
    int numComponents = 0;

    {
        for( int i=0; i<3; i++ )
        {
            if( Faces[0].attributes[0][i] != 0 )
            {
                if( i == 1 )
                    numComponents += 2; // UV.
                else
                    numComponents += 3; // Position or normal.
            }
        }

        //for( int i=0; i<4; i++ )
        //{
        //    if( Faces[0].attributes[i][0] != 0 ) // If it has a position, index 0 is an invalid vertex position.
        //        numvertsinaface++;
        //}
    }

    // We need a vert count, the number will be different if we remove dupes or not.
    int numVerts = 0;
    int numTriangles = 0;

    // Count the number of verts and triangles and copy the vertex indices into the face list.
    {
        double startTime = MyTime_GetSystemTime();

        numVerts = 0;
        numTriangles = 0;
        int dupesFound = 0;

        // Loop through all faces and vertices.
        for( int f=0; f<faceCount; f++ )
        {
            if( removeDuplicateVertices )
            {
                if( f % 10000 == 0 )
                {
                    double endtime = MyTime_GetSystemTime();
                    LOGInfo( LOGTag, "Looking for dupes (%0.0f) - faces %d, dupes found %d\n", endtime - startTime, f, dupesFound );
                }
            }

            int numVertsInFace = 3;
            numTriangles++;
            if( Faces[f].attributes[3][0] != 0 ) // If the 4th vertex has a valid position index, then it's a valid vert.
            {
                numVertsInFace = 4;
                numTriangles++;
            }

            for( int v=0; v<numVertsInFace; v++ )
            {
                // Try to reduce the number of vertices by searching for duplicate vertices in face list.
                //   Needs some serious optimizing. TODO: optimize this step.
                // Search all previous vertices looking for a duplicate.
                if( removeDuplicateVertices )
                {
                    for( int origF=0; origF < f; origF++ )
                    {
                        for( int origV=0; origV < numVertsInFace; origV++ )
                        {
                            if( Faces[f].attributes[v][0] == Faces[origF].attributes[origV][0] &&
                                Faces[f].attributes[v][1] == Faces[origF].attributes[origV][1] &&
                                Faces[f].attributes[v][2] == Faces[origF].attributes[origV][2] )
                            {
                                Faces[f].vertindex[v] = Faces[origF].vertindex[origV];
                                dupesFound++;
                                goto foundDuplicate_SkipToNextVert;
                            }
                        }
                    }
                }

                Faces[f].vertindex[v] = numVerts + 1;
                numVerts++;
foundDuplicate_SkipToNextVert:
                ;
            }
        }

        double endTime = MyTime_GetSystemTime();

        if( removeDuplicateVertices )
        {
            LOGInfo( LOGTag, "Looking for dupes (%0.0f) - faces %d, dupes found %d\n", endTime - startTime, faceCount, dupesFound );
        }
    }

    int vertBufferSize = numVerts * numComponents;
    int indexCount = numTriangles * 3;

    int bytesPerIndex = 1;
    if( numVerts > 256 && numVerts <= 256*256 )
        bytesPerIndex = 2;
    else if( numVerts > 256*256 )
        bytesPerIndex = 4;

    float* verts = MyNew float[vertBufferSize];
    unsigned char* indices = MyNew unsigned char[indexCount * bytesPerIndex];

    // Cycle through the faces and copy the vertex info into the final vertex/index buffers.
    {
        int indexByteCount = 0;
        int vertCount = 0;
        for( int f=0; f<faceCount; f++ )
        {
            int vertIndices[4];

            vertIndices[0] = Faces[f].vertindex[0] - 1;
            vertIndices[1] = Faces[f].vertindex[1] - 1;
            vertIndices[2] = Faces[f].vertindex[2] - 1;
            vertIndices[3] = Faces[f].vertindex[3] - 1;
            
            int numVertsInFace = 3;
            if( Faces[f].attributes[3][0] != 0 ) // if the 4th vertex has a valid position index, then it's a valid vert.
                numVertsInFace = 4;

            for( int v=0; v<numVertsInFace; v++ )
            {
                int attrOffset = vertIndices[v] * numComponents;

                int vertPosIndex = Faces[f].attributes[v][0] - 1;
                verts[attrOffset] = VertexPositions[vertPosIndex].x * scale; attrOffset++;
                verts[attrOffset] = VertexPositions[vertPosIndex].y * scale; attrOffset++;
                verts[attrOffset] = VertexPositions[vertPosIndex].z * scale; attrOffset++;

                int vertUVIndex = Faces[f].attributes[v][1] - 1;
                if( vertUVIndex != -1 )
                {
                    verts[attrOffset] = VertexUVs[vertUVIndex].x; attrOffset++;
                    verts[attrOffset] = VertexUVs[vertUVIndex].y; attrOffset++;
                }

                int vertNormalIndex = Faces[f].attributes[v][2] - 1;
                if( vertNormalIndex != -1 )
                {
                    verts[attrOffset] = VertexNormals[vertNormalIndex].x; attrOffset++;
                    verts[attrOffset] = VertexNormals[vertNormalIndex].y; attrOffset++;
                    verts[attrOffset] = VertexNormals[vertNormalIndex].z; attrOffset++;
                }
            }

            if( numVertsInFace == 3 )
            {
                SetValueOfIndex( indices, indexByteCount, vertIndices[0], bytesPerIndex ); indexByteCount += bytesPerIndex;
                SetValueOfIndex( indices, indexByteCount, vertIndices[1], bytesPerIndex ); indexByteCount += bytesPerIndex;
                SetValueOfIndex( indices, indexByteCount, vertIndices[2], bytesPerIndex ); indexByteCount += bytesPerIndex;
            }
            else if( numVertsInFace == 4 )
            {
                MyAssert( false ); // Shouldn't reach this code anymore, faces are broken up into triangles in ParseFaceInfo().
                SetValueOfIndex( indices, indexByteCount, vertIndices[0], bytesPerIndex ); indexByteCount += bytesPerIndex;
                SetValueOfIndex( indices, indexByteCount, vertIndices[1], bytesPerIndex ); indexByteCount += bytesPerIndex;
                SetValueOfIndex( indices, indexByteCount, vertIndices[2], bytesPerIndex ); indexByteCount += bytesPerIndex;

                SetValueOfIndex( indices, indexByteCount, vertIndices[0], bytesPerIndex ); indexByteCount += bytesPerIndex;
                SetValueOfIndex( indices, indexByteCount, vertIndices[2], bytesPerIndex ); indexByteCount += bytesPerIndex;
                SetValueOfIndex( indices, indexByteCount, vertIndices[3], bytesPerIndex ); indexByteCount += bytesPerIndex;
            }
            else
            {
                MyAssert( false );
            }

            vertCount += numVertsInFace;
        }
    }

    delete[] VertexPositions;
    delete[] VertexUVs;
    delete[] VertexNormals;
    delete[] Faces;

    VertexFormats format;

    if( VertexUVs && VertexNormals )
        format = VertexFormat_XYZUVNorm;
    else if( VertexUVs )
        format = VertexFormat_XYZUV;
    else if( VertexNormals )
        format = VertexFormat_XYZNorm;
    else
        format = VertexFormat_XYZ;

    // Give verts and indices pointers to BufferDefinition objects, which will handle the delete[]'s.
    if( *ppVBO == nullptr )
    {
        *ppVBO = g_pBufferManager->CreateBuffer();
    }

    if( *ppIBO == nullptr )
    {
        *ppIBO = g_pBufferManager->CreateBuffer();
    }

    if( pAABB )
    {
        Vector3 center = (minVert + maxVert) / 2;
        pAABB->Set( center, maxVert - center );
    }

    // The buffer will delete the allocated arrays of verts/indices.
    (*ppVBO)->InitializeBuffer( verts, vertBufferSize*4, MyRE::BufferType_Vertex, MyRE::BufferUsage_StaticDraw, true, 1, format, nullptr, "OBJLoader", "VBO" );
    (*ppIBO)->InitializeBuffer( indices, indexCount*bytesPerIndex, MyRE::BufferType_Index, MyRE::BufferUsage_DynamicDraw, true, 1, bytesPerIndex, "OBJLoader", "IBO" );
}
