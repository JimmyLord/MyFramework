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

#ifndef __MyMatrix_H__
#define __MyMatrix_H__

class MyMatrix
{
public:
    float m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44;

public:
    MyMatrix() {}
    MyMatrix(const Vector3& right, const Vector3& up, const Vector3& at)
        : m11(right.x), m12(up.x), m13(at.x), m14(0)
        , m21(right.y), m22(up.y), m23(at.y), m24(0)
        , m31(right.z), m32(up.z), m33(at.z), m34(0)
        , m41(0),       m42(0),    m43(0),    m44(1) {}
    MyMatrix(float v11, float v12, float v13, float v14,
             float v21, float v22, float v23, float v24,
             float v31, float v32, float v33, float v34,
             float v41, float v42, float v43, float v44)
        : m11(v11), m12(v12), m13(v13), m14(v14)
        , m21(v21), m22(v22), m23(v23), m24(v24)
        , m31(v31), m32(v32), m33(v33), m34(v34)
        , m41(v41), m42(v42), m43(v43), m44(v44) {}

    void SetIdentity();
    void SetPosition(Vector3 pos);
    void SetPosition(float x, float y, float z);
    void SetFrustum(float left, float right, float bottom, float top, float nearZ, float farZ);
    void SetPerspective(float fovy, float aspect, float nearZ, float farZ);
    void SetOrtho(float left, float right, float bottom, float top, float nearZ, float farZ);
    void SetLookAtLH(const Vector3& eye, const Vector3& up, const Vector3& at);
    void SetLookAtRH(const Vector3& eye, const Vector3& up, const Vector3& at);

    void Multiply(MyMatrix* src);

    void Scale(float sx, float sy, float sz);
    void TranslatePreRotation(float tx, float ty, float tz);
    void TranslatePostRotation(float tx, float ty, float tz);
    void Rotate(float angle, float x, float y, float z);

    Vector2 TransformVector2(const Vector2& vec)
    {
        Vector4 result = TransformVector4( Vector4(vec.x, vec.y, 0, 1) );
        if( result.w )
            return Vector2( result.x/result.w, result.y/result.w );
        else
            return Vector2( result.x, result.y );
    }

    Vector3 TransformVector3(const Vector3& vec)
    {
        Vector4 result = TransformVector4( Vector4(vec.x, vec.y, vec.z, 1) );
        if( result.w )
            return Vector3( result.x/result.w, result.y/result.w, result.z/result.w );
        else
            return Vector3( result.x, result.y, result.z );
    }

    Vector4 TransformVector4(const Vector4& vec)
    {
        return Vector4( m11 * vec.x + m21 * vec.y + m31 * vec.z + m41 * vec.w,
                        m12 * vec.x + m22 * vec.y + m32 * vec.z + m42 * vec.w,
                        m13 * vec.x + m23 * vec.y + m33 * vec.z + m43 * vec.w,
                        m14 * vec.x + m24 * vec.y + m34 * vec.z + m44 * vec.w );
    }

    void Transpose()
    {
        float temp;

        temp = m12; m12 = m21; m21 = temp;
        temp = m13; m13 = m31; m31 = temp;
        temp = m14; m14 = m41; m41 = temp;
        temp = m23; m23 = m32; m32 = temp;
        temp = m24; m24 = m42; m42 = temp;
        temp = m34; m34 = m43; m43 = temp;
    }

    inline MyMatrix operator *(const float o) const
    {
        MyMatrix newmat;

        newmat.m11 = this->m11 * o;
        newmat.m12 = this->m12 * o;
        newmat.m13 = this->m13 * o;
        newmat.m14 = this->m14 * o;
        newmat.m21 = this->m21 * o;
        newmat.m22 = this->m22 * o;
        newmat.m23 = this->m23 * o;
        newmat.m24 = this->m24 * o;
        newmat.m31 = this->m31 * o;
        newmat.m32 = this->m32 * o;
        newmat.m33 = this->m33 * o;
        newmat.m34 = this->m34 * o;
        newmat.m41 = this->m41 * o;
        newmat.m42 = this->m42 * o;
        newmat.m43 = this->m43 * o;
        newmat.m44 = this->m44 * o;
    
        return newmat;
    }

