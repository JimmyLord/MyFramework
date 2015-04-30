//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "OBJLoader.h"

// This code is assuming a list of faces longer than 3 is a triangle fan and turning into a list of tris.
// TODO: obj's can have concave face lists... so this is a step better, but still broken.

struct FaceInfo
{
    int attributes[4][3];

    int vertindex[4];
};

#if _DEBUG
char* LoadFile(const char* filename, long* length)
{
    char* filecontents = 0;

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
        filecontents[size] = 0;

        if( length )
            *length = size;

        fclose( filehandle );
    }

    return filecontents;
}
#endif

int FindIndexOfNextSpaceOrTab(char* buffer, int index)
{
    // find first non-whitespace
    while( buffer[index] != '\t' && buffer[index] != ' ' )
    {
        index++;
    }

    return index;
}

int FindIndexOfFirstNonWhitespace(char* buffer, int index)
{
    // find first non-whitespace
    while( buffer[index] == '\r' || buffer[index] == '\n' || buffer[index] == '\t' || buffer[index] == ' ' )
    {
        index++;
    }

    return index;
}

int FindIndexOfFirstNonSpaceOrTab(char* buffer, int index)
{
    // find first non-whitespace
    while( buffer[index] == '\t' || buffer[index] == ' ' )
    {
        index++;
    }

    return index;
}

int FindIndexOfFirstNonWhitespaceOfNextLine(char* buffer, int index)
{
    // find end of line
    while( buffer[index] != '\n' && buffer[index] != 0 )
    {
        index++;
    }

    return FindIndexOfFirstNonWhitespace( buffer, index );
}

float ReadFloatAndMoveOn(char* buffer, int* index)
{
    char numberbuffer[100];
    int numberbufferindex = 0;

    while( (buffer[*index] >= '0' && buffer[*index] <= '9') || buffer[*index] == '.' || buffer[*index] == '-' || buffer[*index] == 'e' || buffer[*index] == '+' )
    {
        numberbuffer[numberbufferindex] = buffer[*index];
        numberbuffer[numberbufferindex+1] = 0;
        numberbufferindex++;

        (*index)++;
    }

    return (float)atof( numberbuffer );
}

int ReadIntAndMoveOn(char* buffer, int* index)
{
    char numberbuffer[100];
    int numberbufferindex = 0;

    while( (buffer[*index] >= '0' && buffer[*index] <= '9') || buffer[*index] == '-' )
    {
        numberbuffer[numberbufferindex] = buffer[*index];
        numberbuffer[numberbufferindex+1] = 0;
        numberbufferindex++;

        (*index)++;
    }

    return atoi( numberbuffer );
}

Vector3 ParseVertex(char* buffer, int index)
{
    assert( buffer[index] == 'v' );

    Vector3 outvector(0,0,0);

    // jump to the first space after the v, vt, or vn
    index = FindIndexOfNextSpaceOrTab( buffer, index );
    index = FindIndexOfFirstNonWhitespace( buffer, index+1 );

    int count = 0;

    while( buffer[index] != '\r' && buffer[index] != '\n' && buffer[index] != 0 )
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
                assert( value == 1 ); // seen "v 3.25000 -2.48000 14.0000 1.00000" in a file, not sure why 1 is there.
            else
                outvector[count] = value;
        }
    }

    //if( outvector.LengthSquared() == 0 )
    //    int bp = 1;

    return outvector;
}

