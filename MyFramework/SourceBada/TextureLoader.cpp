//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#include "TextureLoader.h"
#include "../../Soil/SOIL.h"

MyFileObject* RequestFile(const char* filename)
{
    // TODO: uncomment next line
    //return g_pFileManager->RequestFile( filename );

    LOGInfo( LOGTag, "OLD FASHIONED RequestFile %s\n", filename );

    char fullpath[MAX_PATH];
    sprintf_s( fullpath, MAX_PATH, "app/native/%s", filename);

    int length = 0;
    MyFileObject* file = MyNew MyFileObject;
    char* buffer = LoadFile( fullpath, &length );
    file->FakeFileLoad( buffer, length );

    return file;
}

char* LoadFile(const char* filename, int* length)
{
    MyAssert( false ); // migrate to g_pFileManager->RequestFile( filename );

    char* filecontents = 0;

    FILE* filehandle;
    filehandle = fopen( filename, "rb" );

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = MyNew char[size]; //+1];
        fread( filecontents, size, 1, filehandle );
        //filecontents[size] = 0;

        if( length )
            *length = size;
        
        fclose( filehandle );
    }

    return filecontents;
}

//GLuint LoadTextureFromMemory(TextureDefinition* texturedef)
//{
//    GLuint tex_ID;
//
//    char* buffer = texturedef->m_pFile->m_pBuffer;
//    int length = texturedef->m_pFile->m_FileLength;
//
//    tex_ID = SOIL_load_OGL_texture_from_memory( (unsigned char*)buffer, length, 
//                                    SOIL_LOAD_AUTO,
//                                    SOIL_CREATE_NEW_ID,
//                                    SOIL_FLAG_POWER_OF_TWO
//                                    //| SOIL_FLAG_MIPMAPS
//                                    //| SOIL_FLAG_MULTIPLY_ALPHA
//                                    //| SOIL_FLAG_COMPRESS_TO_DXT
//                                    | SOIL_FLAG_DDS_LOAD_DIRECT
//                                    //| SOIL_FLAG_NTSC_SAFE_RGB
//                                    //| SOIL_FLAG_CoCg_Y
//                                    //| SOIL_FLAG_TEXTURE_RECTANGLE
//                                  );
//
//    g_pRenderer->SetTextureMinMagFilters( ..., m_MinFilter, m_MagFilter );
//    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturedef->m_MinFilter ); //LINEAR );
//    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texturedef->m_MagFilter ); //GL_LINEAR );
//
//    return tex_ID;
//}

GLuint LoadTexture(const char* filename)
{
    //GLuint texture;

    //// allocate a texture name
    //glGenTextures( 1, &texture );

    //// select our current texture
    //glBindTexture( GL_TEXTURE_2D, texture );

    //return texture;

    GLuint tex_ID;

    tex_ID = SOIL_load_OGL_texture( filename,
                                    SOIL_LOAD_AUTO,
                                    SOIL_CREATE_NEW_ID,
                                    SOIL_FLAG_POWER_OF_TWO
                                    | SOIL_FLAG_MIPMAPS
                                    //| SOIL_FLAG_MULTIPLY_ALPHA
                                    //| SOIL_FLAG_COMPRESS_TO_DXT
                                    | SOIL_FLAG_DDS_LOAD_DIRECT
                                    //| SOIL_FLAG_NTSC_SAFE_RGB
                                    //| SOIL_FLAG_CoCg_Y
                                    //| SOIL_FLAG_TEXTURE_RECTANGLE
                                  );

    //if( tex_ID > 0 )
    //{
    //    // enable texturing
    //    glEnable( GL_TEXTURE_2D );
    //    //glEnable( 0x84F5 );// enables texture rectangle
    //    //  bind an OpenGL texture ID
    //    glBindTexture( GL_TEXTURE_2D, tex_ID );
    //}
    //else
    //{
    //    MyAssert( false );

    //    // loading of the texture failed...why?
    //    glDisable( GL_TEXTURE_2D );
    //}

    return tex_ID;
}
