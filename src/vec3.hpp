#ifndef VEC3_HPP_INCLUDED
#define VEC3_HPP_INCLUDED

#include <cmath>

struct Vec3{
    float x;
    float y;
    float z;

    inline void normalize();
    inline float length() const;
    
    Vec3& operator+=(const Vec3& rhs) {
        x+=rhs.x; y+=rhs.y; z+=rhs.z;
        return *this;
    }

    Vec3& operator-() {
        x=-x; y=-y; z=-z;
        return *this;
    }

    bool operator==(const Vec3& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }
};

// operator overloading for useful operators
inline constexpr const Vec3 operator*(const Vec3& left, const float& right){
	return { left.x * right, left.y * right, left.z * right };
}

inline constexpr const Vec3 operator+(const Vec3& left, const Vec3& right){
    return { left.x + right.x, left.y + right.y, left.z + right.z };
}

inline constexpr const Vec3 operator-(const Vec3& left, const Vec3& right){
    return { left.x - right.x, left.y - right.y, left.z - right.z };
}

inline constexpr const Vec3 operator/(const Vec3& left, const float& right) {
    return { left.x / right, left.y / right, left.z / right };
}

// this normalizes the vector, turning it into a unit vector with the same direction
void Vec3::normalize(){
    float vlength = length();
    x = x / vlength;
    y = y / vlength;
    z = z / vlength;
}

inline Vec3 normalize(Vec3 vec){
    float vlength = vec.length();
    return { vec.x / vlength, vec.y / vlength, vec.z / vlength };
}

// this returns the length of the vector
float Vec3::length() const{
    return std::sqrt(x*x + y*y + z*z);
}

// function to calculate cross product of two vectors
inline Vec3 cross(const Vec3& v1, const Vec3& v2){
    return {
        v1.y*v2.z - v1.z*v2.y, 
        v1.z*v2.x - v1.x*v2.z, 
        v1.x*v2.y - v1.y*v2.x
    };
}

inline float dot(const Vec3& v1, const Vec3& v2){
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

#endif