int ParseFaceInfo(FaceInfo* faces, char* buffer, int index)
{
    // divide the face into a bunch of triangles... assumes face is defined as triangle fan, which it isn't.

    int trianglecount = 0;

    assert( buffer[index] == 'f' );

    // jump to the first number
    index = FindIndexOfFirstNonWhitespace( buffer, index+1 );

    FaceInfo faceinfo;
    memset( &faceinfo, 0, sizeof(FaceInfo) );

    int numverts = 0;
    int numattrs = 0;

    int lastnumber = -1;

    while( 1 )
    {
        if( buffer[index] == ' ' || buffer[index] == '\t' || buffer[index] == '\r' || buffer[index] == '\n' || buffer[index] == 0 )
        {
            assert( numverts < 3 );

            if( lastnumber > 0 )
            {
                faceinfo.attributes[numverts][numattrs] = lastnumber;
                numverts++;

                if( numverts == 3 ) // if we filled a triangle, create a new one.
                {
                    faces[trianglecount] = faceinfo;
                    trianglecount++;

                    // assuming a triangle fan, so copy vert2 into vert1, vert0 stays the same, then read a new vert2
                    for( int i=0; i<3; i++ )
                        faceinfo.attributes[1][i] = faceinfo.attributes[2][i];
                    numverts--;
                }
    
                index = FindIndexOfFirstNonSpaceOrTab( buffer, index );
            }

            lastnumber = -1; // invalid index
            numattrs = 0;

            if( buffer[index] == '\r' || buffer[index] == '\n' || buffer[index] == 0 )
                break;
        }
        else if( buffer[index] == '/' )
        {
            assert( numverts < 3 );
            assert( numattrs < 3 );

            faceinfo.attributes[numverts][numattrs] = lastnumber;
            lastnumber = 0; // invalid index

            numattrs++;
            index++;
        }
        else
        {
            lastnumber = ReadIntAndMoveOn( buffer, &index );
        }
    }

    return trianglecount;
}

#if _DEBUG
void LoadBasicOBJFromFile(char* filename, BufferDefinition** ppVBO, BufferDefinition** ppIBO, bool removeduplicatevertices, float scale)
{
    assert( ppVBO );
    assert( ppIBO );

    assert( false ); // don't use this function, file i/o should be done through a RequestFile call.

    long size;
    char* buffer = LoadFile( filename, &size );

    LoadBasicOBJ( buffer, ppVBO, ppIBO, removeduplicatevertices, scale );

    delete[] buffer;
}
#endif

void SetValueOfIndex(unsigned char* indices, int index, unsigned int value, int indexbytes)
{
    if( indexbytes == 1 )
    {
        *(unsigned char*)&indices[index] = (unsigned char)value;
    }
    if( indexbytes == 2 )
    {
        *(unsigned short*)&indices[index] = (unsigned short)value;
    }
    if( indexbytes == 4 )
    {
        *(unsigned int*)&indices[index] = (unsigned int)value;
    }
}