    inline MyMatrix operator *(const MyMatrix o) const
    {
        MyMatrix newmat;

        newmat.m11 = this->m11 * o.m11 + this->m12 * o.m21 + this->m13 * o.m31 + this->m14 * o.m41;
        newmat.m12 = this->m11 * o.m12 + this->m12 * o.m22 + this->m13 * o.m32 + this->m14 * o.m42;
        newmat.m13 = this->m11 * o.m13 + this->m12 * o.m23 + this->m13 * o.m33 + this->m14 * o.m43;
        newmat.m14 = this->m11 * o.m14 + this->m12 * o.m24 + this->m13 * o.m34 + this->m14 * o.m44;
        newmat.m21 = this->m21 * o.m11 + this->m22 * o.m21 + this->m23 * o.m31 + this->m24 * o.m41;
        newmat.m22 = this->m21 * o.m12 + this->m22 * o.m22 + this->m23 * o.m32 + this->m24 * o.m42;
        newmat.m23 = this->m21 * o.m13 + this->m22 * o.m23 + this->m23 * o.m33 + this->m24 * o.m43;
        newmat.m24 = this->m21 * o.m14 + this->m22 * o.m24 + this->m23 * o.m34 + this->m24 * o.m44;
        newmat.m31 = this->m31 * o.m11 + this->m32 * o.m21 + this->m33 * o.m31 + this->m34 * o.m41;
        newmat.m32 = this->m31 * o.m12 + this->m32 * o.m22 + this->m33 * o.m32 + this->m34 * o.m42;
        newmat.m33 = this->m31 * o.m13 + this->m32 * o.m23 + this->m33 * o.m33 + this->m34 * o.m43;
        newmat.m34 = this->m31 * o.m14 + this->m32 * o.m24 + this->m33 * o.m34 + this->m34 * o.m44;
        newmat.m41 = this->m41 * o.m11 + this->m42 * o.m21 + this->m43 * o.m31 + this->m44 * o.m41;
        newmat.m42 = this->m41 * o.m12 + this->m42 * o.m22 + this->m43 * o.m32 + this->m44 * o.m42;
        newmat.m43 = this->m41 * o.m13 + this->m42 * o.m23 + this->m43 * o.m33 + this->m44 * o.m43;
        newmat.m44 = this->m41 * o.m14 + this->m42 * o.m24 + this->m43 * o.m34 + this->m44 * o.m44;
    
        return newmat;
    }

    bool Inverse(float tolerance = 0.0001f)
    {
        // Determinants of 2x2 submatrices
        float S0 = m11 * m22 - m12 * m21;
        float S1 = m11 * m23 - m13 * m21;
        float S2 = m11 * m24 - m14 * m21;
        float S3 = m12 * m23 - m13 * m22;
        float S4 = m12 * m24 - m14 * m22;
        float S5 = m13 * m24 - m14 * m23;

        float C5 = m33 * m44 - m34 * m43;
        float C4 = m32 * m44 - m34 * m42;
        float C3 = m32 * m43 - m33 * m42;
        float C2 = m31 * m44 - m34 * m41;
        float C1 = m31 * m43 - m33 * m41;
        float C0 = m31 * m42 - m32 * m41;

        // If determinant equals 0, there is no inverse
        float det = S0 * C5 - S1 * C4 + S2 * C3 + S3 * C2 - S4 * C1 + S5 * C0;
        if( fabs(det) <= tolerance )
            return false;

        // Compute adjugate matrix
        *this = MyMatrix(
             m22 * C5 - m23 * C4 + m24 * C3, -m12 * C5 + m13 * C4 - m14 * C3,
             m42 * S5 - m43 * S4 + m44 * S3, -m32 * S5 + m33 * S4 - m34 * S3,

            -m21 * C5 + m23 * C2 - m24 * C1,  m11 * C5 - m13 * C2 + m14 * C1,
            -m41 * S5 + m43 * S2 - m44 * S1,  m31 * S5 - m33 * S2 + m34 * S1,

             m21 * C4 - m22 * C2 + m24 * C0, -m11 * C4 + m12 * C2 - m14 * C0,
             m41 * S4 - m42 * S2 + m44 * S0, -m31 * S4 + m32 * S2 - m34 * S0,

            -m21 * C3 + m22 * C1 - m23 * C0,  m11 * C3 - m12 * C1 + m13 * C0,
            -m41 * S3 + m42 * S1 - m43 * S0,  m31 * S3 - m32 * S1 + m33 * S0 ) * (1 / det);

        return true;
    }
};

#endif //__MyMatrix_H__
