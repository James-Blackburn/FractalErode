#ifndef VEC2_HPP_INCLUDED
#define VEC2_HPP_INCLUDED

#include <cmath>

struct Vec2{
    float x,y;

    inline void normalize();
    inline float length();
};

// this normalizes the vector, turning it into a unit vector with the same direction
void Vec2::normalize(){
    float vlength = length();
    x = x / vlength;
    y = y / vlength;
}

inline Vec2 normalize(Vec2 vec){
    float vlength = vec.length();
    return {
        vec.x / vlength,
        vec.y / vlength,
    };
}

// this returns the length of the vector
float Vec2::length(){
    return std::sqrt(x*x + y*y);
}

// function to calculate dot product of two vectors
inline float dot(const Vec2& v1, const Vec2& v2){
    return v1.x*v2.x + v1.y*v2.y;
}

#endif