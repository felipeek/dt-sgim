#include "graphics_math.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

extern boolean gmInverseMat4(const Mat4* m, Mat4* invOut)
{
	Mat4 inv;
	r32 det;
	s32 i;

	r32* mData = (r32*)m->data;
	r32* invOutData = (r32*)invOut->data;
	r32* invData = (r32*)inv.data;

	invData[0] = mData[5] * mData[10] * mData[15] -
		mData[5] * mData[11] * mData[14] -
		mData[9] * mData[6] * mData[15] +
		mData[9] * mData[7] * mData[14] +
		mData[13] * mData[6] * mData[11] -
		mData[13] * mData[7] * mData[10];

	invData[4] = -mData[4] * mData[10] * mData[15] +
		mData[4] * mData[11] * mData[14] +
		mData[8] * mData[6] * mData[15] -
		mData[8] * mData[7] * mData[14] -
		mData[12] * mData[6] * mData[11] +
		mData[12] * mData[7] * mData[10];

	invData[8] = mData[4] * mData[9] * mData[15] -
		mData[4] * mData[11] * mData[13] -
		mData[8] * mData[5] * mData[15] +
		mData[8] * mData[7] * mData[13] +
		mData[12] * mData[5] * mData[11] -
		mData[12] * mData[7] * mData[9];

	invData[12] = -mData[4] * mData[9] * mData[14] +
		mData[4] * mData[10] * mData[13] +
		mData[8] * mData[5] * mData[14] -
		mData[8] * mData[6] * mData[13] -
		mData[12] * mData[5] * mData[10] +
		mData[12] * mData[6] * mData[9];

	invData[1] = -mData[1] * mData[10] * mData[15] +
		mData[1] * mData[11] * mData[14] +
		mData[9] * mData[2] * mData[15] -
		mData[9] * mData[3] * mData[14] -
		mData[13] * mData[2] * mData[11] +
		mData[13] * mData[3] * mData[10];

	invData[5] = mData[0] * mData[10] * mData[15] -
		mData[0] * mData[11] * mData[14] -
		mData[8] * mData[2] * mData[15] +
		mData[8] * mData[3] * mData[14] +
		mData[12] * mData[2] * mData[11] -
		mData[12] * mData[3] * mData[10];

	invData[9] = -mData[0] * mData[9] * mData[15] +
		mData[0] * mData[11] * mData[13] +
		mData[8] * mData[1] * mData[15] -
		mData[8] * mData[3] * mData[13] -
		mData[12] * mData[1] * mData[11] +
		mData[12] * mData[3] * mData[9];

	invData[13] = mData[0] * mData[9] * mData[14] -
		mData[0] * mData[10] * mData[13] -
		mData[8] * mData[1] * mData[14] +
		mData[8] * mData[2] * mData[13] +
		mData[12] * mData[1] * mData[10] -
		mData[12] * mData[2] * mData[9];

	invData[2] = mData[1] * mData[6] * mData[15] -
		mData[1] * mData[7] * mData[14] -
		mData[5] * mData[2] * mData[15] +
		mData[5] * mData[3] * mData[14] +
		mData[13] * mData[2] * mData[7] -
		mData[13] * mData[3] * mData[6];

	invData[6] = -mData[0] * mData[6] * mData[15] +
		mData[0] * mData[7] * mData[14] +
		mData[4] * mData[2] * mData[15] -
		mData[4] * mData[3] * mData[14] -
		mData[12] * mData[2] * mData[7] +
		mData[12] * mData[3] * mData[6];

	invData[10] = mData[0] * mData[5] * mData[15] -
		mData[0] * mData[7] * mData[13] -
		mData[4] * mData[1] * mData[15] +
		mData[4] * mData[3] * mData[13] +
		mData[12] * mData[1] * mData[7] -
		mData[12] * mData[3] * mData[5];

	invData[14] = -mData[0] * mData[5] * mData[14] +
		mData[0] * mData[6] * mData[13] +
		mData[4] * mData[1] * mData[14] -
		mData[4] * mData[2] * mData[13] -
		mData[12] * mData[1] * mData[6] +
		mData[12] * mData[2] * mData[5];

	invData[3] = -mData[1] * mData[6] * mData[11] +
		mData[1] * mData[7] * mData[10] +
		mData[5] * mData[2] * mData[11] -
		mData[5] * mData[3] * mData[10] -
		mData[9] * mData[2] * mData[7] +
		mData[9] * mData[3] * mData[6];

	invData[7] = mData[0] * mData[6] * mData[11] -
		mData[0] * mData[7] * mData[10] -
		mData[4] * mData[2] * mData[11] +
		mData[4] * mData[3] * mData[10] +
		mData[8] * mData[2] * mData[7] -
		mData[8] * mData[3] * mData[6];

	invData[11] = -mData[0] * mData[5] * mData[11] +
		mData[0] * mData[7] * mData[9] +
		mData[4] * mData[1] * mData[11] -
		mData[4] * mData[3] * mData[9] -
		mData[8] * mData[1] * mData[7] +
		mData[8] * mData[3] * mData[5];

	invData[15] = mData[0] * mData[5] * mData[10] -
		mData[0] * mData[6] * mData[9] -
		mData[4] * mData[1] * mData[10] +
		mData[4] * mData[2] * mData[9] +
		mData[8] * mData[1] * mData[6] -
		mData[8] * mData[2] * mData[5];

	det = mData[0] * invData[0] + mData[1] * invData[4] + mData[2] * invData[8] + mData[3] * invData[12];

	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	for (i = 0; i < 16; i++)
		invOutData[i] = invData[i] * det;

	return true;
}

