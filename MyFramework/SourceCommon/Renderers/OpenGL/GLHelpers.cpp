//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"

#include "GLHelpers.h"
#include "../SourceWindows/OpenGL/GLExtensions.h"

void printGLString(const char* name, GLenum s)
{
#if _DEBUG && !MYFW_NACL && !USE_D3D
    const char* v = (const char*)glGetString(s);
    LOGInfo( LOGTag, "GL %s = %s\n", name, v );
#endif
}

bool checkGlErrorFunc(const char* op, const char* file, int line)
{
#if !MYFW_NACL && !USE_D3D
    int errorcount = 0;
    for( GLint error = glGetError(); error; error = glGetError() )
    {
        //LOGError( LOGTag, "GLError(error count %d) after \"%s\" glError (0x%x), file %s, line %d\n", errorcount+1, op, error, file, line );
        LOGError( LOGTag, "%s(%d):GLError(error count %d) after \"%s\" glError (0x%x)\n", file, line, errorcount+1, op, error );

        if( error == GL_INVALID_ENUM )
            LOGError( LOGTag, "GLError - GL_INVALID_ENUM\n" );
        else if( error == GL_INVALID_VALUE )
            LOGError( LOGTag, "GLError - GL_INVALID_VALUE\n" );
        else if( error == GL_INVALID_OPERATION )
            LOGError( LOGTag, "GLError - GL_INVALID_OPERATION\n" );
        else
            LOGError( LOGTag, "GLError - OTHER\n" );

        errorcount++;
        if( errorcount > 10 )
            break;
    }

    return errorcount > 0;
#else
    return false;
#endif
}

GLint GetAttributeLocation(GLuint programhandle, const char* name, ...)
{
#define MAX_MESSAGE 128
    char tempstring[MAX_MESSAGE];
    va_list arg;
    va_start( arg, name );
    vsnprintf_s( tempstring, sizeof(tempstring), _TRUNCATE, name, arg );
    va_end( arg );

    GLint handle = glGetAttribLocation( programhandle, tempstring );

    if( handle == -1 )
    {
        //LOGInfo( LOGTag, "Attribute not found: %s\n", name );
    }

    return handle;
}

GLint GetUniformLocation(GLuint programhandle, const char* name, ...)
{
#define MAX_MESSAGE 128
    char tempstring[MAX_MESSAGE];
    va_list arg;
    va_start( arg, name );
    vsnprintf_s( tempstring, sizeof(tempstring), _TRUNCATE, name, arg );
    va_end( arg );

    GLint handle = glGetUniformLocation( programhandle, tempstring );

    //if( handle == -1 )
    //    LOGInfo( LOGTag, "Uniform not found: %s\n", tempstring );

    return handle;
}

void printShaderSource(GLuint shaderhandle)
{
    GLint bufLength = 0;
    glGetShaderiv( shaderhandle, GL_SHADER_SOURCE_LENGTH, &bufLength );

    if( bufLength )
    {
        char* buf = (char*)malloc( bufLength );
        if( buf )
        {
            GLsizei lengthreturned;
            glGetShaderSource( shaderhandle, bufLength, &lengthreturned, buf );
            LOGError( LOGTag, "shader source:\n%s\n", buf );

            free(buf);
        }
    }
}

GLuint loadShader(GLenum shaderType, int numchunks, const char** ppChunks, int* pLengths)
{
    GLuint shaderid = glCreateShader(shaderType);
    if( shaderid )
    {
        glShaderSource( shaderid, numchunks, ppChunks, pLengths );

        glCompileShader( shaderid );
        GLint compiled = 0;
        glGetShaderiv( shaderid, GL_COMPILE_STATUS, &compiled );
        if( !compiled )
        {
            LOGError( LOGTag, "============================\n" );
            if( shaderType == GL_VERTEX_SHADER )
                LOGError( LOGTag, "Could not compile vertex shader:\n" );
#if USE_GEOMETRY_SHADER
            else if( shaderType == GL_GEOMETRY_SHADER )
                LOGError( LOGTag, "Could not compile geometry shader:\n" );
#endif //USE_GEOMETRY_SHADER
            else if( shaderType == GL_FRAGMENT_SHADER )
                LOGError( LOGTag, "Could not compile fragment shader:\n" );
            else 
                LOGError( LOGTag, "Could not compile unknown shader type:\n" );

#if !USE_D3D
            GLint infoLen = 0;
            glGetShaderiv( shaderid, GL_INFO_LOG_LENGTH, &infoLen );
            if( infoLen )
            {
                char* buf = (char*)malloc( infoLen );
                if( buf )
                {
                    printShaderSource( shaderid );

                    glGetShaderInfoLog( shaderid, infoLen, NULL, buf );
                    LOGError( LOGTag, "%s\n", buf );

                    //MyAssert( false );

                    free( buf );
                }
            }
#endif
            LOGError( LOGTag, "============================\n" );

            glDeleteShader( shaderid );
            shaderid = 0;
        }
    }

    return shaderid;
}

