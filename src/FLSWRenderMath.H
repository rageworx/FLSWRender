#ifndef __MATHES_H__
#define __MATHES_H__
#pragma once

#include <cmath>
#include <vector>
#include <algorithm>

#ifndef M_PI
    #define M_PI    acos(-1) 
#endif

#pragma region Vector
namespace ____MATH__Vector
{
    template <class T> struct Vector2
    {
        union 
        {
            struct{ T x,y;};
            struct{ T u,v;};
            T raw[2];
        };

        Vector2():x(0),y(0){ }
        Vector2(T xx,T yy):x(xx),y(yy){ }

        inline Vector2<T> operator+ \
            (const Vector2<T>& a) const{ return Vector2<T>(a.x+x,a.y+y); }
        inline Vector2<T> operator- \
            (const Vector2<T>& a) const{ return Vector2<T>(x-a.x,y-a.y); }
        inline Vector2<T> operator* \
            (T f) const {return Vector2<T>(x*f,y*f); }
    };

    template <class T> struct Vector3
    {
        union {
            struct {T x, y, z;};
            struct { T ivert, iuv, inorm; };
            T raw[3];
        };

        Vector3() : x(0), y(0), z(0) {}
        Vector3(T xx, T yy, T zz) : x(xx),y(yy),z(zz) {}
        Vector3(T data[3]):x(data[0]),y(data[1]),z(data[2]){}

        // Cross product
        inline Vector3<T> operator^ \
            (const Vector3<T> &v) const 
            { return Vector3<T>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
        
        inline Vector3<T> operator+ \
            (const Vector3<T> &v) const 
            { return Vector3<T>(x+v.x, y+v.y, z+v.z); }
        
        inline Vector3<T> operator- \
            (const Vector3<T> &v) const 
            { return Vector3<T>(x-v.x, y-v.y, z-v.z); }
        
        inline Vector3<T> operator* \
            (float f) const { return Vector3<T>(x*f, y*f, z*f); }

        inline T operator* \
            (const Vector3<T> &v) const 
            { return x*v.x + y*v.y + z*v.z; }

        float len() const 
            { return std::sqrt(x*x+y*y+z*z); }

        Vector3<T>& normalize() 
            { *this = (*this)*(1/len()); return *this; }
    };
}

struct vec4f
{
    union {
        struct {float x,y,z,w;};
        float raw[4];
    };
    
    vec4f(){ }
    vec4f(float xx,float yy,float zz,float ww) \
        :x(xx),y(yy),z(zz),w(ww){ }
    inline vec4f operator* (float f) const \
        {return vec4f(x*f,y*f,z*f,w*f); }
};

typedef ____MATH__Vector::Vector2<float> vec2f;
typedef ____MATH__Vector::Vector2<int>   vec2i;
typedef ____MATH__Vector::Vector3<float> vec3f;
typedef ____MATH__Vector::Vector3<int>   vec3i;
typedef std::vector<vec3f> vvec3f;
typedef std::vector<vec3i> vvec3i;
typedef std::vector<vec2f> vvec2f;
#pragma endregion

#pragma region Matrix
namespace ____MATH__Matrix
{
    class Matrix
    {
        private:
            size_t ncols;
            size_t nrows;
            std::vector<std::vector<float>> m;
            
        public:
            Matrix(int row,int col)
            :m(std::vector<std::vector<float>> \
                    (row,std::vector<float>(col,0.f))),\
                    ncols(col),nrows(row){ }
            Matrix(){ }
            
            // Column matrix 4*1 
            Matrix(vec3f v)
            :m(std::vector<std::vector<float>> \
                    (4, std::vector<float>(1, 1.f))),nrows(4), ncols(1)
            {
                m[0][0] = v.x;
                m[1][0] = v.y;
                m[2][0] = v.z;
                m[3][0] = 1.f;
            }

        public:
            size_t rows() {return nrows;}
            size_t cols() {return ncols;}

            // Define the identity matrix 
            static Matrix identity(size_t row);
            std::vector<float>& operator[](const size_t i);
            Matrix operator*(const Matrix& a);
            Matrix transpose();
            Matrix inverse();

            // 4x1 matrix is converted to a vector, 
            // and the w component is lost 
            vec3f ToVec3f(){ return vec3f(m[0][0],m[1][0],m[2][0]); }
            
            // Convert 4x1 matrix to vector without losing w component 
            vec4f ToVec4f(){ return vec4f(m[0][0],m[1][0],m[2][0],m[3][0]); }
    };
}

//mat(r,c)
typedef ____MATH__Matrix::Matrix mat;
#pragma endregion

#endif /// of __MATHES_H__
