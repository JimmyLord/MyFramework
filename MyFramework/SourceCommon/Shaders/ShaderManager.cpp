//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"

ShaderManager* g_pShaderManager = 0;

const char* g_pBrokenShaderString =
"                                                                                       \n\
#ifndef WIN32                                                                           \n\
precision mediump float;                                                                \n\
#endif                                                                                  \n\
                                                                                        \n\
uniform float u_Time;                                                                   \n\
                                                                                        \n\
#ifdef VertexShader                                                                     \n\
    attribute vec4 a_Position;                                                          \n\
    uniform mat4 u_WorldViewProj;                                                       \n\
    void main()                                                                         \n\
    {                                                                                   \n\
        gl_Position = u_WorldViewProj * a_Position;                                     \n\
    }                                                                                   \n\
#endif                                                                                  \n\
                                                                                        \n\
#ifdef FragmentShader                                                                   \n\
    void main()                                                                         \n\
    {                                                                                   \n\
    	vec2 offset = floor( (gl_FragCoord.xy + u_Time*5) / vec2(10, 10) );             \n\
        gl_FragColor = vec4( mod( offset.x + offset.y, 2.0 ) * vec3( 1, 0.5, 0 ), 1 );  \n\
    }                                                                                   \n\
#endif                                                                                  \n\
";

BaseShader::BaseShader()
{
    Init_BaseShader();
    Init( ShaderPass_Main );
}

BaseShader::BaseShader(ShaderPassTypes type)
{
    Init_BaseShader();
    Init( type );
}

void BaseShader::Init_BaseShader()
{
    ClassnameSanityCheck();

    m_Initialized = false;
    m_ShaderFailedToCompile = false;

    m_PassType = ShaderPass_NumTypes;
    m_BlendType = MaterialBlendType_Off;
    m_BlendFactorSrc = MaterialBlendFactor_SrcAlpha;
    m_BlendFactorDest = MaterialBlendFactor_OneMinusSrcAlpha;

    m_pFilename = 0;
    m_pFile = 0;
    m_pFilePixelShader = 0;

    m_pVSPredefinitions = 0;
    m_pGSPredefinitions = 0;
    m_pFSPredefinitions = 0;

    m_ProgramHandle = 0;
    m_VertexShaderHandle = 0;
    m_GeometryShaderHandle = 0;
    m_FragmentShaderHandle = 0;

    g_pShaderManager->AddShader( this );
}

BaseShader::~BaseShader()
{
    this->Remove();
    
    SAFE_RELEASE( m_pFile );
    SAFE_RELEASE( m_pFilePixelShader );

    SAFE_DELETE_ARRAY( m_pVSPredefinitions );
    SAFE_DELETE_ARRAY( m_pGSPredefinitions );
    SAFE_DELETE_ARRAY( m_pFSPredefinitions );

    Invalidate( true );
}

void BaseShader::Init(ShaderPassTypes type)
{
    m_PassType = type;
}

void BaseShader::OverridePredefs(const char* VSpredef, const char* GSpredef, const char* FSpredef, bool alsousedefaults)
{
    if( VSpredef )
    {
        int vslen = (int)strlen( VSpredef );
        if( alsousedefaults )
            vslen += (int)strlen( VERTEXPREDEFINES );
        vslen += 1;
        char* newvsstr = MyNew char[vslen];
        strcpy_s( newvsstr, vslen, VSpredef );
        if( alsousedefaults )
            strcat_s( newvsstr, vslen, VERTEXPREDEFINES );
        m_pVSPredefinitions = newvsstr;
    }

    if( GSpredef )
    {
        int gslen = (int)strlen( GSpredef );
        if( alsousedefaults )
            gslen += (int)strlen( GEOMETRYPREDEFINES );
        gslen += 1;
        char* newgsstr = MyNew char[gslen];
        strcpy_s( newgsstr, gslen, GSpredef );
        if( alsousedefaults )
            strcat_s( newgsstr, gslen, GEOMETRYPREDEFINES );
        m_pGSPredefinitions = newgsstr;
    }

    if( FSpredef )
    {
        int fslen = (int)strlen( FSpredef );
        if( alsousedefaults )
            fslen += (int)strlen( FRAGMENTPREDEFINES );
        fslen += 1;
        char* newfsstr = MyNew char[fslen];
        strcpy_s( newfsstr, fslen, FSpredef );
        if( alsousedefaults )
            strcat_s( newfsstr, fslen, FRAGMENTPREDEFINES );
        m_pFSPredefinitions = newfsstr;
    }
}

