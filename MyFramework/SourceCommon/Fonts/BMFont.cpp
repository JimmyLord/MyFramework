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

#include "BMFont.h"

unsigned int ReadNextBytesAsUInt(const char* ptr, unsigned int expected)
{
    const unsigned char c1 = *(ptr+0);
    const unsigned char c2 = *(ptr+1);
    const unsigned char c3 = *(ptr+2);
    const unsigned char c4 = *(ptr+3);

    unsigned int result = (unsigned int)(c4*256*256*256 + c3*256*256 + c2*256 + c1);

#if MYFW_WINDOWS && _DEBUG
    MyAssert( expected == result );
#endif
    return result;
}

unsigned short ReadNextBytesAsUShort(const char* ptr, unsigned short expected)
{
    const unsigned char c1 = *(ptr+0);
    const unsigned char c2 = *(ptr+1);

    unsigned short result = (unsigned short)(c2*256 + c1);

#if MYFW_WINDOWS && _DEBUG
    MyAssert( expected == result );
#endif
    return result;
}

short ReadNextBytesAsShort(const char* ptr, short expected)
{
    const unsigned char c1 = *(ptr+0);
    const char c2 = *(ptr+1);

    short result = (short)(c2<<8 | c1);

#if MYFW_WINDOWS && _DEBUG
    MyAssert( expected == result );
#endif
    return result;
}

