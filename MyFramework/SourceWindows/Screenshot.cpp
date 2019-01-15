//
// Copyright (c) 2012-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyFrameworkPCH.h"
#pragma warning( push )
#pragma warning( disable : 4996 )
#include "../../Libraries/LodePNG/lodepng.h"
#pragma warning( pop )

#pragma warning( disable : 4996 )

#include <gl/GL.h>
#include "../SourceCommon/Renderers/OpenGL/GLHelpers.h"

// Taken from: http://dave.thehorners.com/content/view/124/67 and massaged.
// But really simple otherwise.
void SaveScreenshot(int windowWidth, int windowHeight, char* filename)
{
#if USE_OPENGL
    // Reduce capture size to multiple of 4.
    // TODO: Fix this (bmp only, png is fine), it's cropping the right edge of the image.
    //windowWidth -= windowWidth & 0x03;

    LOGInfo( LOGTag, "Saving Screenshot (%dx%d)\n", windowWidth, windowHeight );

    byte* bmpBuffer = (byte*)malloc( windowWidth*windowHeight*3 );
    if( bmpBuffer == nullptr )
        return;

    // By default glReadPixels() expects width/height to be multiples of 4, change that to 1 and change it back after.
    //glReadBuffer( GL_FRONT );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // Not sure why GL_BGR (for bmps) is working for me,
    //   might be how we're initing the framebuffers or it's something do do with the bitmap below.
    //   either way, good enough for now since it works.
    glReadPixels( 0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, bmpBuffer );

    checkGlError( "glReadPixels" );

    if( true ) // Save as png.
    {
        // Flip bmp buffer vertically.
        {
            // Temp allocation big enough for one line.
            unsigned char* temp = MyNew unsigned char[windowWidth*3];
            int lineSize = windowWidth*3 * sizeof(unsigned char);

            unsigned char* buffer = (unsigned char*)bmpBuffer;
            for( int y=0; y<windowHeight/2; y++ )
            {
                int LineOffsetY = y*windowWidth*3;
                int LineOffsetHminusY = (windowHeight-1-y)*windowWidth*3;

                memcpy( temp, &buffer[LineOffsetY], lineSize );
                memcpy( &buffer[LineOffsetY], &buffer[LineOffsetHminusY], lineSize );
                memcpy( &buffer[LineOffsetHminusY], temp, lineSize );
            }

            delete[] temp;
        }

        unsigned char* pngBuffer;
        size_t pngSize;
        lodepng_encode24( &pngBuffer, &pngSize, bmpBuffer, windowWidth, windowHeight );

        char finalFilename[MAX_PATH];
        sprintf_s( finalFilename, MAX_PATH, "%s.png", filename );
        lodepng_save_file( pngBuffer, pngSize, finalFilename );
    }
    else // Save as bmp.
    {
        // Saving as bmp is broken for non multiple of 4 sized windows... not sure why.

        char finalFilename[MAX_PATH];
        sprintf_s( finalFilename, MAX_PATH, "%s.bmp", filename );

        FILE* filePtr = fopen( finalFilename, "wb" );
        if( !filePtr )
            return;

        BITMAPFILEHEADER bitmapFileHeader;
        BITMAPINFOHEADER bitmapInfoHeader;

        bitmapFileHeader.bfType = 0x4D42; //"BM"
        bitmapFileHeader.bfSize = windowWidth*windowHeight*3;
        bitmapFileHeader.bfReserved1 = 0;
        bitmapFileHeader.bfReserved2 = 0;
        bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfoHeader.biWidth = windowWidth;
        bitmapInfoHeader.biHeight = windowHeight;
        bitmapInfoHeader.biPlanes = 1;
        bitmapInfoHeader.biBitCount = 24;
        bitmapInfoHeader.biCompression = BI_RGB;
        bitmapInfoHeader.biSizeImage = 0;
        bitmapInfoHeader.biXPelsPerMeter = 0; // ?
        bitmapInfoHeader.biYPelsPerMeter = 0; // ?
        bitmapInfoHeader.biClrUsed = 0;
        bitmapInfoHeader.biClrImportant = 0;

        fwrite( &bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr );
        fwrite( &bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr );
        fwrite( bmpBuffer, windowWidth*windowHeight*3, 1, filePtr );
        fclose( filePtr );
    }

    free( bmpBuffer );

    glPixelStorei( GL_PACK_ALIGNMENT, 4 );
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
#endif
}
