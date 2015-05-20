//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>

#include <GLES/gl.h>

#include "UnthoughtOutAndroidHeader.h"

MyFileObject* RequestFile(const char* filename)
{
    // TODO: uncomment next line
    return g_pFileManager->RequestFile( filename );

    //LOGInfo( LOGTag, "OLD FASHIONED RequestFile %s\n", filename );

    //MyFileObject* pFile = 0;

    //int length = 0;
    //
    //int len = (int)strlen( filename );
    //if( len > 5 && strcmp( &filename[len-5], ".glsl" ) == 0 )
    //    pFile = MyNew MyFileObjectShader;
    //else
    //    pFile = MyNew MyFileObject;

    //char* buffer = LoadFile( filename, &length );
    //pFile->FakeFileLoad( buffer, length );

    //return pFile;
}

MyFileObject* RequestTexture(const char* filename, TextureDefinition* texturedef)
{
    LOGInfo( LOGTag, "RequestFile %s - texturedef %d\n", filename, texturedef );

    MyFileObject* file = MyNew MyFileObject;
    
    char* pBuffer = LoadTexture( filename, &texturedef->m_Width, &texturedef->m_Height );
    LOGInfo( LOGTag, "RequestTexture - LoadTexture done" );
    int length = texturedef->m_Width*texturedef->m_Height*4;
    LOGInfo( LOGTag, "RequestTexture - length = %d", length );
    file->FakeFileLoad( pBuffer, length );
    LOGInfo( LOGTag, "RequestTexture - FakeFileLoad done" );
    
    texturedef->m_pFile = file;
    texturedef->m_TextureID = Android_LoadTextureFromMemory( texturedef );
    
    return file;
}

char* LoadFile(const char* filepath, int* length)
{
    LOGInfo( LOGTag, ">>>>>>>>>>>>>>>> LoadFile - Loading %s", filepath );

    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "LoadFile() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return 0;
    }

//- get a request to load an uncompressed resource file from a native function
//- call back to Java to get file size from the resource's file descriptor
//- malloc required memory in native code
//- create a direct byte buffer with NewDirectByteBuffer in native code
//- pass this buffer back to Java to use for reading the resource file
//- Java code then uses java.nio.channels.FileChannel to load the file:
//    AssetFileDescriptor.createInputStream().getChannel().read( passed in ByteBuffer jobject passed in ); 
    LOGInfo( LOGTag, "LoadFile - jenv %p", g_pJavaEnvironment );

    jstring name = g_pJavaEnvironment->NewStringUTF( filepath );
    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pBMPFactoryLoader );

    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "GetBinaryFileSize", "(Ljava/lang/String;)J" );
    long filesize = g_pJavaEnvironment->CallLongMethod( g_pBMPFactoryLoader, methodid, name );
    
    if( filesize == 0 )
    {
        LOGError( LOGTag, "=========================================================");
        LOGError( LOGTag, "= LoadFile - ERROR LOADING %s", filepath );
        LOGError( LOGTag, "=========================================================");
        return 0;
    }
    LOGInfo( LOGTag, "LoadFile - filesize: %d", (int)filesize );
    if( length )
        *length = filesize;

    char* filecontents = new char[filesize+1];
    jobject bytebuffer = g_pJavaEnvironment->NewDirectByteBuffer( filecontents, filesize+1 );
    LOGInfo( LOGTag, "LoadBinaryFile - created buffer" );

    LOGInfo( LOGTag, "LoadBinaryFile - bmploadercls %d", cls );
    //jclass cls = g_pJavaEnvironment->GetObjectClass( g_pAssetManager );
    // Ask the bitmaploader for a file

    methodid = g_pJavaEnvironment->GetMethodID( cls, "LoadBinaryFile", "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V" );
    LOGInfo( LOGTag, "LoadBinaryFile - methodid %d", methodid );

    g_pJavaEnvironment->CallVoidMethod( g_pBMPFactoryLoader, methodid, name, bytebuffer );
    filecontents[filesize] = 0;

    g_pJavaEnvironment->DeleteLocalRef( name );

    LOGInfo( LOGTag, "LoadBinaryFile - done?" );

    //LOGInfo( LOGTag, filecontents );

    return filecontents;
}

