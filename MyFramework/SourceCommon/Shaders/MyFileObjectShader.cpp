//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

MyFileObjectShader::MyFileObjectShader()
{
    m_ScannedForIncludes = false;
    m_NumIncludes = 0;
}

MyFileObjectShader::~MyFileObjectShader()
{
    for( int i=0; i<m_NumIncludes; i++ )
    {
        assert( m_pIncludes[i].m_pIncludedFile != 0 );

        g_pFileManager->FreeFile( m_pIncludes[i].m_pIncludedFile );
    }
}

void MyFileObjectShader::CheckFileForIncludesAndAddToList()
{
    if( m_ScannedForIncludes == true )
        return;

    m_ScannedForIncludes = true;

    assert( m_FileLoadStatus == FileLoadStatus_Success );

    for( unsigned int i=0; i<m_FileLength; i++ )
    {
        if( (i == 0 || m_pBuffer[i-1] != '/') &&
            m_pBuffer[i] == '#' && strncmp( &m_pBuffer[i], "#include", 8 ) == 0 )
        {
#pragma warning (disable : 4996)
            char includefilename[MAX_PATH];
            strcpy( includefilename, m_FullPath );
            int pathlen = strlen(m_FullPath) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot);
            includefilename[pathlen] = 0;
            int bufferleft = MAX_PATH-1 - pathlen;
            int charsread;
            int result = sscanf( &m_pBuffer[i], "#include \"%[^\"]\"%n", &includefilename[pathlen], &charsread );
#pragma warning (default : 4996)

            if( result == 1 )
            {
                MyFileObject* pIncludeFile = ::RequestFile( includefilename );
                if( dynamic_cast<MyFileObjectShader*>( pIncludeFile ) == 0 )
                {
                    LOGError( LOGTag, "MyFileObjectShader: Including a non-shader file\n" );
                    g_pFileManager->FreeFile( pIncludeFile );
                    return;
                }

                m_pIncludes[m_NumIncludes].m_pIncludedFile = (MyFileObjectShader*)pIncludeFile;
                m_pIncludes[m_NumIncludes].m_Include_StartIndex = i;
                m_pIncludes[m_NumIncludes].m_Include_EndIndex = i + charsread;
                m_NumIncludes++;
            }
        }
    }
}

bool MyFileObjectShader::AreAllIncludesLoaded()
{
    for( int i=0; i<m_NumIncludes; i++ )
    {
        assert( m_pIncludes[i].m_pIncludedFile != 0 );
        
        if( m_pIncludes[i].m_pIncludedFile->m_FileLoadStatus != FileLoadStatus_Success )
            return false;
    }

    return true;
}

int MyFileObjectShader::GetShaderChunkCount()
{
    int numchunks = 1;

    for( int i=0; i<m_NumIncludes; i++ )
    {
        numchunks += m_pIncludes[i].m_pIncludedFile->GetShaderChunkCount();
        numchunks += 1; // for every include file we have a pre-include and post-include chunk.
    }

    return numchunks;
}

int MyFileObjectShader::GetShaderChunks(const char** pStrings, int* pLengths)
{
    pStrings[0] = m_pBuffer;

    unsigned int lengthtoendoffile = m_FileLength;
    unsigned int currentoffset = 0;

    int count = 0;

    for( int i=0; i<m_NumIncludes; i++ )
    {
        pLengths[count] = m_pIncludes[i].m_Include_StartIndex - currentoffset;
        currentoffset = m_pIncludes[i].m_Include_EndIndex;

        count += m_pIncludes[i].m_pIncludedFile->GetShaderChunks( &pStrings[count+1], &pLengths[count+1] );

        count++;

        pStrings[count] = &m_pBuffer[currentoffset];
    }

    pLengths[count] = m_FileLength-1 - currentoffset;

    return count+1;
}
