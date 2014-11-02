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

void MyMatrix::SetIdentity()
{
    m12 = m13 = m14 = m21 = m23 = m24 = m31 = m32 = m34 = m41 = m42 = m43 = 0;
    m11 = m22 = m33 = m44 = 1;
}

void MyMatrix::SetPosition(Vector3 pos)
{
    m41 = pos.x;
    m42 = pos.y;
    m43 = pos.z;
}

void MyMatrix::SetPosition(float x, float y, float z)
{
    m41 = x;
    m42 = y;
    m43 = z;
}

void MyMatrix::Multiply(MyMatrix* src)
{
    MyMatrix tmp = *this;

    m11 = tmp.m11 * src->m11  +  tmp.m12 * src->m21  +  tmp.m13 * src->m31  +  tmp.m14 * src->m41;
    m12 = tmp.m11 * src->m12  +  tmp.m12 * src->m22  +  tmp.m13 * src->m32  +  tmp.m14 * src->m42;
    m13 = tmp.m11 * src->m13  +  tmp.m12 * src->m23  +  tmp.m13 * src->m33  +  tmp.m14 * src->m43;
    m14 = tmp.m11 * src->m14  +  tmp.m12 * src->m24  +  tmp.m13 * src->m34  +  tmp.m14 * src->m44;
                                                                                     
    m21 = tmp.m21 * src->m11  +  tmp.m22 * src->m21  +  tmp.m23 * src->m31  +  tmp.m24 * src->m41;
    m22 = tmp.m21 * src->m12  +  tmp.m22 * src->m22  +  tmp.m23 * src->m32  +  tmp.m24 * src->m42;
    m23 = tmp.m21 * src->m13  +  tmp.m22 * src->m23  +  tmp.m23 * src->m33  +  tmp.m24 * src->m43;
    m24 = tmp.m21 * src->m14  +  tmp.m22 * src->m24  +  tmp.m23 * src->m34  +  tmp.m24 * src->m44;

    m31 = tmp.m31 * src->m11  +  tmp.m32 * src->m21  +  tmp.m33 * src->m31  +  tmp.m34 * src->m41;
    m32 = tmp.m31 * src->m12  +  tmp.m32 * src->m22  +  tmp.m33 * src->m32  +  tmp.m34 * src->m42;
    m33 = tmp.m31 * src->m13  +  tmp.m32 * src->m23  +  tmp.m33 * src->m33  +  tmp.m34 * src->m43;
    m34 = tmp.m31 * src->m14  +  tmp.m32 * src->m24  +  tmp.m33 * src->m34  +  tmp.m34 * src->m44;

    m41 = tmp.m41 * src->m11  +  tmp.m42 * src->m21  +  tmp.m43 * src->m31  +  tmp.m44 * src->m41;
    m42 = tmp.m41 * src->m12  +  tmp.m42 * src->m22  +  tmp.m43 * src->m32  +  tmp.m44 * src->m42;
    m43 = tmp.m41 * src->m13  +  tmp.m42 * src->m23  +  tmp.m43 * src->m33  +  tmp.m44 * src->m43;
    m44 = tmp.m41 * src->m14  +  tmp.m42 * src->m24  +  tmp.m43 * src->m34  +  tmp.m44 * src->m44;
}

void MyMatrix::Scale(float sx, float sy, float sz)
{
    m11 *= sx; m12 *= sx; m13 *= sx; m14 *= sx;
    m21 *= sy; m22 *= sy; m23 *= sy; m24 *= sy;
    m31 *= sz; m32 *= sz; m33 *= sz; m34 *= sz;
}

void MyMatrix::TranslatePreRotation(float tx, float ty, float tz)
{
    m41 += m11 * tx  +  m21 * ty  +  m31 * tz;
    m42 += m12 * tx  +  m22 * ty  +  m32 * tz;
    m43 += m13 * tx  +  m23 * ty  +  m33 * tz;
    m44 += m14 * tx  +  m24 * ty  +  m34 * tz;
}

void MyMatrix::TranslatePostRotation(float tx, float ty, float tz)
{
    m41 += tx; m42 += ty; m43 += tz;
}

