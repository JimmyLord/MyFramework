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

#include "CommonHeader.h"

#pragma warning (disable:4996)

// taken from: http://dave.thehorners.com/content/view/124/67 and massaged.
// but really simple otherwise.
void SaveScreenshot(int windowWidth, int windowHeight, char* filename)
{
#if !USE_D3D
    byte* bmpBuffer = (byte*)malloc( windowWidth*windowHeight*3 );
    if( bmpBuffer == 0 )
        return;

    // not sure why GL_BGR is working for me,
    //   might be how we're initing the framebuffers or it's something do do with the bitmap below.
    //   either way, good enough for now since it works.
    glReadPixels( 0, 0, windowWidth, windowHeight, GL_BGR, GL_UNSIGNED_BYTE, bmpBuffer );

    FILE* filePtr = fopen( filename, "wb" );
    if( !filePtr )
        return;

    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;

    bitmapFileHeader.bfType = 0x4D42; //"BM"
    bitmapFileHeader.bfSize = windowWidth*windowHeight*3;
    bitmapFileHeader.bfReserved1 = 0;
    bitmapFileHeader.bfReserved2 = 0;
    bitmapFileHeader.bfOffBits =
    sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

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

    free( bmpBuffer );
#endif
}
