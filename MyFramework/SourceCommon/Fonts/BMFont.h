//
// Copyright (c) 2012-2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __BMFont_H__
#define __BMFont_H__

#include "../Shaders/VertexFormats.h"
#include "../DataTypes/ColorStructs.h"

class BMFont
{
protected:
    struct FontProperties
    {
        //<info face="System" size="24" bold="0" italic="0" charset="" unicode="1" stretchH="100" smooth="1" aa="1" padding="0,0,0,0" spacing="1,1" outline="0"/>
        //<common lineHeight="16" base="13" scaleW="128" scaleH="128" pages="1" packed="0" alphaChnl="0" redChnl="4" greenChnl="4" blueChnl="4"/>
        //  <page id="0" file="dontcheckin-testing_0.png" />

        unsigned short lineHeight;  // This is the distance in pixels between each line of text.
        unsigned short base;        // The number of pixels from the absolute top of the line to the base of the characters.
        unsigned short scaleW;      // The width of the texture, normally used to scale the x pos of the character image.
        unsigned short scaleH;
        unsigned short pages;       // The number of texture pages included in the font.
        bool packed;                // Set to 1 if the monochrome characters have been packed into each of the texture channels. In this case alphaChnl describes what is stored in each channel.
        unsigned char alphaChnl;    // Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyph and the outline, 3 if its set to zero, and 4 if its set to one.
        unsigned char redChnl;
        unsigned char greenChnl;
        unsigned char blueChnl;
    };

    struct CharDescriptor
    {
        //<char id="32" x="85" y="33" width="4" height="0" xoffset="0" yoffset="16" xadvance="4" page="0" chnl="15" />

        unsigned int id;
        unsigned short x;       // The left position of the character image in the texture.
        unsigned short y;
        unsigned short width;   // The width of the character image in the texture.
        unsigned short height;
        short xoffset;          // How much the current position should be offset when copying the image from the texture to the screen.
        short yoffset;
        short xadvance;         // How much the current position should be advanced after drawing the character.
        unsigned char page;     // The texture page where the character image is found.
        unsigned char chnl;     // The texture channel where the character image is found (1 = blue, 2 = green, 4 = red, 8 = alpha, 15 = all channels).
    };

    struct KerningPair
    {
        unsigned int first;     // The first character id.
        unsigned int second;    // The second character id.
        short amount;           // How much the x position should be adjusted when drawing the second character immediately following the first.
    };

protected:
    bool m_Initialized;
    char m_ImageName[MAX_PATH];
    FontProperties m_Properties;
    CharDescriptor* m_Chars;
    KerningPair* m_KerningPairs;
    unsigned int m_NumChars; // number of characters in font.
    unsigned int m_LowestID; // basically m_Chars[0].id
    unsigned int m_FirstNonsequentialCharIndex; // first index that's not sequential from m_LowestID, allows for faster character lookups in simple cases(english).
    unsigned int m_NumKerningPairs;

    void ReadFont(const char* filebuffer, unsigned int buffersize);

public:
    BMFont(const char* filebuffer, unsigned int buffersize)
    {
        m_Initialized = false;

        m_ImageName[0] = 0;
        m_Chars = 0;
        m_KerningPairs = 0;
        m_NumChars = 0;
        m_LowestID = 0;
        m_FirstNonsequentialCharIndex = 0;
        m_NumKerningPairs = 0;

        ReadFont( filebuffer, buffersize );
    }

    virtual ~BMFont()
    {
        if( m_Chars )
            delete[] m_Chars;
        if( m_KerningPairs )
            delete[] m_KerningPairs;
    }

    char* QueryImageName() { return m_ImageName; }
    unsigned short QueryLineHeight() { return m_Properties.lineHeight; }
    Vector4 QueryLetterMinMaxUV(char letter);
    
    Vector2 GetSize(const char* string, float height);

    // return number of characters drawn.
    unsigned int GenerateVerts(const char* string, bool quads, Vertex_Sprite* verts, float height, unsigned char justificationflags = Justify_Top|Justify_Left);
    unsigned int GenerateVerts(const char* string, bool quads, Vertex_XYZUV_RGBA* verts, float height, unsigned char justificationflags, ColorByte color);
};

#endif //__BMFont_H__
