#ifndef __VMATH_H__
#define __VMATH_H__

#ifndef M_PI
#define M_PI   3.14159265358979323846f
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923f
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962f
#endif
#ifndef DEG_IN_RAD
#define DEG_IN_RAD (M_PI/180.0f)
#endif

#include <cmath>
#include <algorithm>

#include "common.h"

class Vector
{
public:
    friend inline Vector operator ^ (const Vector& first, const Vector& second);

    static const Vector Zero;
    static const Vector One;
    static const Vector X;
    static const Vector Y;
    static const Vector Z;

    union
    {
        float v[4];
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
    };
  
    Vector() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
    {
    }

    Vector(float x, float y, float z, float w=1.0f) : x(x), y(y), z(z), w(w) 
    {
    }

    Vector(const float* vec) : x(vec[0]), y(vec[1]), z(vec[2]), w(vec[3])
    {
    }

    Vector(const Vector& vec) : x(vec.x), y(vec.y), z(vec.z), w(vec.w)
    {
    }

    Vector& operator = (const float* vec)
    {
        x = vec[0];
        y = vec[1];
        z = vec[2];
        w = vec[3];
        return *this;
    }

    Vector& operator = (const Vector& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return *this;
    }
   
    Vector operator - () const
    {
        return Vector(-x, -y, -z, -w);
    }

    Vector operator + () const
    {
        return Vector(*this);
    }