void LoadBasicOBJ(char* buffer, BufferDefinition** ppVBO, BufferDefinition** ppIBO, bool removeduplicatevertices, float scale)
{
    assert( ppVBO );
    assert( ppIBO );

    // count the number of faces and each type of vertex attribute.
    int facecount = 0;
    int vertposcount = 0;
    int vertuvcount = 0;
    int vertnormalcount = 0;

    {
        int index = FindIndexOfFirstNonWhitespace( buffer, 0 );
        while( buffer[index] != 0 )
        {
            if( buffer[index] == 'v' )
            {
                if( buffer[index+1] == ' ' )        vertposcount++;
                else if( buffer[index+1] == 't' )   vertuvcount++;
                else if( buffer[index+1] == 'n' )   vertnormalcount++;
            }
            else if( buffer[index] == 'f' )
            {
                // count the number of verices, I'll assume the list produces a convex shape.
                // so, numfaces is 2 less than the number of verts.
                int numvertsinface = -1;
                while( buffer[index] != '\r' && buffer[index] != '\n' )
                {
                    // skip all non-whitespace.
                    while( buffer[index] != '\t' && buffer[index] != ' ' && buffer[index] != '\r' && buffer[index] != '\n' )
                        index++;

                    numvertsinface++;

                    // skip all whitespace.
                    while( buffer[index] == '\t' || buffer[index] == ' ' )
                        index++;
                }

                facecount += numvertsinface-2;
            }

            index = FindIndexOfFirstNonWhitespaceOfNextLine( buffer, index );
        }
    }

    // allocate individual buffers for pos/uv/norm, then parse again to store values
    Vector3* VertexPositions = MyNew Vector3[vertposcount];
    Vector2* VertexUVs = vertuvcount ? MyNew Vector2[vertuvcount] : 0;
    Vector3* VertexNormals = vertnormalcount ? MyNew Vector3[vertnormalcount] : 0;
    FaceInfo* Faces = MyNew FaceInfo[facecount];

    {
        int index = FindIndexOfFirstNonWhitespace( buffer, 0 );
        facecount = 0;
        vertposcount = 0;
        vertuvcount = 0;
        vertnormalcount = 0;

        while( buffer[index] != 0 )
        {
            if( buffer[index] == 'v' )
            {
                if( buffer[index+1] == ' ' )
                    VertexPositions[vertposcount++] = ParseVertex( buffer, index );
                else if( buffer[index+1] == 't' )
                    VertexUVs[vertuvcount++] = ParseVertex( buffer, index ).XY();
                else if( buffer[index+1] == 'n' )
                    VertexNormals[vertnormalcount++] = ParseVertex( buffer, index );
            }

            if( buffer[index] == 'f' )
            {
                facecount += ParseFaceInfo( &Faces[facecount], buffer, index );
            }

            index = FindIndexOfFirstNonWhitespaceOfNextLine( buffer, index );
        }
    }

    // check the first face, to see how many attributes we need to allocate.
    int numcomponents = 0;

    {
        for( int i=0; i<3; i++ )
        {
            if( Faces[0].attributes[0][i] != 0 )
            {
                if( i == 1 )
                    numcomponents += 2; // uv
                else
                    numcomponents += 3; // position or normal
            }
        }

        //for( int i=0; i<4; i++ )
        //{
        //    if( Faces[0].attributes[i][0] != 0 ) // if it has a position, index 0 is an invalid vertex position
        //        numvertsinaface++;
        //}
    }

    // we need a vert count, the number will be different if we remove dupes or not.
    int numverts = 0;
    int numtriangles = 0;

    // count the number of verts and triangles and copy the vertex indices into the face list.
    {
        double starttime = MyTime_GetSystemTime();

        numverts = 0;
        numtriangles = 0;
        int dupesfound = 0;

        // loop through all faces and vertices
        for( int f=0; f<facecount; f++ )
        {
            if( removeduplicatevertices )
            {
                if( f % 10000 == 0 )
                {
                    double endtime = MyTime_GetSystemTime();
                    LOGInfo( LOGTag, "Looking for dupes (%0.0f) - faces %d, dupes found %d\n", endtime - starttime, f, dupesfound );
                }
            }

            int numvertsinface = 3;
            numtriangles++;
            if( Faces[f].attributes[3][0] != 0 ) // if the 4th vertex has a valid position index, then it's a valid vert
            {
                numvertsinface = 4;
                numtriangles++;
            }

            for( int v=0; v<numvertsinface; v++ )
            {
                // try to reduce the number of vertices by searching for duplicate vertices in face list.
                //   needs some serious optimizing. TODO: optimize this step.
                // search all previous vertices looking for a duplicate.
                if( removeduplicatevertices )
                {
                    for( int origf=0; origf < f; origf++ )
                    {
                        for( int origv=0; origv < numvertsinface; origv++ )
                        {
                            if( Faces[f].attributes[v][0] == Faces[origf].attributes[origv][0] &&
                                Faces[f].attributes[v][1] == Faces[origf].attributes[origv][1] &&
                                Faces[f].attributes[v][2] == Faces[origf].attributes[origv][2] )
                            {
                                Faces[f].vertindex[v] = Faces[origf].vertindex[origv];
                                dupesfound++;
                                goto foundduplicate_skiptonextvert;
                            }
                        }
                    }
                }

                Faces[f].vertindex[v] = numverts + 1;
                numverts++;
foundduplicate_skiptonextvert:
                ;
            }
        }

        double endtime = MyTime_GetSystemTime();

        if( removeduplicatevertices )
        {
            LOGInfo( LOGTag, "Looking for dupes (%0.0f) - faces %d, dupes found %d\n", endtime - starttime, facecount, dupesfound );
        }
    }

    int vertbuffersize = numverts * numcomponents;
    int indexcount = numtriangles * 3;

    int bytesperindex = 1;
    if( numverts > 256 && numverts <= 256*256 )
        bytesperindex = 2;
    else if( numverts > 256*256 )
        bytesperindex = 4;

    float* verts = MyNew float[vertbuffersize];
    unsigned char* indices = MyNew unsigned char[indexcount * bytesperindex];

    // cycle through the faces and copy the vertex info into the final vertex/index buffers.
    {
        int indexbytecount = 0;
        int vertcount = 0;
        for( int f=0; f<facecount; f++ )
        {
            int vertindices[4];

            vertindices[0] = Faces[f].vertindex[0] - 1;
            vertindices[1] = Faces[f].vertindex[1] - 1;
            vertindices[2] = Faces[f].vertindex[2] - 1;
            vertindices[3] = Faces[f].vertindex[3] - 1;

            int numvertsinface = 3;
            if( Faces[f].attributes[3][0] != 0 ) // if the 4th vertex has a valid position index, then it's a valid vert
                numvertsinface = 4;

            for( int v=0; v<numvertsinface; v++ )
            {
                int attroffset = vertindices[v] * numcomponents;

                int vertposindex = Faces[f].attributes[v][0] - 1;
                verts[attroffset] = VertexPositions[vertposindex].x * scale; attroffset++;
                verts[attroffset] = VertexPositions[vertposindex].y * scale; attroffset++;
                verts[attroffset] = VertexPositions[vertposindex].z * scale; attroffset++;

                int vertuvindex = Faces[f].attributes[v][1] - 1;
                if( vertuvindex != -1 )
                {
                    verts[attroffset] = VertexUVs[vertuvindex].x; attroffset++;
                    verts[attroffset] = VertexUVs[vertuvindex].y; attroffset++;
                }

                int vertnormalindex = Faces[f].attributes[v][2] - 1;
                if( vertnormalindex != -1 )
                {
                    verts[attroffset] = VertexNormals[vertnormalindex].x; attroffset++;
                    verts[attroffset] = VertexNormals[vertnormalindex].y; attroffset++;
                    verts[attroffset] = VertexNormals[vertnormalindex].z; attroffset++;
                }
            }

            if( numvertsinface == 3 )
            {
                SetValueOfIndex( indices, indexbytecount, vertindices[0], bytesperindex ); indexbytecount += bytesperindex;
                SetValueOfIndex( indices, indexbytecount, vertindices[1], bytesperindex ); indexbytecount += bytesperindex;
                SetValueOfIndex( indices, indexbytecount, vertindices[2], bytesperindex ); indexbytecount += bytesperindex;
            }
            else if( numvertsinface == 4 )
            {
                assert( false ); // should reach this code anymore, faces are broken up into triangles in ParseFaceInfo();
                SetValueOfIndex( indices, indexbytecount, vertindices[0], bytesperindex ); indexbytecount += bytesperindex;
                SetValueOfIndex( indices, indexbytecount, vertindices[1], bytesperindex ); indexbytecount += bytesperindex;
                SetValueOfIndex( indices, indexbytecount, vertindices[2], bytesperindex ); indexbytecount += bytesperindex;

                SetValueOfIndex( indices, indexbytecount, vertindices[0], bytesperindex ); indexbytecount += bytesperindex;
                SetValueOfIndex( indices, indexbytecount, vertindices[2], bytesperindex ); indexbytecount += bytesperindex;
                SetValueOfIndex( indices, indexbytecount, vertindices[3], bytesperindex ); indexbytecount += bytesperindex;
            }
            else
            {
                assert( false );
            }

            vertcount += numvertsinface;
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
    (*ppVBO)->InitializeBuffer( verts, vertbuffersize*4, GL_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, format, 0, "OBJLoader", "VBO" );
    (*ppIBO)->InitializeBuffer( indices, indexcount*bytesperindex, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, true, 1, bytesperindex, "OBJLoader", "IBO" );

    //delete[] verts;
    //delete[] indices;
}
