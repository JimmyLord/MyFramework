//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#include "TextureLoader.h"
#include "../Soil/SOIL.h"

MyFileObject* RequestFile(const char* filename)
{
    // TODO: uncomment next line
    //return g_pFileManager->RequestFile( filename );

    LOGInfo( LOGTag, "OLD FASHIONED RequestFile %s\n", filename );

    int length = 0;
    MyFileObject* file = MyNew MyFileObject;
    char* buffer = LoadFile( filename, &length );
    if( buffer == 0 )
    {
        LOGInfo( LOGTag, "*********** File not found: %s\n", filename );
        //MyAssert( false );
        file->m_LoadFailed = true;
    }
    else
    {
        file->FakeFileLoad( buffer, length );
    }

    return file;
}

char* LoadFile(const char* filename, int* length)
{
    MyAssert( false ); // migrate to g_pFileManager->RequestFile( filename );

    char* filecontents = 0;

    FILE* filehandle;
    errno_t error = fopen_s( &filehandle, filename, "rb" );
    
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

//GLuint LoadTextureFromMemory(TextureDefinition* texturedef) //char* buffer, int length)
//{
//    GLuint textureid = 0;
//
//    char* buffer = texturedef->m_pFile->m_pBuffer;
//    int length = texturedef->m_pFile->m_FileLength;
//
//    //LOGInfo( LOGTag, "********************************* LoadTextureFromMemory() not implemented\n" );
//
//    int i;
//    for( i=0; i<MAX_TEXTURES; i++ )
//    {
//        if( g_D3DTextures[i].m_InUse == false )
//        {
//            g_D3DTextures[i].m_InUse = true;
//            break;
//        }
//    }
//
//    textureid = i;
//
//    if( textureid < MAX_TEXTURES )
//    {
//        // Use soil to load the png into a buffer.
//        unsigned char* img;
//        int width, height, channels;
//
//        img = SOIL_load_image_from_memory(
//                        (unsigned char*)buffer, length,
//                        &width, &height, &channels,
//                        SOIL_LOAD_RGBA ); //SOIL_LOAD_AUTO );
//
//        // create a descriptor for the texture and buffered data.
//        D3D11_TEXTURE2D_DESC tex2d_desc;
//        tex2d_desc.Width = width;
//        tex2d_desc.Height = height;
//        tex2d_desc.MipLevels = 1;
//        tex2d_desc.ArraySize = 1;
//        tex2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//        tex2d_desc.SampleDesc.Count = 1;
//        tex2d_desc.SampleDesc.Quality = 0;
//        tex2d_desc.Usage = D3D11_USAGE_DEFAULT;
//        tex2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//        tex2d_desc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE;
//        tex2d_desc.MiscFlags = 0; //D3D11_RESOURCE_MISC_TEXTURECUBE
//    
//        D3D11_SUBRESOURCE_DATA subdata;
//        subdata.pSysMem = img;
//        subdata.SysMemPitch = width*4;
//        subdata.SysMemSlicePitch = width*height*4;
//
//        // create a texture.
//        ID3D11Texture2D* pTexture;
//        g_pD3DDevice->CreateTexture2D( &tex2d_desc, &subdata, &pTexture );
//
//        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
//        memset( &SRVDesc, 0, sizeof( SRVDesc ) );
//        SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//        SRVDesc.Texture2D.MipLevels = 1;
//
//        ID3D11ShaderResourceView* pResourceView;
//        HRESULT result = g_pD3DDevice->CreateShaderResourceView( pTexture, &SRVDesc, &pResourceView );
//
//        SOIL_free_image_data( img );
//
//        if( FAILED(result) )
//        {
//            g_D3DTextures[textureid].m_InUse = false;
//            pTexture->Release();
//            return false;
//        }
//
//        g_D3DTextures[textureid].m_pTexture = pTexture;
//        g_D3DTextures[textureid].m_ResourceView = pResourceView;
//    }
//
//    // TODO: make these lines do something.
//    glBindTexture( GL_TEXTURE_2D, textureid );
//    g_pRenderer->SetTextureMinMagFilters( ..., m_MinFilter, m_MagFilter );
//    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturedef->m_MinFilter );
//    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texturedef->m_MagFilter );
//
//    return textureid+1;
//}

GLuint LoadTexture(const char* filename)
{
    //GLuint texture;

    //// allocate a texture name
    //glGenTextures( 1, &texture );

    //// select our current texture
    //glBindTexture( GL_TEXTURE_2D, texture );

    //return texture;

    GLuint tex_ID = 0;

    LOGInfo( LOGTag, "********************************* LoadTexture() not implemented\n" );

    //tex_ID = SOIL_load_OGL_texture( filename,
    //                                SOIL_LOAD_AUTO,
    //                                SOIL_CREATE_NEW_ID,
    //                                SOIL_FLAG_POWER_OF_TWO
    //                                //| SOIL_FLAG_MIPMAPS
    //                                //| SOIL_FLAG_MULTIPLY_ALPHA
    //                                //| SOIL_FLAG_COMPRESS_TO_DXT
    //                                | SOIL_FLAG_DDS_LOAD_DIRECT
    //                                //| SOIL_FLAG_NTSC_SAFE_RGB
    //                                //| SOIL_FLAG_CoCg_Y
    //                                //| SOIL_FLAG_TEXTURE_RECTANGLE
    //                              );

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