    Vector& operator += (const Vector& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector& operator -= (const Vector& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    Vector& operator *= (float num)
    {
        x *= num;
        y *= num;
        z *= num;
        w *= num;
        return *this;
    }

    Vector& operator /= (float num)
    {
        if (num == 0.0f)
        {
            return *this;
        }
        else
        {
            return operator *= (1.0f/num);
        }
    }

    Vector& operator ^= (const Vector& other)
    {
        float x = y*other.z - z*other.y;
        float y = z*other.x - x*other.z;
        float z = x*other.y - y*other.x;
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = 1.0f;
        return *this;
    }
  
    float operator [] (size_t idx) const
    {
        return v[idx];
    }
  
    float& operator [] (size_t idx)
    {
        return v[idx];
    }

    float magnitude() const
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    float magnitude2() const
    {
        return x*x + y*y + z*z;
    }

    void norm()
    {
        float L = std::sqrt(x*x + y*y + z*z);
        if (L != 0.0f)
        {
            L = 1.0f / L;
            x *= L;
            y *= L;
            z *= L;
        }
    }
    
    float getRotationY()
    {
        return std::atan2(z, x);
    }
};

inline Vector operator + (const Vector& first, const Vector& second)
{
    return Vector(first.x+second.x, first.y+second.y, first.z+second.z);
}

inline Vector operator - (const Vector& first, const Vector& second) 
{
    return Vector(first.x-second.x, first.y-second.y, first.z-second.z);
}

inline Vector operator * (const Vector& vec, float num)
{
    return Vector(num*vec.x, num*vec.y, num*vec.z);
}

inline Vector operator * (float num, const Vector& vec)
{
    return Vector(num*vec.x, num*vec.y, num*vec.z);
}

inline Vector operator / (const Vector& vec, float num)
{
    if (num == 0.0f)
    {
        return vec;
    }
    else
    {
        const float tmp = 1.0f/num;
        return Vector(tmp*vec.x, tmp*vec.y, tmp*vec.z);
    }
}

inline float operator % (const Vector& first, const Vector& second)
{
    return first.x*second.x + first.y*second.y + first.z*second.z;
}
  
inline Vector operator ^ (const Vector& first, const Vector& second)
{
    return Vector(first.y*second.z - first.z*second.y, 
                  first.z*second.x - first.x*second.z, 
                  first.x*second.y - first.y*second.x);
}

inline bool operator == (const Vector& first, const Vector& second)
{
    return (first.x==second.x && first.y==second.y && first.z==second.z);
}
  
inline bool operator != (const Vector& first, const Vector& second)
{
    return (first.x!=second.x || first.y!=second.y || first.z!=second.z);
}

std::ostream& operator << (std::ostream& os, const Vector& vec);


class Matrix
{
public:
    friend Matrix operator * (const Matrix& first, const Matrix& second);

    union
    {
        float m[16];
        struct
        {
            float m00; float m01; float m02; float m03;
            float m10; float m11; float m12; float m13;
            float m20; float m21; float m22; float m23;
            float m30; float m31; float m32; float m33;
        };
    };
  
    Matrix()
    {
    }

    Matrix(float m00, float m01, float m02, float m03,
           float m10, float m11, float m12, float m13,
           float m20, float m21, float m22, float m23,
           float m30, float m31, float m32, float m33) :
        m00(m00), m01(m01), m02(m02), m03(m03), 
        m10(m10), m11(m11), m12(m12), m13(m13), 
        m20(m20), m21(m21), m22(m22), m23(m23), 
        m30(m30), m31(m31), m32(m32), m33(m33)
    {
    }

    Matrix(const float matrix[])
    {
        std::copy(matrix, matrix+16, m);
    }

    Matrix(const Matrix& other)
    {
        std::copy(other.m, other.m+16, m);
    }

    Matrix& operator = (const float* matrix)
    {
        std::copy(matrix, matrix+16, m);
        return *this;
    }
  
    Matrix& operator = (const Matrix& other)
    {
        std::copy(other.m, other.m+16, m);
        return *this;
    }

    Matrix& operator *= (const Matrix& other)
    {
        Matrix m(*this);
        return operator = (m*other);
    }

    float operator [] (int idx) const
    {
        return m[idx];
    }
  
    float& operator [] (int idx)
    {
        return m[idx];
    }

    static Matrix identity()
    {
        Matrix result;
        std::fill_n(result.m, 16, 0.0f);
        result.m00 = result.m11 = result.m22 = result.m33 = 1.0f;
        return result;
    }

    void transpose()
    {
        std::swap(m01, m10);
        std::swap(m02, m20);
        std::swap(m12, m21);
        std::swap(m30, m03);
        std::swap(m31, m13); 
        std::swap(m32, m23); 
    }
  
    void rotationX(float angle)
    {
        float s = std::sin(angle);
        float c = std::cos(angle);
        std::fill_n(m, 16, 0.0f);
        m00 = m33 = 1.0f;
        m11 = m22 = c;
        m12 = s;
        m21 = -s;
    }

    static Matrix rotateX(float angle)
    {
        Matrix result;
        result.rotationX(angle);
        return result;
    }

    void rotationY(float angle)
    {
        float s = std::sin(angle);
        float c = std::cos(angle);
        std::fill_n(m, 16, 0.0f);
        m11 = m33 = 1.0f;
        m00 = m22 = c;
        m20 = s;
        m02 = -s;
    }
  
    static Matrix rotateY(float angle)
    {
        Matrix result;
        result.rotationY(angle);
        return result;
    }

    void rotationZ(float angle)
    {
        float s = std::sin(angle);
        float c = std::cos(angle);
        std::fill_n(m, 16, 0.0f);
        m22 = m33 = 1.0f;
        m00 = m11 = c;
        m01 = s;
        m10 = -s;
    }
    
    static Matrix rotateZ(float angle)
    {
        Matrix result;
        result.rotationZ(angle);
        return result;
    }

    void rotation(float angle, const Vector& vec);
  
    void translation(const Vector& vec)
    {
        std::fill_n(m, 16, 0.0f);
        m00 = m11 = m22 = m33 = 1.0f;
        m30 = vec.x;
        m31 = vec.y;
        m32 = vec.z;
    }

    static Matrix translate(const Vector& vec)
    {
        Matrix result;
        result.translation(vec);
        return result;
    }

    void scaled(const Vector& vec)
    {
        std::fill_n(m, 16, 0.0f);
        m00 = vec.x;
        m11 = vec.y;
        m22 = vec.z;
        m33 = 1.0f;
    }

    static Matrix scale(const Vector& vec)
    {
        Matrix result;
        result.scaled(vec);
        return result;
    }

    Vector column(int idx) const
    {
        if (idx>=0 && idx<=3)
        {
            return Vector(m[0+idx], m[4+idx], m[8+idx], m[12+idx]);
        }
        return Vector();
    }
  
    Vector row(int idx) const
    {
        if (idx>=0 && idx<=3)
        {
            return Vector(m[idx*4+0], m[idx*4+1], m[idx*4+2], m[idx*4+3]);
        }
        return Vector();
    }
};

Vector lerp(const Vector& v1, const Vector& v2, float alpha);

Vector operator * (const Matrix& mx, const Vector& vec);
Vector operator * (const Vector& vec, const Matrix& mx);
Matrix operator * (const Matrix& first, const Matrix& second);

inline bool operator == (const Matrix& first, const Matrix& second)
{
    for (int i=0; i<16; i++)
    {
        if (first.m[i]!=second.m[i])
        {
            return false;
        }
    }
    return true;
}

inline bool operator != (const Matrix& first, const Matrix& second)
{
    for (int i=0; i<16; i++)
    {
        if (first.m[i]!=second.m[i])
        {
            return true;
        }
    }
    return false;
}

std::ostream& operator << (std::ostream& os, const Matrix& mx);

#endif