extern Mat4 gmMultiplyMat4(const Mat4* m1, const Mat4* m2)
{
	Mat4 result;

	result.data[0][0] = m1->data[0][0] * m2->data[0][0] + m1->data[0][1] * m2->data[1][0] + m1->data[0][2] * m2->data[2][0] + m1->data[0][3] * m2->data[3][0];
	result.data[0][1] = m1->data[0][0] * m2->data[0][1] + m1->data[0][1] * m2->data[1][1] + m1->data[0][2] * m2->data[2][1] + m1->data[0][3] * m2->data[3][1];
	result.data[0][2] = m1->data[0][0] * m2->data[0][2] + m1->data[0][1] * m2->data[1][2] + m1->data[0][2] * m2->data[2][2] + m1->data[0][3] * m2->data[3][2];
	result.data[0][3] = m1->data[0][0] * m2->data[0][3] + m1->data[0][1] * m2->data[1][3] + m1->data[0][2] * m2->data[2][3] + m1->data[0][3] * m2->data[3][3];
	result.data[1][0] = m1->data[1][0] * m2->data[0][0] + m1->data[1][1] * m2->data[1][0] + m1->data[1][2] * m2->data[2][0] + m1->data[1][3] * m2->data[3][0];
	result.data[1][1] = m1->data[1][0] * m2->data[0][1] + m1->data[1][1] * m2->data[1][1] + m1->data[1][2] * m2->data[2][1] + m1->data[1][3] * m2->data[3][1];
	result.data[1][2] = m1->data[1][0] * m2->data[0][2] + m1->data[1][1] * m2->data[1][2] + m1->data[1][2] * m2->data[2][2] + m1->data[1][3] * m2->data[3][2];
	result.data[1][3] = m1->data[1][0] * m2->data[0][3] + m1->data[1][1] * m2->data[1][3] + m1->data[1][2] * m2->data[2][3] + m1->data[1][3] * m2->data[3][3];
	result.data[2][0] = m1->data[2][0] * m2->data[0][0] + m1->data[2][1] * m2->data[1][0] + m1->data[2][2] * m2->data[2][0] + m1->data[2][3] * m2->data[3][0];
	result.data[2][1] = m1->data[2][0] * m2->data[0][1] + m1->data[2][1] * m2->data[1][1] + m1->data[2][2] * m2->data[2][1] + m1->data[2][3] * m2->data[3][1];
	result.data[2][2] = m1->data[2][0] * m2->data[0][2] + m1->data[2][1] * m2->data[1][2] + m1->data[2][2] * m2->data[2][2] + m1->data[2][3] * m2->data[3][2];
	result.data[2][3] = m1->data[2][0] * m2->data[0][3] + m1->data[2][1] * m2->data[1][3] + m1->data[2][2] * m2->data[2][3] + m1->data[2][3] * m2->data[3][3];
	result.data[3][0] = m1->data[3][0] * m2->data[0][0] + m1->data[3][1] * m2->data[1][0] + m1->data[3][2] * m2->data[2][0] + m1->data[3][3] * m2->data[3][0];
	result.data[3][1] = m1->data[3][0] * m2->data[0][1] + m1->data[3][1] * m2->data[1][1] + m1->data[3][2] * m2->data[2][1] + m1->data[3][3] * m2->data[3][1];
	result.data[3][2] = m1->data[3][0] * m2->data[0][2] + m1->data[3][1] * m2->data[1][2] + m1->data[3][2] * m2->data[2][2] + m1->data[3][3] * m2->data[3][2];
	result.data[3][3] = m1->data[3][0] * m2->data[0][3] + m1->data[3][1] * m2->data[1][3] + m1->data[3][2] * m2->data[2][3] + m1->data[3][3] * m2->data[3][3];

	return result;
}