char* LoadTexture(const char* filepath, int* widthout, int* heightout)
{
    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "LoadTexture() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return 0;
    }

    if( widthout )
        *widthout = 0;
    if( heightout )
        *heightout = 0;

    LOGInfo( LOGTag, ">>> LoadTexture - Loading %s", filepath );

    //const char* filepath = "VtNTitle.png"; //"images/myimage.png";

    //LOGInfo( LOGTag, "LoadTexture - jenv %p", g_pJavaEnvironment );
    //LOGInfo( LOGTag, "LoadTexture - bitmaploader %p", g_pBMPFactoryLoader );

    int version = g_pJavaEnvironment->GetVersion();
    LOGInfo( LOGTag, "LoadTexture - version %d", version );

    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pBMPFactoryLoader );
    LOGInfo( LOGTag, "LoadTexture - bmploadercls %d", cls );

    // Ask the bitmaploader for a bitmap
    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "open", "(Ljava/lang/String;)Landroid/graphics/Bitmap;" );
    LOGInfo( LOGTag, "LoadTexture - methodid %d", methodid );

    if( methodid == 0 )
        return 0;

    jstring name = g_pJavaEnvironment->NewStringUTF( filepath );
    jobject bmp = g_pJavaEnvironment->CallObjectMethod( g_pBMPFactoryLoader, methodid, name );
    g_pJavaEnvironment->DeleteLocalRef( name );
    LOGInfo( LOGTag, "LoadTexture - bmp object %d", bmp );

    if( bmp == 0 )
        return 0;

    //g_pJavaEnvironment->NewGlobalRef( bmp );

    // Get image dimensions
    methodid = g_pJavaEnvironment->GetMethodID( cls, "getWidth", "(Landroid/graphics/Bitmap;)I" );
    int width = g_pJavaEnvironment->CallIntMethod( g_pBMPFactoryLoader, methodid, bmp );
    LOGInfo( LOGTag, "LoadTexture - bmp width %d", width );

    methodid = g_pJavaEnvironment->GetMethodID( cls, "getHeight", "(Landroid/graphics/Bitmap;)I");
    int height = g_pJavaEnvironment->CallIntMethod( g_pBMPFactoryLoader, methodid, bmp);
    LOGInfo( LOGTag, "LoadTexture - bmp height %d", height );

    if( widthout )
        *widthout = width;
    if( heightout )
        *heightout = height;

    // Get pixels
    jintArray pixelarray = g_pJavaEnvironment->NewIntArray( width * height );
    //g_pJavaEnvironment->NewGlobalRef( pixelarray );
    methodid = g_pJavaEnvironment->GetMethodID( cls, "getPixels", "(Landroid/graphics/Bitmap;[I)V" );
    g_pJavaEnvironment->CallVoidMethod( g_pBMPFactoryLoader, methodid, bmp, pixelarray );

    LOGInfo( LOGTag, "LoadTexture - called getPixels" );

    char* pBuffer;

    if( 1 )
    {
        pBuffer = MyNew char[width*height*4];

        g_pJavaEnvironment->GetIntArrayRegion( pixelarray, 0, width*height, (int*)pBuffer );

        // flip pixels from abgr to argb
        for( int i=0; i<width*height; i++ )
        {
            int* bufferasint = (int*)&pBuffer[i*4];
            int pixel = *bufferasint;
            *bufferasint = (pixel & 0xff00ff00) | ((pixel & 0x00ff0000) >> 16 ) | ((pixel & 0x000000ff) << 16 );
        }

        LOGInfo( LOGTag, "LoadTexture - copied pixels from pixelarray" );
    }
    //else
    //{
    //    jint* pixels = g_pJavaEnvironment->GetIntArrayElements( pixelarray, 0 );

    //    // flip pixels from abgr to argb
    //    for( int i=0; i<width*height; i++ )
    //    {
    //        int pixel = pixels[i];
    //        pixels[i] = (pixel & 0xff00ff00) | ((pixel & 0x00ff0000) >> 16 ) | ((pixel & 0x000000ff) << 16 );
    //    }

    //    LOGInfo( LOGTag, "LoadTexture - got pixels" );

    //    //GLuint tex_id = Android_LoadTextureFromMemory( pixels, width, height );
    //    pBuffer = MyNew char[width*height*4];
    //    memcpy( pBuffer, pixels, width*height*4 );

    //    LOGInfo( LOGTag, "LoadTexture - Allocated and copied pixels" );

    //    g_pJavaEnvironment->ReleaseIntArrayElements( pixelarray, pixels, JNI_ABORT ); // abort won't copy data back if a copy was passed to us.
    //    //g_pJavaEnvironment->DeleteGlobalRef( pixelarray );

    //    LOGInfo( LOGTag, "LoadTexture - released pixelarray" );
    //}

    // Free image
    methodid = g_pJavaEnvironment->GetMethodID( cls, "close", "(Landroid/graphics/Bitmap;)V" );
    LOGInfo( LOGTag, "LoadTexture - close methodid %d", methodid );

    g_pJavaEnvironment->CallVoidMethod( g_pBMPFactoryLoader, methodid, bmp );
    LOGInfo( LOGTag, "LoadTexture - close called" );

    //g_pJavaEnvironment->DeleteGlobalRef( bmp );

    LOGInfo( LOGTag, "LoadTexture - returning buffer" );

    return pBuffer;
}

GLuint Android_LoadTextureFromMemory(TextureDefinition* texturedef)
{
    LOGInfo( LOGTag, "Android_LoadTextureFromMemory texturedef(%d)\n", texturedef );
    if( texturedef == 0 )
        return 0;

    checkGlError( "Android_LoadTextureFromMemory" );

    GLuint tex_id = 0;

    glGenTextures( 1, &tex_id );
    checkGlError( "glGenTextures" );

    LOGInfo( LOGTag, "Android_LoadTextureFromMemory file(%d)", texturedef->m_pFile );

    char* buffer = texturedef->m_pFile->m_pBuffer;
    int width = texturedef->m_Width;
    int height = texturedef->m_Height;
    int minfilter = texturedef->m_MinFilter;
    int magfilter = texturedef->m_MagFilter;

    LOGInfo( LOGTag, "Android_LoadTextureFromMemory - tex_id %d, size: %dx%d", tex_id, width, height );

    if( tex_id != 0 )
    {
        glBindTexture( GL_TEXTURE_2D, tex_id );
        checkGlError( "glBindTexture" );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter ); //LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter ); //GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
        checkGlError( "glTexImage2D" );
    }

    return tex_id;
}
