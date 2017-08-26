//
// Copyright (c) 2012-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "CommonHeader.h"
#pragma warning( push )
#pragma warning( disable : 4996 )
#include "../../Libraries/LodePNG/lodepng.h"
#pragma warning( pop )

#pragma warning( disable : 4996 )

// taken from: http://dave.thehorners.com/content/view/124/67 and massaged.
// but really simple otherwise.
void SaveScreenshot(int windowWidth, int windowHeight, char* filename)
{
#if !USE_D3D
    // reduce capture size to multiple of 4
    // TODO: fix this (bmp only, png is fine), it's cropping the right edge of the image.
    //windowWidth -= windowWidth & 0x03;

    LOGInfo( LOGTag, "Saving Screenshot (%dx%d)\n", windowWidth, windowHeight );

    byte* bmpbuffer = (byte*)malloc( windowWidth*windowHeight*3 );
    if( bmpbuffer == 0 )
        return;

    // by default glReadPixels() expects width/height to be multiples of 4, change that to 1 and change it back after.
    //glReadBuffer( GL_FRONT );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // not sure why GL_BGR (for bmps) is working for me,
    //   might be how we're initing the framebuffers or it's something do do with the bitmap below.
    //   either way, good enough for now since it works.
    glReadPixels( 0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, bmpbuffer );

    checkGlError( "glReadPixels" );

    if( 1 ) // save as png
    {
        // flip bmpbuffer vertically.
        {
            // temp allocation big enough for one line
            unsigned char* temp = new unsigned char[windowWidth*3];
            int linesize = windowWidth*3 * sizeof(unsigned char);

            unsigned char* buffer = (unsigned char*)bmpbuffer;
            for( int y=0; y<windowHeight/2; y++ )
            {
                int LineOffsetY = y*windowWidth*3;
                int LineOffsetHminusY = (windowHeight-1-y)*windowWidth*3;

                memcpy( temp, &buffer[LineOffsetY], linesize );
                memcpy( &buffer[LineOffsetY], &buffer[LineOffsetHminusY], linesize );
                memcpy( &buffer[LineOffsetHminusY], temp, linesize );
            }

            delete[] temp;
        }

        unsigned char* pngbuffer;
        size_t pngsize;
        lodepng_encode24( &pngbuffer, &pngsize, bmpbuffer, windowWidth, windowHeight );

        char finalfilename[MAX_PATH];
        sprintf_s( finalfilename, MAX_PATH, "%s.png", filename );
        lodepng_save_file( pngbuffer, pngsize, finalfilename );
    }
    else
    {
        // saving as bmp is broken for non multiple of 4 sized windows... not sure why.

        char finalfilename[MAX_PATH];
        sprintf_s( finalfilename, MAX_PATH, "%s.bmp", filename );

        FILE* filePtr = fopen( finalfilename, "wb" ); // save as bmp
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
        fwrite( bmpbuffer, windowWidth*windowHeight*3, 1, filePtr );
        fclose( filePtr );
    }

    free( bmpbuffer );

    glPixelStorei( GL_PACK_ALIGNMENT, 4 );
    //glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
#endif
}
