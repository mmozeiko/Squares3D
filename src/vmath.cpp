#include <ostream>

#include "vmath.h"

const Vector Vector::Zero(0.0f, 0.0f, 0.0f, 0.0f);
const Vector Vector::One(1.0f, 1.0f, 1.0f, 1.0f);
const Vector Vector::X(1.0f, 0.0f, 0.0f, 0.0f);
const Vector Vector::Y(0.0f, 1.0f, 0.0f, 0.0f);
const Vector Vector::Z(0.0f, 0.0f, 1.0f, 0.0f);

std::ostream& operator << (std::ostream& os, const Vector& vec)
{
    return os << "Vector(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
}

Vector lerp(const Vector& v1, const Vector& v2, float alpha)
{
    return v1 * (1.0f - alpha) + v2 * alpha;
}

void Matrix::rotation(float angle, const Vector& vec)
{
    float s = std::sin(angle);
    float c = std::cos(angle);
    float c1 = 1-c;
    float x = vec.x;
    float y = vec.y;
    float z = vec.z;
    m00 =    c + x*x*c1;
    m01 = -z*s + x*y*c1;
    m02 =  y*s + x*z*c1;

    m10 =  z*s + y*x*c1;
    m11 =    c + y*y*c1;
    m12 = -x*s + y*z*c1;

    m20 = -y*s + z*x*c1;
    m21 =  x*s + z*y*c1;
    m22 =    c + z*z*c1;

    m03=m13=m23=m30=m31=m32=0.0f;
    m33=1.0f;
}


Vector operator * (const Matrix& mx, const Vector& vec)
{
    return Vector(mx.m00*vec.x + mx.m10*vec.y + mx.m20*vec.z + mx.m30,
                  mx.m01*vec.x + mx.m11*vec.y + mx.m21*vec.z + mx.m31,
                  mx.m02*vec.x + mx.m12*vec.y + mx.m22*vec.z + mx.m32);
}

Vector operator * (const Vector& vec, const Matrix& mx)
{
    return Vector(mx.m00*vec.x + mx.m01*vec.x + mx.m02*vec.x,
                  mx.m10*vec.y + mx.m11*vec.y + mx.m12*vec.y,
                  mx.m20*vec.z + mx.m21*vec.z + mx.m22*vec.z);
}

Matrix operator * (const Matrix& first, const Matrix& second)
{
    Matrix m;
    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {
            float sum = 0.0f;
            for (int k=0; k<4; k++)
            {
                sum += second.m[i*4+k] * first.m[k*4+j];
            }
            m.m[i*4+j] = sum;
        }
    }
/*
    m.m00 = first.m00*second.m00 + first.m10*second.m01 + first.m20*second.m02;
    m.m01 = first.m00*second.m10 + first.m10*second.m11 + first.m20*second.m12;
    m.m02 = first.m00*second.m20 + first.m10*second.m21 + first.m20*second.m22;
    m.m03 = first.m00*second.m30 + first.m10*second.m31 + first.m20*second.m32 + first.m33;

    m.m10 = first.m10*second.m00 + first.m11*second.m10 + first.m12*second.m20;
    m.m11 = first.m10*second.m01 + first.m11*second.m11 + first.m12*second.m21;
    m.m12 = first.m10*second.m02 + first.m11*second.m12 + first.m12*second.m22;
    m.m13 = first.m10*second.m03 + first.m11*second.m13 + first.m12*second.m23 + first.m13;

    m.m20 = first.m20*second.m00 + first.m21*second.m10 + first.m22*second.m20;
    m.m21 = first.m20*second.m01 + first.m21*second.m11 + first.m22*second.m21;
    m.m22 = first.m20*second.m02 + first.m21*second.m12 + first.m22*second.m22;
    m.m23 = first.m20*second.m03 + first.m21*second.m13 + first.m22*second.m23 + first.m23;
*/
    return m;
}

std::ostream& operator << (std::ostream& os, const Matrix& mx)
{
    os << "Matrix(";
    for (int i=0; i<15; i++)
    {
        os << mx.m[i] << ", ";
    }
    os << mx.m[15] << ")";
    return os;
}