void BMFont::ReadFont(const char* filebuffer, unsigned int buffersize)
{
    LOGInfo( LOGTag, "ReadFont - %d, %c%c%c%c\n", buffersize, filebuffer[0], filebuffer[1], filebuffer[2], filebuffer[3] );

    MyAssert( filebuffer[0] == 'B' );
    MyAssert( filebuffer[1] == 'M' );
    MyAssert( filebuffer[2] == 'F' );
    MyAssert( filebuffer[3] == 3 ); // This code only reads version 3 files... update it if using a newer BMFont format.

    const char* fileptr = filebuffer + 4;
    
    while( fileptr < filebuffer + buffersize )
    {
        //LOGInfo( LOGTag, "ReadFont - switch( %d )\n", (int)fileptr[0] );

        switch( fileptr[0] )
        {
        case 0: // end of file?
            {
                MyAssert( fileptr == filebuffer + buffersize - 1 );
                fileptr = filebuffer + buffersize;
            }
            break;

        case 1: // Block type 1: info
            {
                fileptr += 1; // skip past the block type.
                unsigned int blocksize = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                fileptr += 4; // skip past size;

                //LOGInfo( LOGTag, "ReadFont - blocksize = %d\n", (int)blocksize );

                // ignoring this block
                fileptr += blocksize;
            }
            break;

        case 2: //Block type 2: common
            {
                fileptr += 1; // skip past the block type.
                //unsigned int blocksize = *(unsigned int*)fileptr;
                fileptr += 4; // skip past size;

                m_Properties.lineHeight = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                fileptr += sizeof( m_Properties.lineHeight );

                m_Properties.base = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                fileptr += sizeof( m_Properties.base );

                m_Properties.scaleW = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                fileptr += sizeof( m_Properties.scaleW );

                m_Properties.scaleH = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                fileptr += sizeof( m_Properties.scaleH );

                m_Properties.pages = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                fileptr += sizeof( m_Properties.pages );

                unsigned char bitfields = *(unsigned char*)fileptr; // bits 0-6: reserved, bit 7: packed
                m_Properties.packed = (bitfields & 1<<7) > 0 ? true : false;
                fileptr += sizeof( bitfields );

                m_Properties.alphaChnl = *(unsigned char*)fileptr;
                fileptr += sizeof( m_Properties.alphaChnl );

                m_Properties.redChnl = *(unsigned char*)fileptr;
                fileptr += sizeof( m_Properties.redChnl );

                m_Properties.greenChnl = *(unsigned char*)fileptr;
                fileptr += sizeof( m_Properties.greenChnl );

                m_Properties.blueChnl = *(unsigned char*)fileptr;
                fileptr += sizeof( m_Properties.blueChnl );
            }
            break;

        case 3: // Block type 3: pages
            {
                fileptr += 1; // skip past the block type.
                unsigned int blocksize = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                fileptr += 4; // skip past size;

                // only readying the first file, ignoring others.
#if MYFW_NACL
                strcpy( m_ImageName, fileptr );
#else
                strcpy_s( m_ImageName, MAX_PATH, fileptr );
#endif
                fileptr += blocksize;
            }
            break;

        case 4: // Block type 4: chars
            {
                fileptr += 1; // skip past the block type.
                unsigned int blocksize = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                fileptr += 4; // skip past size;

                m_NumChars = blocksize / 20;

                m_Chars = MyNew CharDescriptor[m_NumChars];

                for( unsigned int i=0; i<m_NumChars; i++ )
                {
                    m_Chars[i].id = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                    fileptr += sizeof( m_Chars[i].id );

                    m_Chars[i].x = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                    fileptr += sizeof( m_Chars[i].x );

                    m_Chars[i].y = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                    fileptr += sizeof( m_Chars[i].y );

                    m_Chars[i].width = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                    fileptr += sizeof( m_Chars[i].width );

                    m_Chars[i].height = ReadNextBytesAsUShort( fileptr, *(unsigned short*)fileptr );
                    fileptr += sizeof( m_Chars[i].height );

                    m_Chars[i].xoffset = ReadNextBytesAsShort( fileptr, *(short*)fileptr );
                    fileptr += sizeof( m_Chars[i].xoffset );

                    m_Chars[i].yoffset = ReadNextBytesAsShort( fileptr, *(short*)fileptr );
                    fileptr += sizeof( m_Chars[i].yoffset );

                    m_Chars[i].xadvance = ReadNextBytesAsShort( fileptr, *(short*)fileptr );
                    fileptr += sizeof( m_Chars[i].xadvance );

                    m_Chars[i].page = *(unsigned char*)fileptr;
                    fileptr += sizeof( m_Chars[i].page );

                    m_Chars[i].chnl = *(unsigned char*)fileptr;
                    fileptr += sizeof( m_Chars[i].chnl );

                    //LOGInfo( LOGTag, "letter %c (%d, %d)\n", m_Chars[i].id, m_Chars[i].x, m_Chars[i].y );
                }

                m_LowestID = m_Chars[0].id;

                for( unsigned int i=0; i<m_NumChars; i++ )
                {
                    if( m_Chars[i].id != m_LowestID+i )
                    {
                        m_FirstNonsequentialCharIndex = i;
                        break;
                    }
                }
            }
            break;

        case 5: // Block type 5: kerning pairs
            {
                fileptr += 1; // skip past the block type.
                unsigned int blocksize = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                fileptr += 4; // skip past size;
            
                m_NumKerningPairs = blocksize / 10;

                m_KerningPairs = MyNew KerningPair[m_NumKerningPairs];

                for( unsigned int i=0; i<m_NumKerningPairs; i++ )
                {
                    m_KerningPairs[i].first = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                    fileptr += sizeof( m_KerningPairs[i].first );

                    m_KerningPairs[i].second = ReadNextBytesAsUInt( fileptr, *(unsigned int*)fileptr );
                    fileptr += sizeof( m_KerningPairs[i].second );

                    m_KerningPairs[i].amount = ReadNextBytesAsShort( fileptr, *(short*)fileptr );
                    fileptr += sizeof( m_KerningPairs[i].amount );
                }
            }
            break;

        default:
            MyAssert( false );
            LOGError( LOGTag, "Failed to load bmfont file %s\n", m_ImageName );
            return;
            break;
        }

        //LOGInfo( LOGTag, "ReadFont - fileptr = %d, filebuffer + buffersize = %d\n", (int)fileptr, (int)(filebuffer + buffersize) );
    }

    LOGInfo( LOGTag, "Font Initialized - %s with %d chars.\n", m_ImageName, m_NumChars );
    m_Initialized = true;
}