extern Mat3 gmMultiplyMat3(const Mat3* m1, const Mat3* m2)
{
	Mat3 result;

	result.data[0][0] = m1->data[0][0] * m2->data[0][0] + m1->data[0][1] * m2->data[1][0] + m1->data[0][2] * m2->data[2][0];
	result.data[0][1] = m1->data[0][0] * m2->data[0][1] + m1->data[0][1] * m2->data[1][1] + m1->data[0][2] * m2->data[2][1];
	result.data[0][2] = m1->data[0][0] * m2->data[0][2] + m1->data[0][1] * m2->data[1][2] + m1->data[0][2] * m2->data[2][2];
	result.data[1][0] = m1->data[1][0] * m2->data[0][0] + m1->data[1][1] * m2->data[1][0] + m1->data[1][2] * m2->data[2][0];
	result.data[1][1] = m1->data[1][0] * m2->data[0][1] + m1->data[1][1] * m2->data[1][1] + m1->data[1][2] * m2->data[2][1];
	result.data[1][2] = m1->data[1][0] * m2->data[0][2] + m1->data[1][1] * m2->data[1][2] + m1->data[1][2] * m2->data[2][2];
	result.data[2][0] = m1->data[2][0] * m2->data[0][0] + m1->data[2][1] * m2->data[1][0] + m1->data[2][2] * m2->data[2][0];
	result.data[2][1] = m1->data[2][0] * m2->data[0][1] + m1->data[2][1] * m2->data[1][1] + m1->data[2][2] * m2->data[2][1];
	result.data[2][2] = m1->data[2][0] * m2->data[0][2] + m1->data[2][1] * m2->data[1][2] + m1->data[2][2] * m2->data[2][2];

	return result;
}

extern Mat2 gmMultiplyMat2(const Mat2* m1, const Mat2* m2)
{
	Mat2 result;

	result.data[0][0] = m1->data[0][0] * m2->data[0][0] + m1->data[0][1] * m2->data[1][0];
	result.data[0][1] = m1->data[0][0] * m2->data[0][1] + m1->data[0][1] * m2->data[1][1];
	result.data[1][0] = m1->data[1][0] * m2->data[0][0] + m1->data[1][1] * m2->data[1][0];
	result.data[1][1] = m1->data[1][0] * m2->data[0][1] + m1->data[1][1] * m2->data[1][1];

	return result;
}

