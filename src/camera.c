#include "camera.h"
#include <math.h>

extern s32 windowWidth;
extern s32 windowHeight;

static void truncateAngles(PerspectiveCamera* camera)
{
	const float yawTruncateThreshold = 0.1f;
	const float yawBottomLimit = PI_F - yawTruncateThreshold;
	const float yawTopLimit = yawTruncateThreshold;

	if (camera->yaw > yawBottomLimit)
		camera->yaw = yawBottomLimit;
	else if (camera->yaw < yawTopLimit)
		camera->yaw = yawTopLimit;
}

static void recalculateAngles(PerspectiveCamera* camera)
{
	camera->pitch = atan2f(camera->view.x, camera->view.z);
	camera->yaw = atan2f(sqrtf(camera->view.x * camera->view.x + camera->view.z * camera->view.z), camera->view.y);
	truncateAngles(camera);
}

static void recalculateViewMatrix(PerspectiveCamera* camera)
{
	Vec3 upVec3 = (Vec3) { camera->up.x, camera->up.y, camera->up.z };
	Vec4 w = gmScalarProductVec4(-1, gmNormalizeVec4(camera->view));
	Vec3 wVec3 = (Vec3) { w.x, w.y, w.z };
	Vec3 upWCross = gmCrossProduct(upVec3, wVec3);
	Vec4 u = gmNormalizeVec4((Vec4) { upWCross.x, upWCross.y, upWCross.z, 0.0f });
	Vec3 uVec3 = (Vec3) { u.x, u.y, u.z };
	Vec3 vVec3 = gmCrossProduct(wVec3, uVec3);
	Vec4 v = (Vec4) { vVec3.x, vVec3.y, vVec3.z, 0.0f };
	// Useless, but conceptually correct.
	Vec4 worldToCameraVec = gmSubtractVec4(camera->position, (Vec4) { 0.0f, 0.0f, 0.0f, 1.0f });

	camera->xAxis = u;
	camera->yAxis = v;
	camera->zAxis = w;

	// Need to transpose when sending to shader
	camera->viewMatrix = (Mat4) {
		u.x, u.y, u.z, -gmDotProductVec4(u, worldToCameraVec),
			v.x, v.y, v.z, -gmDotProductVec4(v, worldToCameraVec),
			w.x, w.y, w.z, -gmDotProductVec4(w, worldToCameraVec),
			0.0f, 0.0f, 0.0f, 1.0f
	};
}


static void recalculateProjectionMatrix(PerspectiveCamera* camera)
{
	r32 near = camera->nearPlane;
	r32 far = camera->farPlane;
	r32 top = (r32)fabs(near) * atanf(gmRadians(camera->fov) / 2.0f);
	r32 bottom = -top;
	r32 right = top * ((r32)windowWidth / (r32)windowHeight);
	r32 left = -right;

	Mat4 P = (Mat4) {
		near, 0, 0, 0,
			0, near, 0, 0,
			0, 0, near + far, -near * far,
			0, 0, 1, 0
	};

	Mat4 M = (Mat4) {
		2.0f / (right - left), 0, 0, -(right + left) / (right - left),
			0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
			0, 0, 2.0f / (far - near), -(far + near) / (far - near),
			0, 0, 0, 1
	};

	// Need to transpose when sending to shader
	Mat4 MP = gmMultiplyMat4(&M, &P);
	camera->projectionMatrix = gmScalarProductMat4(-1, &MP);
}

static void recalculateView(PerspectiveCamera* camera)
{
	camera->view.x = sinf(camera->pitch) * sinf(camera->yaw);
	camera->view.y = cosf(camera->yaw);
	camera->view.z = cosf(camera->pitch) * sinf(camera->yaw);
	camera->view.w = 0.0f;
}

extern void cameraInit(PerspectiveCamera* camera, Vec4 position, Vec4 up, Vec4 view, r32 nearPlane, r32 farPlane, r32 fov)
{
	camera->position = position;
	camera->up = up;
	camera->view = view;
	camera->nearPlane = nearPlane;
	camera->farPlane = farPlane;
	camera->fov = fov;
	recalculateAngles(camera);
	recalculateViewMatrix(camera);
	recalculateProjectionMatrix(camera);
}

extern void cameraSetPosition(PerspectiveCamera* camera, Vec4 position)
{
	camera->position = position;
	recalculateViewMatrix(camera);
}

extern void cameraSetUp(PerspectiveCamera* camera, Vec4 up)
{
	camera->up = up;
	recalculateViewMatrix(camera);
}

extern void cameraSetView(PerspectiveCamera* camera, Vec4 view)
{
	camera->view = view;
	recalculateAngles(camera);
	recalculateViewMatrix(camera);
}

extern void cameraSetNearPlane(PerspectiveCamera* camera, r32 nearPlane)
{
	camera->nearPlane = nearPlane;
	recalculateProjectionMatrix(camera);
}

extern void cameraSetFarPlane(PerspectiveCamera* camera, r32 farPlane)
{
	camera->farPlane = farPlane;
	recalculateProjectionMatrix(camera);
}

extern void cameraIncPitch(PerspectiveCamera* camera, r32 angle)
{
	camera->pitch += angle;
	truncateAngles(camera);
	recalculateView(camera);
	recalculateViewMatrix(camera);
}

extern void cameraIncYaw(PerspectiveCamera* camera, r32 angle)
{
	camera->yaw += angle;
	truncateAngles(camera);
	recalculateView(camera);
	recalculateViewMatrix(camera);
}

extern void cameraSetFov(PerspectiveCamera* camera, r32 fov)
{
	camera->fov = fov;
	recalculateProjectionMatrix(camera);
}

extern void cameraForceMatrixRecalculation(PerspectiveCamera* camera)
{
	recalculateViewMatrix(camera);
	recalculateProjectionMatrix(camera);
}