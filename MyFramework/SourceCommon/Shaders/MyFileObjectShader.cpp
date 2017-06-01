//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

MyFileObjectShader::MyFileObjectShader()
{
    ClassnameSanityCheck();

    m_IsAnIncludeFile = false;

    m_ScannedForIncludes = false;
    m_NumIncludes = 0;
}

MyFileObjectShader::~MyFileObjectShader()
{
    ClearIncludedFiles();
}

void MyFileObjectShader::UnloadContents()
{
    MyFileObject::UnloadContents();

    if( m_IsAnIncludeFile )
    {
        g_pShaderManager->InvalidateAllShadersUsingIncludeFile( this );
    }

    ClearIncludedFiles();
}

void MyFileObjectShader::ClearIncludedFiles()
{
    for( int i=0; i<m_NumIncludes; i++ )
    {
        MyAssert( m_pIncludes[i].m_pIncludedFile != 0 );

        g_pFileManager->FreeFile( m_pIncludes[i].m_pIncludedFile );
    }

    m_ScannedForIncludes = false;
    m_NumIncludes = 0;
}

void MyFileObjectShader::CheckFileForIncludesAndAddToList()
{
    if( m_ScannedForIncludes == true )
        return;

    m_ScannedForIncludes = true;

    MyAssert( m_FileLoadStatus == FileLoadStatus_Success );

    for( unsigned int i=0; i<m_FileLength; i++ )
    {
        if( (i == 0 || m_pBuffer[i-1] != '/') &&
            m_pBuffer[i] == '#' && strncmp( &m_pBuffer[i], "#include", 8 ) == 0 )
        {
#pragma warning( push )
#pragma warning( disable : 4996 )
            char includefilename[MAX_PATH];
            includefilename[0] = 0;
            int charsread;
            int result = 0;
            if( m_pBuffer[i+9] == '<' )
            {
                strcpy( includefilename, "Data/DataEngine/Shaders/" );
                int pathlen = (int)( strlen(includefilename) );
                result = sscanf( &m_pBuffer[i], "#include <%[^>]>%n", &includefilename[pathlen], &charsread );
            }
            else
            {
                strcpy( includefilename, m_FullPath );
                int pathlen = (int)( strlen(m_FullPath) - strlen(m_FilenameWithoutExtension) - strlen(m_ExtensionWithDot) );
                includefilename[pathlen] = 0;
                result = sscanf( &m_pBuffer[i], "#include \"%[^\"]\"%n", &includefilename[pathlen], &charsread );
            }
#pragma warning( pop )

            if( result == 1 )
            {
                MyFileObject* pIncludeFile = g_pFileManager->RequestFile( includefilename );
                MyFileObjectShader* pShaderFile = (MyFileObjectShader*)pIncludeFile;
                if( pShaderFile->IsA( "MyFileShader" ) == false )
                {
                    LOGError( LOGTag, "MyFileObjectShader: Including a non-shader file\n" );
                    g_pFileManager->FreeFile( pIncludeFile );
                    continue;
                }

                pShaderFile->m_IsAnIncludeFile = true;

                MyAssert( m_NumIncludes < MAX_INCLUDES );
                m_pIncludes[m_NumIncludes].m_pIncludedFile = (MyFileObjectShader*)pIncludeFile;
                m_pIncludes[m_NumIncludes].m_Include_StartIndex = i;
                m_pIncludes[m_NumIncludes].m_Include_EndIndex = i + charsread;
                m_NumIncludes++;
            }
        }
        else if( m_pBuffer[i] == '#' && ( strncmp( &m_pBuffer[i], "#endif", 6 ) == 0 ||
                                          strncmp( &m_pBuffer[i], "#else", 5 ) == 0 ) )
        {
            // Some Android .glsl compilers are strict about characters appearing after an #endif
            // I like to put comments on some #endif's like so: "#endif //comment"
            // This code will insert a newline in the space after the #endif if there is one
            // or assert if the character immediately following the #endif isn't a space.

            int tokenlen = 6;
            if( strncmp( &m_pBuffer[i], "#else", 5 ) == 0 )
                tokenlen = 5;

            // if there's any code after an #endif or #else
            if( m_pBuffer[i+tokenlen] != '\r' && m_pBuffer[i+tokenlen] != '\n' )
            {
                if( m_pBuffer[i+tokenlen] == ' ' )
                {
                    // if the next character is a space, then put in a newline
                    m_pBuffer[i+tokenlen] = '\n';
                }
                else
                {
                    // Assert, since there may be an issue compiling the shader
                    // it could be fine, since there may be whitespace and this code doesn't check for all types.
                    LOGError( LOGTag, "Characters found after #endif or #else\n" );
                    MyAssert( false );
                }
            }
        }
    }
}

bool MyFileObjectShader::AreAllIncludesLoaded()
{
    for( int i=0; i<m_NumIncludes; i++ )
    {
        MyAssert( m_pIncludes[i].m_pIncludedFile != 0 );

        if( m_pIncludes[i].m_pIncludedFile->GetFileLoadStatus() != FileLoadStatus_Success )
        {
            return false;
        }
        else
        {
            // scan file for #includes and add them to the list
            if( m_pIncludes[i].m_pIncludedFile->m_ScannedForIncludes == false )
                m_pIncludes[i].m_pIncludedFile->CheckFileForIncludesAndAddToList();
        }
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