extern Vec3 gmMultiplyMat3AndVec3(const Mat3* m, Vec3 v)
{
	Vec3 result;

	result.x = v.x * m->data[0][0] + v.y * m->data[0][1] + v.z * m->data[0][2];
	result.y = v.x * m->data[1][0] + v.y * m->data[1][1] + v.z * m->data[1][2];
	result.z = v.x * m->data[2][0] + v.y * m->data[2][1] + v.z * m->data[2][2];

	return result;
}

extern Vec4 gmMultiplyMat4AndVec4(const Mat4* m, Vec4 v)
{
	Vec4 result;

	result.x = v.x * m->data[0][0] + v.y * m->data[0][1] + v.z * m->data[0][2] + v.w * m->data[0][3];
	result.y = v.x * m->data[1][0] + v.y * m->data[1][1] + v.z * m->data[1][2] + v.w * m->data[1][3];
	result.z = v.x * m->data[2][0] + v.y * m->data[2][1] + v.z * m->data[2][2] + v.w * m->data[2][3];
	result.w = v.x * m->data[3][0] + v.y * m->data[3][1] + v.z * m->data[3][2] + v.w * m->data[3][3];

	return result;
}

extern Mat4 gmTransposeMat4(const Mat4* m)
{
	return (Mat4) {
		m->data[0][0], m->data[1][0], m->data[2][0], m->data[3][0],
		m->data[0][1], m->data[1][1], m->data[2][1], m->data[3][1],
		m->data[0][2], m->data[1][2], m->data[2][2], m->data[3][2],
		m->data[0][3], m->data[1][3], m->data[2][3], m->data[3][3]
	};
}

extern Mat3 gmTransposeMat3(const Mat3* m)
{
	return (Mat3) {
		m->data[0][0], m->data[1][0], m->data[2][0],
		m->data[0][1], m->data[1][1], m->data[2][1],
		m->data[0][2], m->data[1][2], m->data[2][2]
	};
}

extern Mat2 gmTransposeMat2(const Mat2* m)
{
	return (Mat2) {
		m->data[0][0], m->data[1][0],
		m->data[0][1], m->data[1][1],
	};
}

extern Mat4 gmScalarProductMat4(r32 scalar, const Mat4* m)
{
	return (Mat4) {
		scalar * m->data[0][0], scalar * m->data[0][1], scalar * m->data[0][2], scalar * m->data[0][3],
		scalar * m->data[1][0], scalar * m->data[1][1], scalar * m->data[1][2], scalar * m->data[1][3],
		scalar * m->data[2][0], scalar * m->data[2][1], scalar * m->data[2][2], scalar * m->data[2][3],
		scalar * m->data[3][0], scalar * m->data[3][1], scalar * m->data[3][2], scalar * m->data[3][3],
	};
}

extern Mat3 gmScalarProductMat3(r32 scalar, const Mat3* m)
{
	return (Mat3) {
		scalar * m->data[0][0], scalar * m->data[0][1], scalar * m->data[0][2],
		scalar * m->data[1][0], scalar * m->data[1][1], scalar * m->data[1][2],
		scalar * m->data[2][0], scalar * m->data[2][1], scalar * m->data[2][2]
	};
}

extern Mat2 gmScalarProductMat2(r32 scalar, const Mat2* m)
{
	return (Mat2) {
		scalar * m->data[0][0], scalar * m->data[0][1],
		scalar * m->data[1][0], scalar * m->data[1][1]
	};
}

extern Mat4 gmIdentityMat4(void)
{
	return (Mat4) {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

extern Mat3 gmIdentityMat3(void)
{
	return (Mat3) {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};
}

extern Mat2 gmIdentityMat2(void)
{
	return (Mat2) {
		1.0f, 0.0f,
		0.0f, 1.0f
	};
}

extern boolean gmEqualVec2(Vec2 v1, Vec2 v2)
{
	if (v1.x == v2.x && v1.y == v2.y)
		return true;
	return false;
}

extern boolean gmEqualVec3(Vec3 v1, Vec3 v2)
{
	if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z)
		return true;
	return false;
}

extern boolean gmEqualVec4(Vec4 v1, Vec4 v2)
{
	if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w)
		return true;
	return false;
}