void BaseShader::Invalidate(bool cleanglallocs)
{
    if( cleanglallocs )
    {
        CleanGLAllocations();
    }
    m_Initialized = false;

    m_ProgramHandle = 0;
    m_VertexShaderHandle = 0;
    m_GeometryShaderHandle = 0;
    m_FragmentShaderHandle = 0;

    m_ShaderFailedToCompile = false;
}

void BaseShader::CleanGLAllocations()
{
    if( m_Initialized )
    {
        m_Initialized = false;

        checkGlError( "start of BaseShader::CleanGLAllocations" );

        if( m_VertexShaderHandle )
            glDetachShader( m_ProgramHandle, m_VertexShaderHandle );
        if( m_GeometryShaderHandle )
            glDetachShader( m_ProgramHandle, m_GeometryShaderHandle );
        if( m_FragmentShaderHandle )
            glDetachShader( m_ProgramHandle, m_FragmentShaderHandle );    

        checkGlError( "BaseShader::CleanGLAllocations" );

        glDeleteShader( m_VertexShaderHandle );
        glDeleteShader( m_GeometryShaderHandle );
        glDeleteShader( m_FragmentShaderHandle );

        checkGlError( "BaseShader::CleanGLAllocations" );

        glDeleteProgram( m_ProgramHandle );

        checkGlError( "end of BaseShader::CleanGLAllocations" );

        m_ProgramHandle = 0;
        m_VertexShaderHandle = 0;
        m_GeometryShaderHandle = 0;
        m_FragmentShaderHandle = 0;
    }

    m_ShaderFailedToCompile = false;
}

void BaseShader::LoadFromFile(const char* filename)
{
    MyAssert( filename != 0 );
    if( filename == 0 )
        return;

    m_pFilename = filename;
    LoadFromFile();
}

void BaseShader::LoadFromFile()
{
    MyAssert( m_pFile == 0 );
#if MYFW_WP8
    char tempfilename[MAX_PATH];
    sprintf_s( tempfilename, MAX_PATH, "%s.vertex.cso", m_pFilename );
    m_pFile = RequestFile( tempfilename );
    sprintf_s( tempfilename, MAX_PATH, "%s.pixel.cso", m_pFilename );
    m_pFilePixelShader = RequestFile( tempfilename );
#else
    char tempfilename[MAX_PATH];
    sprintf_s( tempfilename, MAX_PATH, "%s.glsl", m_pFilename );
    MyFileObject* pFile = RequestFile( tempfilename );
    if( pFile->IsA( "MyFileShader" ) )
    {
        m_pFile = (MyFileObjectShader*)pFile;
    }
    else
    {
        MyAssert( false );
        g_pFileManager->FreeFile( pFile );
    }
#endif
}

void ParseBlendFactor(const char* buffer, MaterialBlendFactors* pBlendFactorOut)
{
    const char* endOfBlendFactor = strpbrk( buffer, " \t\n\r" );
    char blendFactor[32];
    if( endOfBlendFactor - buffer > 31 )
        strncpy_s( blendFactor, buffer, 31 );
    else
        strncpy_s( blendFactor, buffer, endOfBlendFactor - buffer );

    if( _stricmp( blendFactor, "One" ) == 0 )
    {
        *pBlendFactorOut = MaterialBlendFactor_One;
        return;
    }

    // Blend factor not found, output an error.
    LOGError( LOGTag, "BlendMode not supported: %s\n", blendFactor );
}

