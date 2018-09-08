#ifndef GIMMESH_CAMERA_H
#define GIMMESH_CAMERA_H
#include "graphics_math.h"

typedef struct PerspectiveCameraStruct PerspectiveCamera;

struct PerspectiveCameraStruct
{
	Vec4 position;
	Vec4 up;
	Vec4 view;
	r32 nearPlane;
	r32 farPlane;
	r32 pitch;
	r32 yaw;
	r32 fov;
	Mat4 viewMatrix;
	Mat4 projectionMatrix;
	Vec4 xAxis;
	Vec4 yAxis;
	Vec4 zAxis;
};

extern void cameraInit(PerspectiveCamera* camera, Vec4 position, Vec4 up, Vec4 view, r32 nearPlane, r32 farPlane, r32 fov);
extern void cameraSetPosition(PerspectiveCamera* camera, Vec4 position);
extern void cameraSetUp(PerspectiveCamera* camera, Vec4 up);
extern void cameraSetView(PerspectiveCamera* camera, Vec4 view);
extern void cameraSetNearPlane(PerspectiveCamera* camera, r32 nearPlane);
extern void cameraSetFarPlane(PerspectiveCamera* camera, r32 farPlane);
extern void cameraIncPitch(PerspectiveCamera* camera, r32 angle);
extern void cameraIncYaw(PerspectiveCamera* camera, r32 angle);
extern void cameraSetFov(PerspectiveCamera* camera, r32 fov);
extern void cameraForceMatrixRecalculation(PerspectiveCamera* camera);

#endif