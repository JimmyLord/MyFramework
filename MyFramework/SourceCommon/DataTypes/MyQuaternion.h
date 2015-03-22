//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __MyQuaternion_H__
#define __MyQuaternion_H__

class MyQuat
{
public:
    float x;
    float y;
    float z;
    float w;

public:
    MyQuat() {}
    MyQuat(Vector3 nv, float nw) { x = nv.x; y = nv.y; z = nv.z; w = nw; }
    MyQuat(Vector4 nv) { x = nv.x; y = nv.y; z = nv.z; w = nv.w; }
    MyQuat(float nx, float ny, float nz, float nw) { x = nx; y = ny; z = nz; w = nw; }

    inline void Set(float nx, float ny, float nz, float nw) { x = nx; y = ny; z = nz; w = nw; }
    inline float LengthSquared() const { return x*x + y*y + z*z + w*w; }
    inline float Length() const { return sqrtf(x*x + y*y + z*z + w*w); }

    inline MyQuat GetNormalized() const { float len = Length(); if( fequal(len,0) ) return MyQuat(x,y,z,w); len = 1.0f/len; return MyQuat(x*len, y*len, z*len, w*len);}
    inline MyQuat Normalize() { float len = Length(); if( !fequal(len,0) ) { x /= len; y /= len; z /= len; w /= len; } return *this; }
    inline float Dot(const MyQuat &o) const { return x*o.x + y*o.y + z*o.z + w*o.w; }
    inline MyQuat GetConjugate() { return MyQuat( -x, -y, -z, w ); }
    inline void Conjugate() { x *= -1; y *= -1; z *= -1; }
    inline MyQuat GetInverse() { return GetConjugate() * 1.0f/LengthSquared(); }
    inline void Inverse() { Conjugate(); *this *= 1.0f/LengthSquared(); }

    inline bool operator ==(const MyQuat& o) const { return fequal(this->x, o.x) && fequal(this->y, o.y) && fequal(this->z, o.z) && fequal(this->w, o.w); }
    inline bool operator !=(const MyQuat& o) const { return !fequal(this->x, o.x) || !fequal(this->y, o.y) || !fequal(this->z, o.z) || !fequal(this->w, o.w); }

    inline MyQuat operator *(const float o) const { return MyQuat(this->x * o, this->y * o, this->z * o, this->w * o); }
    inline MyQuat operator /(const float o) const { return MyQuat(this->x / o, this->y / o, this->z / o, this->w / o); }
    inline MyQuat operator +(const float o) const { return MyQuat(this->x + o, this->y + o, this->z + o, this->w + o); }
    inline MyQuat operator -(const float o) const { return MyQuat(this->x - o, this->y - o, this->z - o, this->w - o); }
    inline MyQuat operator +(const MyQuat& o) const { return MyQuat(this->x + o.x, this->y + o.y, this->z + o.z, this->w + o.w); }
    inline MyQuat operator -(const MyQuat& o) const { return MyQuat(this->x - o.x, this->y - o.y, this->z - o.z, this->w - o.w); }
    inline MyQuat operator *(const MyQuat& o) const
    {
        return MyQuat(
            w * o.x  +  x * o.w  +  y * o.z  -  z * o.y,
            w * o.y  +  y * o.w  +  z * o.x  -  x * o.z,
            w * o.z  +  z * o.w  +  x * o.y  -  y * o.x,
            w * o.w  -  x * o.x  -  y * o.y  -  z * o.z );
    }
    inline MyQuat operator /(const MyQuat& o) const { MyQuat temp = o; return *this * temp.GetInverse(); }

    inline MyQuat operator *=(const float o) { this->x *= o; this->y *= o; this->z *= o; this->w *= o; return *this; }
    inline MyQuat operator /=(const float o) { this->x /= o; this->y /= o; this->z /= o; this->w /= o; return *this; }
    inline MyQuat operator +=(const float o) { this->x += o; this->y += o; this->z += o; this->w += o; return *this; }
    inline MyQuat operator -=(const float o) { this->x -= o; this->y -= o; this->z -= o; this->w -= o; return *this; }
    inline MyQuat operator +=(const MyQuat& o) { this->x += o.x; this->y += o.y; this->z += o.z; this->w += o.w; return *this; }
    inline MyQuat operator -=(const MyQuat& o) { this->x -= o.x; this->y -= o.y; this->z -= o.z; this->w -= o.w; return *this; }

    static MyQuat Lerp(MyQuat start, MyQuat end, float perc);
    static MyQuat Slerp(MyQuat start, MyQuat end, float perc);
};

#endif //__MyQuaternion_H__
