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

void MyMatrix::SetTranslation(Vector3 pos)
{
    m41 = pos.x;
    m42 = pos.y;
    m43 = pos.z;
}

void MyMatrix::SetTranslation(float x, float y, float z)
{
    m41 = x;
    m42 = y;
    m43 = z;
}

void MyMatrix::CreateSRT(float scale, Vector3 rot, Vector3 pos)
{
    SetIdentity();
    Scale( scale );
    Rotate( rot.z, 0, 0, 1 ); // roll
    Rotate( rot.x, 1, 0, 0 ); // yaw
    Rotate( rot.y, 0, 1, 0 ); // pitch
    Translate( pos.x, pos.y, pos.z );
}

void MyMatrix::CreateSRT(Vector3 scale, Vector3 rot, Vector3 pos)
{
    SetIdentity();
    Scale( scale.x, scale.y, scale.z );
    Rotate( rot.z, 0, 0, 1 ); // roll
    Rotate( rot.x, 1, 0, 0 ); // yaw
    Rotate( rot.y, 0, 1, 0 ); // pitch
    Translate( pos.x, pos.y, pos.z );
}

void MyMatrix::Scale(float scale)
{
    m11 *= scale; m21 *= scale; m31 *= scale; m41 *= scale;
    m12 *= scale; m22 *= scale; m32 *= scale; m42 *= scale;
    m13 *= scale; m23 *= scale; m33 *= scale; m43 *= scale;
}

void MyMatrix::Scale(float sx, float sy, float sz)
{
    m11 *= sx; m21 *= sx; m31 *= sx; m41 *= sx;
    m12 *= sy; m22 *= sy; m32 *= sy; m42 *= sy;
    m13 *= sz; m32 *= sz; m33 *= sz; m43 *= sz;
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

        *this = rotMat * *this;
    }
}

void MyMatrix::Translate(Vector3 pos)
{
    m41 += pos.x;
    m42 += pos.y;
    m43 += pos.z;
}

void MyMatrix::Translate(float x, float y, float z)
{
    m41 += x;
    m42 += y;
    m43 += z;
}

void MyMatrix::CreateFrustum(float left, float right, float bottom, float top, float nearZ, float farZ)
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

void MyMatrix::CreatePerspectiveVFoV(float halfvertfovdegrees, float aspect, float nearZ, float farZ)
{
    GLfloat frustumRight, frustumTop;
   
    frustumTop = tanf( halfvertfovdegrees / 360.0f * PI ) * nearZ;
    frustumRight = frustumTop * aspect;

    CreateFrustum( -frustumRight, frustumRight, -frustumTop, frustumTop, nearZ, farZ );
}

void MyMatrix::CreatePerspectiveHFoV(float halfhorfovdegrees, float aspect, float nearZ, float farZ)
{
    GLfloat frustumRight, frustumTop;
   
    frustumRight = tanf( halfhorfovdegrees / 360.0f * PI ) * nearZ;
    frustumTop = frustumRight / aspect;

    CreateFrustum( -frustumRight, frustumRight, -frustumTop, frustumTop, nearZ, farZ );
}

void MyMatrix::CreateOrtho(float left, float right, float bottom, float top, float nearZ, float farZ)
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

void MyMatrix::CreateLookAtLeftHanded(const Vector3 &eye, const Vector3 &up, const Vector3 &at)
{
    Vector3 zaxis;
    Vector3 xaxis;
    Vector3 yaxis;

    zaxis = (at - eye).Normalize();
    xaxis = (up.Cross(zaxis)).Normalize();
    yaxis = zaxis.Cross(xaxis);

    *this = MyMatrix( xaxis, yaxis, zaxis );
}

void MyMatrix::CreateLookAt(const Vector3 &eye, const Vector3 &up, const Vector3 &at)
{
    Vector3 zaxis = (eye - at).Normalize();
    Vector3 xaxis = (up.Cross(zaxis)).Normalize();
    Vector3 yaxis = zaxis.Cross(xaxis);

    *this = MyMatrix( xaxis, yaxis, zaxis );
    m41 = -xaxis.Dot(eye);
    m42 = -yaxis.Dot(eye);
    m43 = -zaxis.Dot(eye);
}