void MyMatrix::Rotate(float angle, float x, float y, float z)
{
    float sinAngle, cosAngle;
    float mag = sqrtf(x * x + y * y + z * z);
      
    sinAngle = sinf( angle * PI / 180.0f );
    cosAngle = cosf( angle * PI / 180.0f );
    if( mag > 0.0f )
    {
        float xx, yy, zz, xy, yz, zx, xs, ys, zs;
        float oneMinusCos;
   
        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        MyMatrix rotMat;
        rotMat.m11 = (oneMinusCos * xx) + cosAngle;
        rotMat.m12 = (oneMinusCos * xy) - zs;
        rotMat.m13 = (oneMinusCos * zx) + ys;
        rotMat.m14 = 0.0f; 

        rotMat.m21 = (oneMinusCos * xy) + zs;
        rotMat.m22 = (oneMinusCos * yy) + cosAngle;
        rotMat.m23 = (oneMinusCos * yz) - xs;
        rotMat.m24 = 0.0f;

        rotMat.m31 = (oneMinusCos * zx) - ys;
        rotMat.m32 = (oneMinusCos * yz) + xs;
        rotMat.m33 = (oneMinusCos * zz) + cosAngle;
        rotMat.m34 = 0.0f; 

        rotMat.m41 = 0.0f;
        rotMat.m42 = 0.0f;
        rotMat.m43 = 0.0f;
        rotMat.m44 = 1.0f;

        this->Multiply( &rotMat );
    }
}

void MyMatrix::SetFrustum(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float deltaX = right - left;
    float deltaY = top - bottom;
    float deltaZ = farZ - nearZ;

    assert( nearZ > 0.0f && farZ > 0.0f && deltaX > 0.0f && deltaY > 0.0f && deltaZ > 0.0f );

    m11 = 2.0f * nearZ / deltaX;
    m12 = m13 = m14 = 0.0f;

    m22 = 2.0f * nearZ / deltaY;
    m21 = m23 = m24 = 0.0f;

    m31 = (right + left) / deltaX;
    m32 = (top + bottom) / deltaY;
    m33 = -(nearZ + farZ) / deltaZ;
    m34 = -1.0f;

    m43 = -2.0f * nearZ * farZ / deltaZ;
    m41 = m42 = m44 = 0.0f;
}

void MyMatrix::SetPerspective(float fovy, float aspect, float nearZ, float farZ)
{
    GLfloat frustumW, frustumH;
   
    frustumH = tanf( fovy / 360.0f * PI ) * nearZ;
    frustumW = frustumH * aspect;

    SetFrustum( -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}

void MyMatrix::SetOrtho(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    float deltaX = (right - left);
    float deltaY = (top - bottom);
    float deltaZ = (farZ - nearZ);

    assert( (deltaX != 0.0f) && (deltaY != 0.0f) && (deltaZ != 0.0f) );

    m11 = 2.0f / deltaX;
    m12 = m13 = m14 = 0;

    m22 = 2.0f / deltaY;
    m21 = m23 = m24 = 0;
    
    m33 = -2.0f / deltaZ;
    m31 = m32 = m34 = 0;

    m41 = -(right + left) / deltaX;
    m42 = -(top + bottom) / deltaY;
    m43 = -(farZ + nearZ) / deltaZ;
    m44 = 1;
}

void MyMatrix::SetLookAtLH(const Vector3 &eye, const Vector3 &up, const Vector3 &at)
{
    Vector3 zaxis;
    Vector3 xaxis;
    Vector3 yaxis;

    zaxis = (at - eye).Normalize();
    xaxis = (up.Cross(zaxis)).Normalize();
    yaxis = zaxis.Cross(xaxis);

    *this = MyMatrix( xaxis, yaxis, zaxis );
}

void MyMatrix::SetLookAtRH(const Vector3 &eye, const Vector3 &up, const Vector3 &at)
{
    Vector3 zaxis = (eye - at).Normalize();
    Vector3 xaxis = (up.Cross(zaxis)).Normalize();
    Vector3 yaxis = zaxis.Cross(xaxis);

    *this = MyMatrix( xaxis, yaxis, zaxis );
    m41 = -xaxis.Dot(eye);
    m42 = -yaxis.Dot(eye);
    m43 = -zaxis.Dot(eye);

    //Vector3 zaxis;
    //Vector3 xaxis;
    //Vector3 yaxis;

    //zaxis = (eye - at).Normalize();
    //xaxis = (up.Cross(zaxis)).Normalize();
    //yaxis = zaxis.Cross(xaxis);

    //*this = MyMatrix( xaxis, yaxis, zaxis );

    //TranslatePreRotation( -eye.x, -eye.y, -eye.z );

    //Vector3 f = (at - eye).Normalize();
    //Vector3 u = up;
    //u.Normalize();
    //Vector3 s = (u.Cross(f)).Normalize();
    //u = f.Cross(s);

    //m11 =  s.x;
    //m21 =  s.y;
    //m31 =  s.z;
    //m12 =  u.x;
    //m22 =  u.y;
    //m32 =  u.z;
    //m13 = -f.x;
    //m23 = -f.y;
    //m33 = -f.z;
    //m41 = -s.Dot3(eye);
    //m42 = -u.Dot3(eye);
    //m43 = -f.Dot3(eye);
    //m44 = 1;
}