extern Vec4 gmScalarProductVec4(r32 scalar, Vec4 v)
{
	return (Vec4) { scalar * v.x, scalar * v.y, scalar * v.z, scalar * v.w };
}

extern Vec3 gmScalarProductVec3(r32 scalar, Vec3 v)
{
	return (Vec3) { scalar * v.x, scalar * v.y, scalar * v.z };
}

extern Vec2 gmScalarProductVec2(r32 scalar, Vec2 v)
{
	return (Vec2) { scalar * v.x, scalar * v.y };
}

extern Vec4 gmNormalizeVec4(Vec4 v)
{
	if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f && v.w == 0.0f)
	{
		printf("WARNING: gmNormalizeVec4 received <0,0,0,0>. This might be a bug. Returning 0 for now...\n");
		return (Vec4) {0.0f, 0.0f, 0.0f, 0.0f};
	}

	assert(v.x != 0.0f || v.y != 0.0f || v.z != 0.0f || v.w != 0.0f);
	r32 vectorLength = gmLengthVec4(v);
	return (Vec4) { v.x / vectorLength, v.y / vectorLength, v.z / vectorLength, v.w / vectorLength };
}

extern Vec3 gmNormalizeVec3(Vec3 v)
{
	if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)
	{
		printf("WARNING: gmNormalizeVec3 received <0,0,0>. This might be a bug. Returning 0 for now...\n");
		return (Vec3) {0.0f, 0.0f, 0.0f};
	}

	assert(v.x != 0.0f || v.y != 0.0f || v.z != 0.0f);
	r32 vectorLength = gmLengthVec3(v);
	return (Vec3) { v.x / vectorLength, v.y / vectorLength, v.z / vectorLength };
}

extern Vec2 gmNormalizeVec2(Vec2 v)
{
	assert(v.x != 0.0f || v.y != 0.0f);
	r32 vectorLength = gmLengthVec2(v);
	return (Vec2) { v.x / vectorLength, v.y / vectorLength };
}

extern r32 gmLengthVec4(Vec4 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

extern r32 gmLengthVec3(Vec3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

extern r32 gmLengthVec2(Vec2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

extern Vec4 gmAddVec4(Vec4 v1, Vec4 v2)
{
	return (Vec4) { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
}

extern Vec3 gmAddVec3(Vec3 v1, Vec3 v2)
{
	return (Vec3) { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

extern Vec2 gmAddVec2(Vec2 v1, Vec2 v2)
{
	return (Vec2) { v1.x + v2.x, v1.y + v2.y };
}

extern Vec4 gmSubtractVec4(Vec4 v1, Vec4 v2)
{
	return (Vec4) { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
}

extern Vec3 gmSubtractVec3(Vec3 v1, Vec3 v2)
{
	return (Vec3) { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

extern Vec2 gmSubtractVec2(Vec2 v1, Vec2 v2)
{
	return (Vec2) { v1.x - v2.x, v1.y - v2.y };
}

extern r32 gmDotProductVec4(Vec4 v1, Vec4 v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

extern r32 gmDotProductVec3(Vec3 v1, Vec3 v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

extern r32 gmDotProductVec2(Vec2 v1, Vec2 v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

extern r32 gmAngleVec2(Vec2 v)
{
	return atan2f(v.y, v.x);
}

extern r32 gmRadians(r32 degrees)
{
	return PI_F * degrees / 180.0f;
}

extern Vec3 gmCrossProduct(Vec3 v1, Vec3 v2)
{
	Vec3 result;

	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}

extern r32 gmAbsolute(r32 x)
{
	return (x < 0) ? -x : x;
}