Vector4 BMFont::QueryLetterMinMaxUV(char letter)
{
    // quickly look up character array element.
    int index = -1;
    if( letter == 10 )
    {
        return Vector4(0,0,0,0);
    }
    else
    {
        if( m_FirstNonsequentialCharIndex == 0 ||
            ( (letter - (char)m_LowestID) >= 0 && (letter - m_LowestID) < m_FirstNonsequentialCharIndex ) )
        {
            index = letter - m_LowestID;
        }

        // if we didn't find it quick... search for it the slow way.
        if( index <= -1 )
        {
            MyAssert( m_FirstNonsequentialCharIndex != 0 );
            for( unsigned int ch=m_FirstNonsequentialCharIndex; ch<m_NumChars; ch++ )
            {
                if( m_Chars[ch].id == (unsigned int)letter )
                {
                    index = ch;
                    break;
                }
            }
        }

        if( index <= -1 )
            return Vector4(0,0,0,0);
    }

    CharDescriptor* pChar = &m_Chars[index];

    float pagew = m_Properties.scaleW;
    float pageh = m_Properties.scaleH;
    float charx = pChar->x;
    float chary = pChar->y;
    float charwidth = pChar->width;
    float charheight = pChar->height;

    float uleft = charx / pagew;
    float uright = (charx + charwidth) / pagew;
    float vtop = chary / pageh;
    float vbottom = (chary + charheight) / pageh;

    return Vector4( uleft, uright, vtop, vbottom );
}

Vector2 BMFont::GetSize(const char* string, float height)
{
    float biggest_x = 0;

    float current_x = 0;
    float current_y = 0;

    float yadvance = height * 0.8f;

    float scale = height/m_Properties.lineHeight;
    if( height == -1 )
        scale = 1;

    for( unsigned int i=0; i<strlen(string); i++ )
    {
        // quickly look up character array element.
        char letter = string[i];

        int index = -1;
        if( letter == 10 )
        {
            current_y -= yadvance;
            current_x = 0;
            continue;
        }
        else
        {
            if( m_FirstNonsequentialCharIndex == 0 ||
                ( (letter - (char)m_LowestID) >= 0 && (letter - m_LowestID) < m_FirstNonsequentialCharIndex ) )
            {
                index = letter - m_LowestID;
            }

            // if we didn't find it quick... search for it the slow way.
            if( index <= -1 )
            {
                MyAssert( m_FirstNonsequentialCharIndex != 0 );
                for( unsigned int ch=m_FirstNonsequentialCharIndex; ch<m_NumChars; ch++ )
                {
                    if( m_Chars[ch].id == (unsigned int)letter )
                    {
                        index = ch;
                        break;
                    }
                }
            }

            if( index <= -1 )
                continue;
        }

        CharDescriptor* pChar = &m_Chars[index];

        current_x += pChar->xadvance * scale;

        if( current_x > biggest_x )
            biggest_x = current_x;
    }

    current_y -= yadvance;

    return Vector2( biggest_x, -current_y );
}