bool BaseShader::LoadAndCompile(GLuint premadeprogramhandle)
{
    MyAssert( m_pFilePixelShader == 0 ); // TODO: see below, need to fix support for sep. vert/frag files.

    // If we already failed to compile, don't try again.
    if( m_ShaderFailedToCompile )
    {
        //LOGError( LOGTag, "BaseShader::LoadAndCompile - m_ShaderFailedToCompile\n" );
        return false;
    }

    if( m_pFile == 0 )
    {
        if( m_pFilename )
        {
            LoadFromFile();
            return false;
        }
    }

    // If there's a file, then try to compile, otherwise use the default "broken" shader.
    if( m_pFile )
    {
        // If the file isn't loaded, come back next frame and check again.
        if( m_pFile->GetFileLoadStatus() != FileLoadStatus_Success ||
            (m_pFilePixelShader != 0 && m_pFilePixelShader->GetFileLoadStatus() != FileLoadStatus_Success) )
        {
            return false;
        }

        // If there was an error loading the file.
        if( m_pFile->GetFileLoadStatus() > FileLoadStatus_Success )
        {
            LOGInfo( LOGTag, "Shader failed to load - %s\n", m_pFile->GetFullPath() );
            return false;
        }

        // Scan file for #includes and add them to the list
        if( m_pFile->m_ScannedForIncludes == false )
        {
            m_pFile->CheckFileForIncludesAndAddToList();
        }

        // Scan file for 'exposed' uniforms
        if( m_pFile->m_ScannedForExposedUniforms == false )
        {
            m_pFile->ParseAndCleanupExposedUniforms();
        }

        bool creategeometryshader = false;

        const char* buffer = m_pFile->GetBuffer();
        for( unsigned int i=0; i<m_pFile->GetFileLength(); i++ )
        {
            // TODO: Actually parse shader files properly, looking for some setting like these.
            if( (i == 0 || buffer[i-1] != '/') &&
                buffer[i] == '#' )
            {
                char blendstr[] = "#define BLENDING On";
                if( i + strlen(blendstr) < m_pFile->GetFileLength() &&
                    strncmp( &buffer[i], blendstr, strlen( blendstr ) ) == 0 )
                {
                    m_BlendType = MaterialBlendType_On; 
                }

                char blendFuncStr[] = "#define BLENDFUNC";
                int blendFuncStrLen = strlen( blendFuncStr );
                if( i + blendFuncStrLen < m_pFile->GetFileLength() &&
                    strncmp( &buffer[i], blendFuncStr, blendFuncStrLen ) == 0 )
                {
                    i += blendFuncStrLen + 1;

                    const char* endOfLine = strchr( &buffer[i], '\n' );
                    if( endOfLine )
                    {
                        ParseBlendFactor( &buffer[i], &m_BlendFactorSrc );
                     
                        const char* startOfSecondBlendFactor = strchr( &buffer[i], ' ' );

                        if( startOfSecondBlendFactor < endOfLine )
                        {
                            ParseBlendFactor( startOfSecondBlendFactor + 1, &m_BlendFactorDest );
                        }
                    }
                }

                char geoshaderstr[] = "#define USING_GEOMETRY_SHADER 1";
                if( i + strlen(blendstr) < m_pFile->GetFileLength() &&
                    strncmp( &buffer[i], geoshaderstr, strlen( geoshaderstr ) ) == 0 )
                {
                    creategeometryshader = true; 
                }
            }
        }

        // Are all #includes loaded? if not drop out and check again next frame.
        bool loadcomplete = m_pFile->AreAllIncludesLoaded();
        if( loadcomplete == false )
            return false;

        {
            // Check how many shader chunks exist between this file and all others included.
            int numchunks = m_pFile->GetShaderChunkCount();

            // Allocate buffers for all chunks + 2 more for the shader pass defines and the prevertex/prefrag strings.
            numchunks += 2;
            const char** pStrings = MyNew const char*[numchunks];
            int* pLengths = MyNew int[numchunks];

            m_pFile->GetShaderChunks( &pStrings[2], &pLengths[2] );

            // Stick our shader pass defines in the first slot.
            pStrings[0] = g_ShaderPassDefines[m_PassType];
            pLengths[0] = (int)strlen( g_ShaderPassDefines[m_PassType] );

            // Predefines will be stuck into slot 1 by createProgram().
            //    It'll be different for vertex and fragment shader.
            const char* pVSPre = VERTEXPREDEFINES;
            if( m_pVSPredefinitions )
                pVSPre = m_pVSPredefinitions;
            int VSPreLen = (int)strlen( pVSPre );

            const char* pGSPre = GEOMETRYPREDEFINES;
            if( m_pGSPredefinitions )
                pGSPre = m_pGSPredefinitions;
            int GSPreLen = (int)strlen( pGSPre );

            const char* pFSPre = FRAGMENTPREDEFINES;
            if( m_pFSPredefinitions )
                pFSPre = m_pFSPredefinitions;
            int FSPreLen = (int)strlen( pFSPre );

            // Actually create and compile the shader objects.
            if( creategeometryshader )
            {
                m_ProgramHandle = createProgram( &m_VertexShaderHandle, &m_GeometryShaderHandle, &m_FragmentShaderHandle,
                                                 VSPreLen, pVSPre, GSPreLen, pGSPre, FSPreLen, pFSPre,
                                                 numchunks, pStrings, pLengths, premadeprogramhandle );
            }
            else
            {
                m_ProgramHandle = createProgram( &m_VertexShaderHandle, &m_FragmentShaderHandle,
                                                 VSPreLen, pVSPre, FSPreLen, pFSPre,
                                                 numchunks, pStrings, pLengths, premadeprogramhandle );
            }

            delete[] pStrings;
            delete[] pLengths;
        }

        // TODO: Fix support for separate vert/frag files (rather than glsl files).
    //    if( m_pFilePixelShader == 0 )
    //    {
    //#if MYFW_WP8
    //        m_ProgramHandle = createProgram( m_pFile->m_FileLength, m_pFile->m_pBuffer,
    //                                         m_pFile->m_FileLength, m_pFile->m_pBuffer,
    //                                         &m_VertexShaderHandle, &m_FragmentShaderHandle,
    //                                         VSPreLen, pVSPre, FSPreLen, pFSPre,
    //                                         0, premadeprogramhandle );
    //#else
    //        m_ProgramHandle = createProgram( m_pFile->m_FileLength, m_pFile->m_pBuffer,
    //                                         m_pFile->m_FileLength, m_pFile->m_pBuffer,
    //                                         &m_VertexShaderHandle, &m_FragmentShaderHandle,
    //                                         VSPreLen, pVSPre, FSPreLen, pFSPre,
    //                                         g_ShaderPassDefines[m_PassType], premadeprogramhandle );
    //#endif
    //    }
    //    else
    //    {
    //#if MYFW_WP8
    //        m_ProgramHandle = createProgram( m_pFile->m_FileLength, m_pFile->m_pBuffer,
    //                                         m_pFilePixelShader->m_FileLength, m_pFilePixelShader->m_pBuffer,
    //                                         &m_VertexShaderHandle, &m_FragmentShaderHandle,
    //                                         0, 0, 0, 0,
    //                                         0, premadeprogramhandle );
    //#else
    //        m_ProgramHandle = createProgram( m_pFile->m_FileLength, m_pFile->m_pBuffer,
    //                                         m_pFilePixelShader->m_FileLength, m_pFilePixelShader->m_pBuffer,
    //                                         &m_VertexShaderHandle, &m_FragmentShaderHandle,
    //                                         0, 0, 0, 0,
    //                                         g_ShaderPassDefines[m_PassType], premadeprogramhandle );
    //#endif
    //    }
    }

    if( m_ProgramHandle == 0 )
    {
        if( m_pFile )
        {
            LOGError( LOGTag, "Failed to create program from %s\n", m_pFile->GetFullPath() );
            LOGError( LOGTag, "\n" );
        }

        //LOGError( LOGTag, "Could not create program.\n");
        m_ShaderFailedToCompile = true;

        int brokenShaderStringLen = (int)strlen( g_pBrokenShaderString );

        // This is hideous code, createProgram() places the predefine strings in array entry [1] of these 2 arrays. 
        int numchunks = 3;

        const char* pStrings[3];
        int pLengths[3];

        pStrings[0] = g_ShaderPassDefines[m_PassType];
        pLengths[0] = (int)strlen( g_ShaderPassDefines[m_PassType] );

        pStrings[2] = g_pBrokenShaderString;
        pLengths[2] = brokenShaderStringLen;

        // Compiling failed, so fallback on a default "broken" shader effect.
        m_ProgramHandle = createProgram( &m_VertexShaderHandle, &m_FragmentShaderHandle,
                                         (int)strlen( VERTEXPREDEFINES ), VERTEXPREDEFINES, (int)strlen( FRAGMENTPREDEFINES ), FRAGMENTPREDEFINES,
                                         //1, &pBrokenShaderString, &brokenShaderStringLen, premadeprogramhandle );
                                         numchunks, pStrings, pLengths, premadeprogramhandle );

        //return false;
    }
    else
    {
        //LOGInfo( LOGTag, "Successfully created program from %s\n", m_pFile->GetFullPath() );
    }

    MyEvent* pEvent = g_pEventManager->CreateNewEvent( Event_ShaderFinishedLoading );
    g_pEventManager->SendEventNow( pEvent );

    return true;
}

