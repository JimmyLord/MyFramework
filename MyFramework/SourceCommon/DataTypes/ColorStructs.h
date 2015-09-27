//
// Copyright (c) 2012-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ColorStructs_H__
#define __ColorStructs_H__

class ColorByte;
class ColorFloat;

class ColorFloat
{
public:
    float r;
    float g;
    float b;
    float a;

    static const ColorFloat White() { return ColorFloat( 1.0f, 1.0f, 1.0f, 1.0f ); }

    ColorFloat()
    {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
    }
    ColorFloat(float red, float green, float blue, float alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }
    ColorFloat(float rgb, float alpha)
    {
        r = g = b = rgb;
        a = alpha;
    }

    inline void Set(float red, float green, float blue, float alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }

    inline ColorByte AsColorByte() const;
    //{
    //    return ColorByte( (unsigned char)(this->r * 255.0f),
    //                      (unsigned char)(this->g * 255.0f),
    //                      (unsigned char)(this->b * 255.0f),
    //                      (unsigned char)(this->a * 255.0f) );
    //}

    inline ColorFloat operator *(const float o) const
    {
        return ColorFloat( this->r * o,
                           this->g * o,
                           this->b * o,
                           this->a * o );
    }

    inline ColorFloat operator *(const ColorFloat o) const
    {
        return ColorFloat( this->r * o.r,
                           this->g * o.g,
                           this->b * o.b,
                           this->a * o.a );
    }

    inline ColorFloat operator *=(const ColorFloat o)
    {
        this->r = this->r * o.r;
        this->g = this->g * o.g;
        this->b = this->b * o.b;
        this->a = this->a * o.a;

        return *this;
    }

    inline ColorFloat operator +=(const ColorFloat o)
    {
        this->r = this->r + o.r;
        this->g = this->g + o.g;
        this->b = this->b + o.b;
        this->a = this->a + o.a;

        return *this;
    }

    inline ColorFloat operator -=(const ColorFloat o)
    {
        this->r = this->r - o.r;
        this->g = this->g - o.g;
        this->b = this->b - o.b;
        this->a = this->a - o.a;

        return *this;
    }

    inline ColorFloat operator -=(const float o)
    {
        this->r = this->r - o;
        this->g = this->g - o;
        this->b = this->b - o;
        this->a = this->a - o;

        return *this;
    }

    inline bool operator ==(const ColorFloat o)
    {
        return fequal( this->r, o.r ) &&
               fequal( this->g, o.g ) &&
               fequal( this->b, o.b ) &&
               fequal( this->a, o.a );
    }

    inline bool operator !=(const ColorFloat o)
    {
        return !fequal( this->r, o.r ) ||
               !fequal( this->g, o.g ) ||
               !fequal( this->b, o.b ) ||
               !fequal( this->a, o.a );
    }
};

class ColorByte
{
public:
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    static const ColorByte White()  { return ColorByte( 255, 255, 255, 255 ); }
    static const ColorByte Black()  { return ColorByte(   0,   0,   0, 255 ); }
    static const ColorByte Red()    { return ColorByte( 255,   0,   0, 255 ); }
    static const ColorByte Green()  { return ColorByte(   0, 255,   0, 255 ); }
    static const ColorByte Blue()   { return ColorByte(   0,   0, 255, 255 ); }

    ColorByte()
    {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
    }
    ColorByte(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }

    inline void Set(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
    {
        r = red;
        g = green;
        b = blue;
        a = alpha;
    }

    inline ColorFloat AsColorFloat() const
    {
        return ColorFloat( this->r / 255.0f,
                           this->g / 255.0f,
                           this->b / 255.0f,
                           this->a / 255.0f );
    }

    inline ColorByte operator *(const float o) const
    {
        return ColorByte( (unsigned char)MyClamp_Return((float)(this->r * o), 0.0f, 255.0f),
                          (unsigned char)MyClamp_Return((float)(this->g * o), 0.0f, 255.0f),
                          (unsigned char)MyClamp_Return((float)(this->b * o), 0.0f, 255.0f),
                          (unsigned char)MyClamp_Return((float)(this->a * o), 0.0f, 255.0f) );
    }

    inline ColorByte operator *(const ColorByte o) const
    {
        return ColorByte( (unsigned char)((float)(this->r * o.r) / 255.0f),
                          (unsigned char)((float)(this->g * o.g) / 255.0f),
                          (unsigned char)((float)(this->b * o.b) / 255.0f),
                          (unsigned char)((float)(this->a * o.a) / 255.0f) );
    }

    inline ColorByte operator +(const ColorByte o) const
    {
        return ColorByte( (unsigned char)MyClamp_Return( (int)(this->r + o.r), 0, 255 ),
                          (unsigned char)MyClamp_Return( (int)(this->g + o.g), 0, 255 ),
                          (unsigned char)MyClamp_Return( (int)(this->b + o.b), 0, 255 ),
                          (unsigned char)MyClamp_Return( (int)(this->a + o.a), 0, 255 ) );
    }

    inline ColorByte operator -(const ColorByte o) const
    {
        return ColorByte( (unsigned char)MyClamp_Return( (int)(this->r - o.r), 0, 255 ),
                          (unsigned char)MyClamp_Return( (int)(this->g - o.g), 0, 255 ),
                          (unsigned char)MyClamp_Return( (int)(this->b - o.b), 0, 255 ),
                          (unsigned char)MyClamp_Return( (int)(this->a - o.a), 0, 255 ) );
    }

    inline ColorFloat operator *(const ColorFloat o) const
    {
        return ColorFloat( this->r / 255.0f * o.r,
                           this->g / 255.0f * o.g,
                           this->b / 255.0f * o.b,
                           this->a / 255.0f * o.a );
    }

    inline ColorByte operator *=(const ColorByte o)
    {
        this->r = (unsigned char)((float)(this->r * o.r) / 255.0f);
        this->g = (unsigned char)((float)(this->g * o.g) / 255.0f);
        this->b = (unsigned char)((float)(this->b * o.b) / 255.0f);
        this->a = (unsigned char)((float)(this->a * o.a) / 255.0f);

        return *this;
    }

    inline ColorByte operator +=(const ColorByte o)
    {
        this->r = (unsigned char)MyClamp_Return((float)(this->r + o.r), 0.0f, 255.0f);
        this->g = (unsigned char)MyClamp_Return((float)(this->g + o.g), 0.0f, 255.0f);
        this->b = (unsigned char)MyClamp_Return((float)(this->b + o.b), 0.0f, 255.0f);
        this->a = (unsigned char)MyClamp_Return((float)(this->a + o.a), 0.0f, 255.0f);

        return *this;
    }

    inline ColorByte operator -=(const ColorByte o)
    {
        this->r = (unsigned char)MyClamp_Return((float)(this->r - o.r), 0.0f, 255.0f);
        this->g = (unsigned char)MyClamp_Return((float)(this->g - o.g), 0.0f, 255.0f);
        this->b = (unsigned char)MyClamp_Return((float)(this->b - o.b), 0.0f, 255.0f);
        this->a = (unsigned char)MyClamp_Return((float)(this->a - o.a), 0.0f, 255.0f);

        return *this;
    }

    inline ColorByte operator -=(const unsigned char o)
    {
        this->r = (unsigned char)MyClamp_Return((float)(this->r - o), 0.0f, 255.0f);
        this->g = (unsigned char)MyClamp_Return((float)(this->g - o), 0.0f, 255.0f);
        this->b = (unsigned char)MyClamp_Return((float)(this->b - o), 0.0f, 255.0f);
        this->a = (unsigned char)MyClamp_Return((float)(this->a - o), 0.0f, 255.0f);

        return *this;
    }

    inline ColorByte operator *=(const float o)
    {
        this->r = (unsigned char)MyClamp_Return((float)(this->r * o), 0.0f, 255.0f);
        this->g = (unsigned char)MyClamp_Return((float)(this->g * o), 0.0f, 255.0f);
        this->b = (unsigned char)MyClamp_Return((float)(this->b * o), 0.0f, 255.0f);
        this->a = (unsigned char)MyClamp_Return((float)(this->a * o), 0.0f, 255.0f);

        return *this;
    }

    inline bool operator ==(const ColorByte o)
    {
        return this->r == o.r &&
               this->g == o.g &&
               this->b == o.b &&
               this->a == o.a;
    }

    inline bool operator !=(const ColorByte o)
    {
        return this->r != o.r ||
               this->g != o.g ||
               this->b != o.b ||
               this->a != o.a;
    }
};

inline ColorByte ColorFloat::AsColorByte() const
{
    return ColorByte( (unsigned char)(this->r * 255.0f),
                      (unsigned char)(this->g * 255.0f),
                      (unsigned char)(this->b * 255.0f),
                      (unsigned char)(this->a * 255.0f) );
}

#endif //__ColorStructs_H__