GLuint loadShader(GLenum shaderType, const char* pPreSource, int presourcelen, const char* pSource, int sourcelen, const char* pPassDefine)
{
    GLuint shaderid = glCreateShader(shaderType);
    if( shaderid )
    {
        if( pPreSource != 0 )
        {
            if( pPassDefine != 0 )
            {
                const char* sources[3] = { pPassDefine, pPreSource, pSource };
                int lengths[3] = { (int)strlen(pPassDefine), presourcelen, sourcelen };
                glShaderSource( shaderid, 3, sources, lengths );
            }
            else
            {
                const char* sources[2] = { pPreSource, pSource };
                int lengths[2] = { presourcelen, sourcelen };
                glShaderSource( shaderid, 2, sources, lengths );
            }
        }
        else
        {
            if( pPassDefine != 0 )
            {
                const char* sources[2] = { pPassDefine, pSource };
                int lengths[2] = { (int)strlen(pPassDefine), sourcelen };
                glShaderSource( shaderid, 2, sources, lengths );
            }
            else
            {
                const char* sources[1] = { pSource };
                int lengths[1] = { sourcelen };
                glShaderSource( shaderid, 1, sources, lengths );
            }
        }
        glCompileShader( shaderid );
        GLint compiled = 0;
        glGetShaderiv( shaderid, GL_COMPILE_STATUS, &compiled );
        if( !compiled )
        {
#if !USE_D3D
            GLint infoLen = 0;
            glGetShaderiv( shaderid, GL_INFO_LOG_LENGTH, &infoLen );
            if( infoLen )
            {
                char* buf = (char*)malloc(infoLen);
                if( buf )
                {
                    glGetShaderInfoLog( shaderid, infoLen, NULL, buf );
                    LOGError( LOGTag, "Could not compile shader %d:\n%s\n", shaderType, buf );
                    LOGError( LOGTag, "Shader Lengths: Pre-source: %d  source len: %d\n", presourcelen, sourcelen );
                    LOGError( LOGTag, "Shader Pre-source: \n%s\n", pPreSource );
                    LOGError( LOGTag, "Shader source: \n%s\n", pSource );

                    //MyAssert( false );
                    
                    free( buf );
                }
            }
#endif
            LOGError( LOGTag, "============================\n" );
            LOGError( LOGTag, "Could not compile shader %d:\n", shaderType );
            LOGError( LOGTag, "============================\n" );

            glDeleteShader( shaderid );
            shaderid = 0;
        }
    }

    return shaderid;
}

GLuint createProgram(GLuint* vsid, GLuint* fsid, int prevslen, const char* pPreVertexSource, int prefslen, const char* pPreFragmentSource, int numchunks, const char** ppChunks, int* pLengths, GLuint premadeprogramhandle)
{
    checkGlError( "createProgram" );

    ppChunks[1] = pPreVertexSource;
    pLengths[1] = prevslen;
    *vsid = loadShader( GL_VERTEX_SHADER, numchunks, ppChunks, pLengths );
    if( *vsid == 0 )
    {
        return 0;
    }

    ppChunks[1] = pPreFragmentSource;
    pLengths[1] = prefslen;
    *fsid = loadShader( GL_FRAGMENT_SHADER, numchunks, ppChunks, pLengths );
    if( *fsid == 0 )
    {
        glDeleteShader( *vsid );
        *vsid = 0;
        return 0;
    }

    GLuint programid = premadeprogramhandle;
    if( programid == 0 )
        programid = glCreateProgram();
    if( programid )
    {
        glAttachShader( programid, *vsid );
        checkGlError( "glAttachShader" );
        
        glAttachShader( programid, *fsid );
        checkGlError( "glAttachShader" );
        
        glLinkProgram( programid );
        GLint linkStatus = GL_FALSE;
        glGetProgramiv( programid, GL_LINK_STATUS, &linkStatus );

        if( linkStatus != GL_TRUE )
        {
#if !USE_D3D
            GLint bufLength = 0;
            glGetProgramiv( programid, GL_INFO_LOG_LENGTH, &bufLength );
            if( bufLength )
            {
                char* buf = (char*)malloc( bufLength );
                if( buf )
                {
                    glGetProgramInfoLog( programid, bufLength, 0, buf );
                    LOGError( LOGTag, "Could not link program:\n%s\n", buf );

                    //MyAssert( false );

                    printShaderSource( *vsid );
                    printShaderSource( *fsid );

                    free(buf);
                }
            }
#endif
            LOGError( LOGTag, "Could not link program:\n" );

            glDeleteProgram( programid );
            glDeleteShader( *vsid );
            glDeleteShader( *fsid );
            programid = 0;
            *vsid = 0;
            *fsid = 0;
        }
    }

    return programid;
}

