//
// Copyright (c) 2012-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "../SourceCommon/CommonHeader.h"
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#if 0 // also add -landroid to linker settings
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif
#include <stdint.h>

#include <GLES/gl.h>

#include "JavaInterfaceCPP.h"

MyFileObject* RequestFile(const char* filename)
{
    return g_pFileManager->RequestFile( filename );
}

//MyFileObject* RequestTexture(const char* filename, TextureDefinition* texturedef)
//{
//    // this function should be obsolete...
//    LOGInfo( LOGTag, "AndroidRequestFile %s - texturedef %d\n", filename, texturedef );
//
//    MyFileObject* file = MyNew MyFileObject;
//    
//    char* pBuffer = LoadTexture( filename, &texturedef->m_Width, &texturedef->m_Height );
//    LOGInfo( LOGTag, "RequestTexture - LoadTexture done" );
//    int length = texturedef->m_Width*texturedef->m_Height*4;
//    LOGInfo( LOGTag, "RequestTexture - length = %d", length );
//    file->FakeFileLoad( pBuffer, length );
//    LOGInfo( LOGTag, "RequestTexture - FakeFileLoad done" );
//    
//    texturedef->m_pFile = file;
//    texturedef->m_TextureID = Android_LoadTextureFromMemory( texturedef );
//    
//    return file;
//}

char* LoadFile(const char* filepath, int* length)
{
    LOGInfo( LOGTag, ">>>>>>>>>>>>>>>> LoadFile - Loading %s", filepath );

    char* filecontents = 0;

#if 0 // requires Android 2.3?... still using 2.2  - not really tested, likely needs work.
    
    {
        AAssetManager* pManager = AAssetManager_fromJava( g_pJavaEnvironment, g_pAssetManager );

        AAsset* asset = AAssetManager_open( pManager, filepath, AASSET_MODE_STREAMING );
        MyAssert( asset );
        if( asset )
        {
            off_t length = AAsset_getLength( asset );
	        char* filecontents = MyNew char[length];
	    
            int bytesread = AAsset_read(asset, filecontents, length);
            MyAssert( bytesread == length );

	        AAsset_close(asset);
        }
    }

#else

    if( g_pJavaEnvironment == 0 || g_pMainActivity == 0 )
    {
        LOGError( LOGTag, "LoadFile() sanity check failed: g_pJavaEnvironment=%p g_pMainActivity=%p\n", g_pJavaEnvironment, g_pMainActivity );
        return 0;
    }

    //- call Java to get file size
    //- allocate required memory in native code
    //- create a direct byte buffer with NewDirectByteBuffer in native code
    //- pass this buffer back to Java to use for reading the asset file

    // Make a java string for the filename.
    jstring filename = g_pJavaEnvironment->NewStringUTF( filepath );
    
    // Query for the length of the file from java.
    jclass cls = g_pJavaEnvironment->GetObjectClass( g_pBMPFactoryLoader );
    jmethodID methodid = g_pJavaEnvironment->GetMethodID( cls, "GetBinaryFileSize", "(Ljava/lang/String;)J" );
    long filelength = g_pJavaEnvironment->CallLongMethod( g_pBMPFactoryLoader, methodid, filename );
    
    if( filelength == 0 )
    {
        LOGError( LOGTag, "=========================================================");
        LOGError( LOGTag, "= LoadFile - ERROR LOADING %s", filepath );
        LOGError( LOGTag, "=========================================================");
        return 0;
    }
    //LOGInfo( LOGTag, "   LoadFile - filelength: %d", (int)filelength );
    if( length )
        *length = filelength;

    // Allocate a buffer to store the file and create a DirectByteBuffer for java to have pointer to our buffer.
    filecontents = new char[filelength+1]; // +1 for null terminator, added below.
    jobject bytebuffer = g_pJavaEnvironment->NewDirectByteBuffer( filecontents, filelength+1 );

    // Call java to load the actual file.
    methodid = g_pJavaEnvironment->GetMethodID( cls, "LoadBinaryFile", "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V" );
    g_pJavaEnvironment->CallVoidMethod( g_pBMPFactoryLoader, methodid, filename, bytebuffer );

    // stick a null terminator on our file, some code doesn't look at the file length.
    filecontents[filelength] = 0;

    g_pJavaEnvironment->DeleteLocalRef( filename );

    //LOGInfo( LOGTag, "   Done: LoadBinaryFile: %s", filepath );
    //LOGInfo( LOGTag, filecontents );
#endif

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

    const char* buffer = texturedef->m_pFile->GetBuffer();
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
