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

#include <cmath>
#include <algorithm>
#include <iostream>

class Vector
{
public:
    friend inline Vector operator ^ (const Vector& first, const Vector& second);

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
  
    Vector(const float x=0.0f, const float y=0.0f, const float z=0.0f) : 
        x(x), y(y), z(z), w(1.0f) 
    {
    }

    Vector(const float vec[]) : 
        x(vec[0]), y(vec[1]), z(vec[2]), w(1.0f)
    {
    }

    Vector(const Vector& vec) : 
        x(vec.x), y(vec.y), z(vec.z), w(1.0f)
    {
    }

    Vector& operator = (const float vec[])
    {
        x = vec[0];
        y = vec[1];
        z = vec[2];
        return *this;
    }

    Vector& operator = (const Vector& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
   
    Vector operator - () const
    {
        return Vector(-x, -y, -z);
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
        return *this;
    }

    Vector& operator -= (const Vector& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector& operator *= (const float num)
    {
        x *= num;
        y *= num;
        z *= num;
        return *this;
    }

    Vector& operator /= (const float num)
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
        Vector tmp(*this);
        return operator = (tmp^other);
    }
  
    float operator [] (const int idx) const
    {
        return v[idx];
    }
  
    float& operator [] (const int idx)
    {
        return v[idx];
    }

    float len() const
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    void norm()
    {
        float L = std::sqrt(x*x + y*y + z*z);
        if (L!=0.0f) {
            L = 1.0f/L;
            x *= L;
            y *= L;
            z *= L;
        }
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

inline Vector operator * (const Vector& vec, const float num)
{
    return Vector(num*vec.x, num*vec.y, num*vec.z);
}

inline Vector operator * (const float num, const Vector& vec)
{
    return Vector(num*vec.x, num*vec.y, num*vec.z);
}

inline Vector operator / (const Vector& vec, const float num)
{
    if (num == 0.0f)
    {
        return vec;
    }
    else
    {
        float tmp = 1.0f/num;
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

inline std::ostream& operator << (std::ostream& os, const Vector& vec)
{
    return os << "Vector(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
}

class Matrix
{
public:
    friend inline Matrix operator * (const Matrix& first, const Matrix& second);

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
           float m20, float m21, float m22, float m23) :
        m00(m00), m01(m01), m02(m02), m03(m03), 
        m10(m10), m11(m11), m12(m12), m13(m13), 
        m20(m20), m21(m21), m22(m22), m23(m23), 
        m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
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

    Matrix& operator = (const float matrix[])
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

    void rotation(float angle, const Vector& vec)
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
        if (idx>=0 && idx<=2)
        {
            return Vector(m[0+idx], m[4+idx], m[8+idx]);
        }
        return Vector();
    }
  
    Vector row(int idx) const
    {
        if (idx>=0 && idx<=3)
        {
            return Vector(m[idx*4+0], m[idx*4+1], m[idx*4+2]);
        }
        return Vector();
    }
};

inline Vector operator * (const Matrix& mx, const Vector& vec)
{
    return Vector(mx.m00*vec.x + mx.m10*vec.y + mx.m20*vec.z + mx.m30,
                  mx.m01*vec.x + mx.m11*vec.y + mx.m21*vec.z + mx.m31,
                  mx.m02*vec.x + mx.m12*vec.y + mx.m22*vec.z + mx.m32);
}

inline Vector operator * (const Vector& vec, const Matrix& mx)
{
    return Vector(mx.m00*vec.x + mx.m01*vec.x + mx.m02*vec.x,
                  mx.m10*vec.y + mx.m11*vec.y + mx.m12*vec.y,
                  mx.m20*vec.z + mx.m21*vec.z + mx.m22*vec.z);
}

inline Matrix operator * (const Matrix& first, const Matrix& second)
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

inline std::ostream& operator << (std::ostream& os, const Matrix& mx)
{
    os << "Matrix(";
    for (int i=0; i<15; i++)
    {
        os << mx.m[i] << ", ";
    }
    os << mx.m[15] << ")";
    return os;
}

#endif