void BaseShader::InitializeAttributeArray(GLint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    if( index != -1 )
    {
        MyEnableVertexAttribArray( index );
        glVertexAttribPointer( index, size, type, normalized, stride, pointer );
    }
    else
    {
        //LOGInfo( LOGTag, "Shader_Base - Failed to initialize vertex attribute array - %d\n", index );
    }
}

void BaseShader::InitializeAttributeIArray(GLint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
    MyAssert( false ); // no availabe with es 2.0, so avoid it for now.
    //if( index != -1 )
    //{
    //    MyEnableVertexAttribArray( index );
    //    glVertexAttribIPointer( index, size, type, stride, pointer );
    //}
    //else
    //{
    //    //LOGInfo( LOGTag, "Shader_Base - Failed to initialize vertex attribute array - %d\n", index );
    //}
}

void BaseShader::DisableAttributeArray(GLint index, Vector3 value)
{
    if( index != -1 )
    {
        MyDisableVertexAttribArray( index );

        // TODO: set this attribute override value in the MyMesh object.
        glVertexAttrib3fv( index, &value.x );
    }
}

void BaseShader::DisableAttributeArray(GLint index, Vector4 value)
{
    if( index != -1 )
    {
        MyDisableVertexAttribArray( index );

        // TODO: set this attribute override value in the MyMesh object.
        glVertexAttrib4fv( index, &value.x );
    }
}

