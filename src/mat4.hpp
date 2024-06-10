#ifndef MAT4_HPP_INCLUDED
#define MAT4_HPP_INCLUDED

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>

struct Mat4{
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;

	inline void print();
};

inline constexpr Mat4 operator*(const Mat4& left, const Mat4& right){
	return {
		left.m00*right.m00 + left.m01*right.m10 + left.m02*right.m20 + left.m03*right.m30,
		left.m00*right.m01 + left.m01*right.m11 + left.m02*right.m21 + left.m03*right.m31,
		left.m00*right.m02 + left.m01*right.m12 + left.m02*right.m22 + left.m03*right.m32,
		left.m00*right.m03 + left.m01*right.m13 + left.m02*right.m23 + left.m03*right.m33,
		left.m10*right.m00 + left.m11*right.m10 + left.m12*right.m20 + left.m13*right.m30,
		left.m10*right.m01 + left.m11*right.m11 + left.m12*right.m21 + left.m13*right.m31,
		left.m10*right.m02 + left.m11*right.m12 + left.m12*right.m22 + left.m13*right.m32,
		left.m10*right.m03 + left.m11*right.m13 + left.m12*right.m23 + left.m13*right.m33,
		left.m20*right.m00 + left.m21*right.m10 + left.m22*right.m20 + left.m23*right.m30,
		left.m20*right.m01 + left.m21*right.m11 + left.m22*right.m21 + left.m23*right.m31,
		left.m20*right.m02 + left.m21*right.m12 + left.m22*right.m22 + left.m23*right.m32,
		left.m20*right.m03 + left.m21*right.m13 + left.m22*right.m23 + left.m23*right.m33,
		left.m30*right.m00 + left.m31*right.m10 + left.m32*right.m20 + left.m33*right.m30,
		left.m30*right.m01 + left.m31*right.m11 + left.m32*right.m21 + left.m33*right.m31,
		left.m30*right.m02 + left.m31*right.m12 + left.m32*right.m22 + left.m33*right.m32,
		left.m30*right.m03 + left.m31*right.m13 + left.m32*right.m23 + left.m33*right.m33
	};
}

inline Mat4 perspective(float fov, float aspect, float near, float far){
    float s = 1 / (float)tan((fov * M_PI / 180.0f) / 2.0f);
	float sx = s / aspect;
	float a = -((near + far) / (far - near));
	float b = -2.0f * ((far * near) / (far - near));
	return {
		sx, 0.0f, 0.0f, 0.0f,
		0.0f, s, 0.0f, 0.0f,
		0.0f, 0.0f, a, b,
		0.0f, 0.0f, -1.0f, 0.0f
	};
}

inline Mat4 make_scaling(float sX, float sY, float sZ) {
	return {
		sX, 0.0f, 0.0f, 0.0f,
		0.0f, sY, 0.0f, 0.0f,
		0.0f, 0.0f, sZ, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

void Mat4::print(){
	std::printf("| %.2f %.2f %.2f %.2f |\n", m00, m01, m02, m03);
    std::printf("| %.2f %.2f %.2f %.2f |\n", m10, m11, m12, m13);
    std::printf("| %.2f %.2f %.2f %.2f |\n", m20, m21, m22, m23);
    std::printf("| %.2f %.2f %.2f %.2f |\n", m30, m31, m32, m33);
}

#endif