GLuint createProgram(GLuint* vsid, GLuint* gsid, GLuint* fsid, int prevslen, const char* pPreVertexSource, int pregslen, const char* pPreGeometrySource, int prefslen, const char* pPreFragmentSource, int numchunks, const char** ppChunks, int* pLengths, GLuint premadeprogramhandle)
{
    checkGlError( "createProgram" );

    ppChunks[1] = pPreVertexSource;
    pLengths[1] = prevslen;
    *vsid = loadShader( GL_VERTEX_SHADER, numchunks, ppChunks, pLengths );
    if( *vsid == 0 )
    {
        return 0;
    }

#if USE_GEOMETRY_SHADER
    ppChunks[1] = pPreGeometrySource;
    pLengths[1] = pregslen;
    *gsid = loadShader( GL_GEOMETRY_SHADER, numchunks, ppChunks, pLengths );
    if( *gsid == 0 )
    {
        glDeleteShader( *vsid );
        *vsid = 0;
        return 0;
    }
#endif

    ppChunks[1] = pPreFragmentSource;
    pLengths[1] = prefslen;
    *fsid = loadShader( GL_FRAGMENT_SHADER, numchunks, ppChunks, pLengths );
    if( *fsid == 0 )
    {
        glDeleteShader( *vsid );
        glDeleteShader( *gsid );
        *vsid = 0;
        *gsid = 0;
        return 0;
    }

    GLuint programid = premadeprogramhandle;
    if( programid == 0 )
        programid = glCreateProgram();
    if( programid )
    {
        glAttachShader( programid, *vsid );
        checkGlError( "glAttachShader - Vertex" );
        
        glAttachShader( programid, *gsid );
        checkGlError( "glAttachShader - Geometry" );

        glAttachShader( programid, *fsid );
        checkGlError( "glAttachShader - Fragment" );
        
        glLinkProgram( programid );
        GLint linkStatus = GL_FALSE;
        glGetProgramiv( programid, GL_LINK_STATUS, &linkStatus );

        if( linkStatus != GL_TRUE )
        {
#if !USE_D3D
            GLint bufLength = 0;
            glGetProgramiv( programid, GL_INFO_LOG_LENGTH, &bufLength );
            if( bufLength )
            {
                char* buf = (char*)malloc( bufLength );
                if( buf )
                {
                    printShaderSource( *vsid );
                    printShaderSource( *fsid );

                    glGetProgramInfoLog( programid, bufLength, 0, buf );
                    LOGError( LOGTag, "Could not link program:\n%s\n", buf );

                    //MyAssert( false );

                    free(buf);
                }
            }
#endif
            LOGError( LOGTag, "Could not link program:\n" );

            glDeleteProgram( programid );
            glDeleteShader( *vsid );
            glDeleteShader( *fsid );
            programid = 0;
            *vsid = 0;
            *fsid = 0;
        }
    }

    return programid;
}

GLuint createProgram(int vslen, const char* pVertexSource, int fslen, const char* pFragmentSource, GLuint* vsid, GLuint* fsid, int prevslen, const char* pPreVertexSource, int prefslen, const char* pPreFragmentSource, const char* pPassDefine, GLuint premadeprogramhandle)
{
    checkGlError( "createProgram" );

    *vsid = loadShader( GL_VERTEX_SHADER, pPreVertexSource, prevslen, pVertexSource, vslen, pPassDefine );
    if( *vsid == 0 )
    {
        return 0;
    }

    *fsid = loadShader( GL_FRAGMENT_SHADER, pPreFragmentSource, prefslen, pFragmentSource, fslen, pPassDefine );
    if( *fsid == 0 )
    {
        glDeleteShader( *vsid );
        *vsid = 0;
        return 0;
    }

    GLuint programid = premadeprogramhandle;
    if( programid == 0 )
        programid = glCreateProgram();
    if( programid )
    {
        glAttachShader( programid, *vsid );
        checkGlError( "glAttachShader" );
        
        glAttachShader( programid, *fsid );
        checkGlError( "glAttachShader" );
        
        glLinkProgram( programid );
        GLint linkStatus = GL_FALSE;
        glGetProgramiv( programid, GL_LINK_STATUS, &linkStatus );

        if( linkStatus != GL_TRUE )
        {
#if !USE_D3D
            GLint bufLength = 0;
            glGetProgramiv( programid, GL_INFO_LOG_LENGTH, &bufLength );
            if( bufLength )
            {
                char* buf = (char*)malloc( bufLength );
                if( buf )
                {
                    glGetProgramInfoLog( programid, bufLength, 0, buf );
                    LOGError( LOGTag, "Could not link program:\n%s\n", buf );

                    //MyAssert( false );

                    free(buf);
                }
            }
#endif
            LOGError( LOGTag, "Could not link program:\n" );

            glDeleteProgram( programid );
            glDeleteShader( *vsid );
            glDeleteShader( *fsid );
            programid = 0;
            *vsid = 0;
            *fsid = 0;
        }
    }

    return programid;
}