unsigned int BMFont::GenerateVerts(const char* string, bool quads, Vertex_Sprite* verts, float height, GLenum glmode, unsigned char justificationflags)
{
    float biggest_x = 0;

    float current_x = 0;
    float current_y = 0;

    unsigned int lettercount = 0;

    for( unsigned int i=0; i<strlen(string); i++ )
    {
        // quickly look up character array element.
        char letter = string[i];

        int index = -1;
        if( letter == 10 )
        {
            current_y -= height * 0.8f;// * scale;
            current_x = 0;
            continue;
        }
        else
        {
            if( m_FirstNonsequentialCharIndex == 0 ||
                ( (letter - (char)m_LowestID) >= 0 && (letter - m_LowestID) < m_FirstNonsequentialCharIndex ) )
            {
                index = letter - m_LowestID;
            }

            // if we didn't find it quick... search for it the slow way.
            if( index <= -1 )
            {
                MyAssert( m_FirstNonsequentialCharIndex != 0 );
                for( unsigned int ch=m_FirstNonsequentialCharIndex; ch<m_NumChars; ch++ )
                {
                    if( m_Chars[ch].id == (unsigned int)letter )
                    {
                        index = ch;
                        break;
                    }
                }
            }

            if( index <= -1 )
                continue;
        }

        CharDescriptor* pChar = &m_Chars[index];

        float scale = height/m_Properties.lineHeight;
        if( height == -1 )
            scale = 1;
        
        float pagescalew = m_Properties.scaleW * scale;
        float pagescaleh = m_Properties.scaleH * scale;
        float charx = pChar->x * scale;
        float chary = pChar->y * scale;
        float charwidth = pChar->width * scale;
        float charheight = pChar->height * scale;
        float charxoffset = pChar->xoffset * scale;
        float charyoffset = pChar->yoffset * scale;

        //float uleft = (charx + 0.5f) / pagescalew;
        //float uright = ((charx + charwidth) - 0.5f) / pagescalew;
        //float vtop = (chary + 0.5f) / pagescaleh;
        //float vbottom = ((chary + charheight) - 0.5f) / pagescaleh;
        float uleft = charx / pagescalew;
        float uright = (charx + charwidth) / pagescalew;
        float vtop = chary / pagescaleh;
        float vbottom = (chary + charheight) / pagescaleh;

        // default justification is top/left
        float xleft = current_x + charxoffset;
        float xright = current_x + charxoffset + charwidth;
        float ytop = current_y - charyoffset;
        float ybottom = current_y - charyoffset - charheight;

        if( justificationflags & Justify_Bottom )
        {
            ytop += (float)m_Properties.lineHeight * scale;
            ybottom += (float)m_Properties.lineHeight * scale;
        }
        else if( justificationflags & Justify_CenterY )
        {
            ytop += ((float)m_Properties.lineHeight * scale) / 2;
            ybottom += ((float)m_Properties.lineHeight * scale) / 2;
        }

        if( quads == true )
        {
            // upper left
            verts[lettercount*4].x = xleft;
            verts[lettercount*4].y = ytop;
            verts[lettercount*4].u = uleft;
            verts[lettercount*4].v = vtop;

            // upper right
            verts[lettercount*4+1].x = xright;
            verts[lettercount*4+1].y = ytop;
            verts[lettercount*4+1].u = uright;
            verts[lettercount*4+1].v = vtop;

            // lower right
            verts[lettercount*4+2].x = xright;
            verts[lettercount*4+2].y = ybottom;
            verts[lettercount*4+2].u = uright;
            verts[lettercount*4+2].v = vbottom;

            // lower left
            verts[lettercount*4+3].x = xleft;
            verts[lettercount*4+3].y = ybottom;
            verts[lettercount*4+3].u = uleft;
            verts[lettercount*4+3].v = vbottom;
        }
        else //if( quads == false )
        {
            // upper left
            verts[lettercount*6].x = xleft;
            verts[lettercount*6].y = ytop;
            verts[lettercount*6].u = uleft;
            verts[lettercount*6].v = vtop;

            // lower left
            verts[lettercount*6+1].x = xleft;
            verts[lettercount*6+1].y = ybottom;
            verts[lettercount*6+1].u = uleft;
            verts[lettercount*6+1].v = vbottom;

            // upper right
            verts[lettercount*6+2].x = xright;
            verts[lettercount*6+2].y = ytop;
            verts[lettercount*6+2].u = uright;
            verts[lettercount*6+2].v = vtop;

            // lower left
            verts[lettercount*6+3].x = xleft;
            verts[lettercount*6+3].y = ybottom;
            verts[lettercount*6+3].u = uleft;
            verts[lettercount*6+3].v = vbottom;

            // lower right
            verts[lettercount*6+4].x = xright;
            verts[lettercount*6+4].y = ybottom;
            verts[lettercount*6+4].u = uright;
            verts[lettercount*6+4].v = vbottom;

            // upper right
            verts[lettercount*6+5].x = xright;
            verts[lettercount*6+5].y = ytop;
            verts[lettercount*6+5].u = uright;
            verts[lettercount*6+5].v = vtop;
        }

        current_x += pChar->xadvance * scale;

        if( current_x > biggest_x )
            biggest_x = current_x;

        lettercount++;
    }

    float justify_x_offset = 0;

    if( justificationflags & Justify_Right )
    {
        justify_x_offset = biggest_x;
    }
    else if( justificationflags & Justify_CenterX )
    {
        justify_x_offset = biggest_x / 2;
    }

    if( justify_x_offset != 0 )
    {
        for( unsigned int i=0; i<lettercount; i++ )
        {
            int numverts = 6;
            if( quads == true )
            {
                numverts = 4;
            }
            for( int v=0; v<numverts; v++ )
            {
                verts[i*numverts + v].x -= justify_x_offset;
            }
        }
    }
    
    return lettercount;
}

