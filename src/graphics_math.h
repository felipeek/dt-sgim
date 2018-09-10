#ifndef GIMMESH_GRAPHICS_MATH_H
#define GIMMESH_GRAPHICS_MATH_H
#include "common.h"

typedef struct Mat4Struct Mat4;
typedef struct Mat3Struct Mat3;
typedef struct Mat2Struct Mat2;
typedef union Vec4Union Vec4;
typedef union Vec3Union Vec3;
typedef struct Vec2Struct Vec2;
typedef union DiscreteVec4Union DiscreteVec4;
typedef union DiscreteVec3Union DiscreteVec3;
typedef struct DiscreteVec2Struct DiscreteVec2;

#pragma pack(push, 1)
struct Mat4Struct
{
	r32 data[4][4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Mat3Struct
{
	r32 data[3][3];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Mat2Struct
{
	r32 data[2][2];
};
#pragma pack(pop)

#pragma pack(push, 1)
union Vec4Union
{
	struct
	{
		r32 x, y, z, w;
	};
	struct
	{
		r32 r, g, b, a;
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
union Vec3Union
{
	struct
	{
		r32 x, y, z;
	};
	struct
	{
		r32 r, g, b;
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Vec2Struct
{
	r32 x, y;
};
#pragma pack(pop)

#pragma pack(push, 1)
union DiscreteVec4Union
{
	struct
	{
		s32 x, y, z, w;
	};
	struct
	{
		s32 r, g, b, a;
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
union DiscreteVec3Union
{
	struct
	{
		s32 x, y, z;
	};
	struct
	{
		s32 r, g, b;
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DiscreteVec2Struct
{
	s32 x, y;
};
#pragma pack(pop)

extern boolean gmInverseMat4(const Mat4* m, Mat4* invOut);
extern Vec4 gmMultiplyMat4AndVec4(const Mat4* m, Vec4 v);
extern Mat4 gmMultiplyMat4(const Mat4* m1, const Mat4* m2);
extern Mat3 gmMultiplyMat3(const Mat3* m1, const Mat3* m2);
extern Mat2 gmMultiplyMat2(const Mat2* m1, const Mat2* m2);
extern Mat4 gmTransposeMat4(const Mat4* m);
extern Mat3 gmTransposeMat3(const Mat3* m);
extern Mat2 gmTransposeMat2(const Mat2* m);
extern Mat4 gmScalarProductMat4(r32 scalar, const Mat4* m);
extern Mat3 gmScalarProductMat3(r32 scalar, const Mat3* m);
extern Mat2 gmScalarProductMat2(r32 scalar, const Mat2* m);
extern Mat4 gmIdentityMat4(void);
extern Mat3 gmIdentityMat3(void);
extern Mat2 gmIdentityMat2(void);
extern boolean gmEqualVec2(Vec2 v1, Vec2 v2);
extern boolean gmEqualVec3(Vec3 v1, Vec3 v2);
extern boolean gmEqualVec4(Vec4 v1, Vec4 v2);
extern Vec4 gmScalarProductVec4(r32 scalar, Vec4 v);
extern Vec3 gmScalarProductVec3(r32 scalar, Vec3 v);
extern Vec2 gmScalarProductVec2(r32 scalar, Vec2 v);
extern Vec4 gmNormalizeVec4(Vec4 v);
extern Vec3 gmNormalizeVec3(Vec3 v);
extern Vec2 gmNormalizeVec2(Vec2 v);
extern r32 gmLengthVec4(Vec4 v);
extern r32 gmLengthVec3(Vec3 v);
extern r32 gmLengthVec2(Vec2 v);
extern Vec4 gmAddVec4(Vec4 v1, Vec4 v2);
extern Vec3 gmAddVec3(Vec3 v1, Vec3 v2);
extern Vec2 gmAddVec2(Vec2 v1, Vec2 v2);
extern Vec4 gmSubtractVec4(Vec4 v1, Vec4 v2);
extern Vec3 gmSubtractVec3(Vec3 v1, Vec3 v2);
extern Vec2 gmSubtractVec2(Vec2 v1, Vec2 v2);
extern r32 gmDotProductVec4(Vec4 v1, Vec4 v2);
extern r32 gmDotProductVec3(Vec3 v1, Vec3 v2);
extern r32 gmDotProductVec2(Vec2 v1, Vec2 v2);
extern r32 gmAngleVec2(Vec2 v);
extern r32 gmRadians(r32 degrees);
extern Vec3 gmCrossProduct(Vec3 v1, Vec3 v2);
extern r32 gmAbsolute(r32 x);

#endif