//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Math/Matrix4x4.h"
#include "../Core/Log.h"
#include <cstdio>

namespace Alimer
{
    const Matrix4x4 Matrix4x4::Zero(
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f);

    const Matrix4x4 Matrix4x4::Identity(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    String Matrix4x4::ToString() const
    {
        char tempBuffer[256];
        sprintf(tempBuffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
            m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34,
            m41, m42, m43, m44);
        return std::string(tempBuffer);
    }

    Matrix4x4 Matrix4x4::Transpose() const
    {
        Matrix4x4 result;
        Transpose(result);
        return result;
    }

    void Matrix4x4::Transpose(Matrix4x4& result) const
    {
#ifdef ALIMER_SSE2
        __m128 m0 = _mm_loadu_ps(&m11);
        __m128 m1 = _mm_loadu_ps(&m21);
        __m128 m2 = _mm_loadu_ps(&m31);
        __m128 m3 = _mm_loadu_ps(&m41);
        _MM_TRANSPOSE4_PS(m0, m1, m2, m3);
        _mm_storeu_ps(&result.m11, m0);
        _mm_storeu_ps(&result.m21, m1);
        _mm_storeu_ps(&result.m31, m2);
        _mm_storeu_ps(&result.m41, m3);
#else
        result.m11 = m11;
        result.m12 = m21;
        result.m13 = m31;
        result.m14 = m41;
        result.m21 = m12;
        result.m22 = m22;
        result.m23 = m32;
        result.m24 = m42;
        result.m31 = m13;
        result.m32 = m23;
        result.m33 = m33;
        result.m34 = m43;
        result.m41 = m14;
        result.m42 = m24;
        result.m43 = m34;
        result.m44 = m44;
#endif
    }

    Matrix4x4 Matrix4x4::Inverse() const
    {
        float v0 = m31 * m42 - m32 * m41;
        float v1 = m31 * m43 - m33 * m41;
        float v2 = m31 * m44 - m34 * m41;
        float v3 = m32 * m43 - m33 * m42;
        float v4 = m32 * m44 - m34 * m42;
        float v5 = m33 * m44 - m34 * m43;

        float i00 = (v5 * m22 - v4 * m23 + v3 * m24);
        float i10 = -(v5 * m21 - v2 * m23 + v1 * m24);
        float i20 = (v4 * m21 - v2 * m22 + v0 * m24);
        float i30 = -(v3 * m21 - v1 * m22 + v0 * m23);

        float invDet = 1.0f / (i00 * m11 + i10 * m21 + i20 * m13 + i30 * m14);

        i00 *= invDet;
        i10 *= invDet;
        i20 *= invDet;
        i30 *= invDet;

        float i01 = -(v5 * m12 - v4 * m13 + v3 * m14) * invDet;
        float i11 = (v5 * m11 - v2 * m13 + v1 * m14) * invDet;
        float i21 = -(v4 * m11 - v2 * m21 + v0 * m14) * invDet;
        float i31 = (v3 * m11 - v1 * m21 + v0 * m13) * invDet;

        v0 = m21 * m42 - m22 * m41;
        v1 = m21 * m43 - m23 * m41;
        v2 = m21 * m44 - m24 * m41;
        v3 = m22 * m43 - m23 * m42;
        v4 = m22 * m44 - m24 * m42;
        v5 = m23 * m44 - m24 * m43;

        float i02 = (v5 * m12 - v4 * m13 + v3 * m14) * invDet;
        float i12 = -(v5 * m11 - v2 * m13 + v1 * m14) * invDet;
        float i22 = (v4 * m11 - v2 * m12 + v0 * m14) * invDet;
        float i32 = -(v3 * m11 - v1 * m12 + v0 * m13) * invDet;

        v0 = m32 * m21 - m31 * m22;
        v1 = m33 * m21 - m31 * m13;
        v2 = m34 * m21 - m31 * m24;
        v3 = m33 * m22 - m32 * m23;
        v4 = m34 * m22 - m32 * m24;
        v5 = m34 * m23 - m33 * m24;

        float i03 = -(v5 * m12 - v4 * m13 + v3 * m14) * invDet;
        float i13 = (v5 * m11 - v2 * m13 + v1 * m14) * invDet;
        float i23 = -(v4 * m11 - v2 * m12 + v0 * m14) * invDet;
        float i33 = (v3 * m11 - v1 * m12 + v0 * m13) * invDet;

        return Matrix4x4(
            i00, i01, i02, i03,
            i10, i11, i12, i13,
            i20, i21, i22, i23,
            i30, i31, i32, i33);
    }

    Matrix4x4 Matrix4x4::CreateLookAt(const vec3 &eye, const vec3 &target, const vec3 &up)
    {
        vec3 const f(normalize(target - eye));
        vec3 const s(normalize(cross(f, up)));
        vec3 const u(cross(s, f));

        Matrix4x4 result;
        result.data[0][0] = s.x;
        result.data[1][0] = s.y;
        result.data[2][0] = s.z;
        result.data[0][1] = u.x;
        result.data[1][1] = u.y;
        result.data[2][1] = u.z;
        result.data[0][2] = -f.x;
        result.data[1][2] = -f.y;
        result.data[2][2] = -f.z;
        result.data[3][0] = -dot(s, eye);
        result.data[3][1] = -dot(u, eye);
        result.data[3][2] = dot(f, eye);
        return result;
    }

    Matrix4x4 Matrix4x4::CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
    {
        if (fieldOfView <= 0.0f || fieldOfView >= M_PI)
            ALIMER_LOGCRITICAL("Invalid fieldOfView");

        if (nearPlaneDistance <= 0.0f)
            ALIMER_LOGCRITICAL("nearPlaneDistance must be greather than 0");

        if (farPlaneDistance <= 0.0f)
            ALIMER_LOGCRITICAL("farPlaneDistance must be greather than 0");

        if (nearPlaneDistance >= farPlaneDistance)
            ALIMER_LOGCRITICAL("farPlaneDistance must be greather than nearPlaneDistance");

        const float tanHalfFovy = std::tan(fieldOfView / 2.0f);

        float yScale = 1.0f / tanHalfFovy;
        float xScale = yScale / aspectRatio;

        Matrix4x4 result(0.0f);
        result.m11 = xScale;

        result.m22 = yScale;

        float negFarRange = std::isnan(farPlaneDistance) ? -1.0f : farPlaneDistance / (nearPlaneDistance - farPlaneDistance);
        result.m33 = negFarRange;
        result.m34 = -1.0f;

        result.m43 = nearPlaneDistance * negFarRange;
        return result;
    }

    Matrix4x4 Matrix4x4::CreateRotationX(float radians)
    {
        float sinAngle, cosAngle;
        ScalarSinCos(radians, &sinAngle, &cosAngle);

        // [  1  0  0  0 ]
        // [  0  c  s  0 ]
        // [  0 -s  c  0 ]
        // [  0  0  0  1 ]
        Matrix4x4 result(1.0f);
        result.m22 = cosAngle;
        result.m23 = sinAngle;
        result.m32 = -sinAngle;
        result.m33 = cosAngle;
        return result;
    }

    Matrix4x4 Matrix4x4::CreateRotationY(float radians)
    {
        float sinAngle, cosAngle;
        ScalarSinCos(radians, &sinAngle, &cosAngle);

        // [  c  0 -s  0 ]
        // [  0  1  0  0 ]
        // [  s  0  c  0 ]
        // [  0  0  0  1 ]
        Matrix4x4 result(1.0f);
        result.m11 = cosAngle;
        result.m13 = -sinAngle;
        result.m31 = sinAngle;
        result.m33 = cosAngle;
        return result;
    }

    Matrix4x4 Matrix4x4::CreateRotationZ(float radians)
    {
        float sinAngle, cosAngle;
        ScalarSinCos(radians, &sinAngle, &cosAngle);
        
        // [  c  s  0  0 ]
        // [ -s  c  0  0 ]
        // [  0  0  1  0 ]
        // [  0  0  0  1 ]
        Matrix4x4 result(1.0f);
        result.m11 = cosAngle;
        result.m12 = sinAngle;
        result.m21 = -sinAngle;
        result.m22 = cosAngle;
        return result;
    }
}