unsigned int BMFont::GenerateVerts(const char* string, bool quads, Vertex_XYZUV_RGBA* verts, float height, GLenum glmode, unsigned char justificationflags, ColorByte color)
{
    float biggest_x = 0;

    float current_x = 0;
    float current_y = 0;

    unsigned int lettercount = 0;

    for( unsigned int i=0; i<strlen(string); i++ )
    {
        // quickly look up character array element.
        char letter = string[i];

        int index = -1;
        if( letter == 10 )
        {
            current_y -= height * 0.8f;// * scale;
            current_x = 0;
            continue;
        }
        else
        {
            if( m_FirstNonsequentialCharIndex == 0 ||
                ( (letter - (char)m_LowestID) >= 0 && (letter - m_LowestID) < m_FirstNonsequentialCharIndex ) )
            {
                index = letter - m_LowestID;
            }

            // if we didn't find it quick... search for it the slow way.
            if( index <= -1 )
            {
                MyAssert( m_FirstNonsequentialCharIndex != 0 );
                for( unsigned int ch=m_FirstNonsequentialCharIndex; ch<m_NumChars; ch++ )
                {
                    if( m_Chars[ch].id == (unsigned int)letter )
                    {
                        index = ch;
                        break;
                    }
                }
            }

            if( index <= -1 )
                continue;
        }

        CharDescriptor* pChar = &m_Chars[index];

        float scale = height/m_Properties.lineHeight;
        if( height == -1 )
            scale = 1;
        
        if( letter != 32 )
        {
            float pagescalew = m_Properties.scaleW * scale;
            float pagescaleh = m_Properties.scaleH * scale;
            float charx = pChar->x * scale;
            float chary = pChar->y * scale;
            float charwidth = pChar->width * scale;
            float charheight = pChar->height * scale;
            float charxoffset = pChar->xoffset * scale;
            float charyoffset = pChar->yoffset * scale;

            //float uleft = (charx + 0.5f) / pagescalew;
            //float uright = ((charx + charwidth) - 0.5f) / pagescalew;
            //float vtop = (chary + 0.5f) / pagescaleh;
            //float vbottom = ((chary + charheight) - 0.5f) / pagescaleh;
            float uleft = charx / pagescalew;
            float uright = (charx + charwidth) / pagescalew;
            float vtop = chary / pagescaleh;
            float vbottom = (chary + charheight) / pagescaleh;

            // default justification is top/left
            float xleft = current_x + charxoffset;
            float xright = current_x + charxoffset + charwidth;
            float ytop = current_y - charyoffset;
            float ybottom = current_y - charyoffset - charheight;

            if( justificationflags & Justify_Bottom )
            {
                ytop += (float)m_Properties.lineHeight * scale;
                ybottom += (float)m_Properties.lineHeight * scale;
            }
            else if( justificationflags & Justify_CenterY )
            {
                ytop += ((float)m_Properties.lineHeight * scale) / 2;
                ybottom += ((float)m_Properties.lineHeight * scale) / 2;
            }

            if( quads == true )
            {
                // upper left
                verts[lettercount*4].x = xleft;
                verts[lettercount*4].y = ytop;
                verts[lettercount*4].u = uleft;
                verts[lettercount*4].v = vtop;

                // upper right
                verts[lettercount*4+1].x = xright;
                verts[lettercount*4+1].y = ytop;
                verts[lettercount*4+1].u = uright;
                verts[lettercount*4+1].v = vtop;

                // lower right
                verts[lettercount*4+2].x = xright;
                verts[lettercount*4+2].y = ybottom;
                verts[lettercount*4+2].u = uright;
                verts[lettercount*4+2].v = vbottom;

                // lower left
                verts[lettercount*4+3].x = xleft;
                verts[lettercount*4+3].y = ybottom;
                verts[lettercount*4+3].u = uleft;
                verts[lettercount*4+3].v = vbottom;
            }
            else //if( quads == false )
            {
                // upper left
                verts[lettercount*6].x = xleft;
                verts[lettercount*6].y = ytop;
                verts[lettercount*6].u = uleft;
                verts[lettercount*6].v = vtop;

                // lower left
                verts[lettercount*6+1].x = xleft;
                verts[lettercount*6+1].y = ybottom;
                verts[lettercount*6+1].u = uleft;
                verts[lettercount*6+1].v = vbottom;

                // upper right
                verts[lettercount*6+2].x = xright;
                verts[lettercount*6+2].y = ytop;
                verts[lettercount*6+2].u = uright;
                verts[lettercount*6+2].v = vtop;

                // lower left
                verts[lettercount*6+3].x = xleft;
                verts[lettercount*6+3].y = ybottom;
                verts[lettercount*6+3].u = uleft;
                verts[lettercount*6+3].v = vbottom;

                // lower right
                verts[lettercount*6+4].x = xright;
                verts[lettercount*6+4].y = ybottom;
                verts[lettercount*6+4].u = uright;
                verts[lettercount*6+4].v = vbottom;

                // upper right
                verts[lettercount*6+5].x = xright;
                verts[lettercount*6+5].y = ytop;
                verts[lettercount*6+5].u = uright;
                verts[lettercount*6+5].v = vtop;
            }

            lettercount++;
        }

        current_x += pChar->xadvance * scale;

        if( current_x > biggest_x )
            biggest_x = current_x;
    }

    float justify_x_offset = 0;

    if( justificationflags & Justify_Right )
    {
        justify_x_offset = biggest_x;
    }
    else if( justificationflags & Justify_CenterX )
    {
        justify_x_offset = biggest_x / 2;
    }

    if( justify_x_offset != 0 )
    {
        for( unsigned int i=0; i<lettercount; i++ )
        {
            int numverts = 6;
            if( quads == true )
            {
                numverts = 4;
            }
            for( int v=0; v<numverts; v++ )
            {
                verts[i*numverts+v].x -= justify_x_offset;
            }
        }
    }

    for( unsigned int i=0; i<lettercount; i++ )
    {
        int numverts = 6;
        if( quads == true )
        {
            numverts = 4;
        }
        for( int v=0; v<numverts; v++ )
        {
            verts[i*numverts+v].z = 0;
            verts[i*numverts+v].r = color.r;
            verts[i*numverts+v].g = color.g;
            verts[i*numverts+v].b = color.b;
            verts[i*numverts+v].a = color.a;
        }
    }

    return lettercount;
}