void BaseShader::DeactivateShader(BufferDefinition* vbo, bool usevaosifavailable)
{
}

bool BaseShader::DoVAORequirementsMatch(BaseShader* pShader)
{
    return true;
}

void ShaderManager::AddShader(BaseShader* pShader)
{
    m_ShaderList.AddTail( pShader );
}

void ShaderManager::InvalidateAllShaders(bool cleanglallocs)
{
    MyUseProgram( 0 );

    for( CPPListNode* pNode = m_ShaderList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        BaseShader* pShader = (BaseShader*)pNode;
        pShader->Invalidate( cleanglallocs );
    }
}

void ShaderManager::InvalidateAllShadersUsingFile(MyFileObjectShader* pFileToFind)
{
    // Loop through all loaded shaders.
    for( CPPListNode* pNode = m_ShaderList.GetHead(); pNode; pNode = pNode->GetNext() )
    {
        BaseShader* pShader = (BaseShader*)pNode;

        MyFileObjectShader* pFile = pShader->m_pFile;

        if( pFile == pFileToFind )
        {
            pShader->Invalidate( true );
        }

        // Loop through all of their includes.
        for( unsigned int i=0; i<pFile->m_NumIncludes; i++ )
        {
            // If it matches ours, invalidate the shader.
            if( pFile->m_Includes[i].m_pIncludedFile == pFileToFind )
            {
                pShader->Invalidate( true );
            }
        }
    }
}
