#include <GLFW/glfw3.h>
#include "core.h"
#include "graphics.h"

Entity testEntity;
Shader phongShader;
PerspectiveCamera camera;
Light light;

extern void coreInit()
{
	phongShader = graphicsShaderCreate("./shaders/phong_shader.vs", "./shaders/phong_shader.fs");
	cameraInit(&camera, (Vec4){0.0f, 0.0f, 1.0f, 1.0f}, (Vec4){0.0f, 1.0f, 0.0f, 0.0f}, (Vec4){0.0f, 0.0f, -1.0f, 0.0f},
		-0.01f, -1000.0f, 45.0f);
	Mesh m = graphicsMeshCreateFromObj("./res/bunny.obj");
	graphicsEntityCreate(&testEntity, m, (Vec4){0.0f, 0.0f, 0.0f, 1.0f}, (Vec3){0.0f, 0.0f, 0.0f}, (Vec3){1.0f, 1.0f, 1.0f});

	graphicsLightCreate(&light, (Vec4){1.0f, 0.0f, 0.0f, 1.0f}, (Vec4){0.4f, 0.4f, 0.4f, 1.0f},
		(Vec4){1.0f, 1.0f, 1.0f, 1.0f}, (Vec4){1.0f, 1.0f, 1.0f, 1.0f});
}

extern void coreDestroy()
{

}

extern void coreUpdate(r32 deltaTime)
{

}

extern void coreRender()
{
	graphicsEntityRenderPhongShader(phongShader, &camera, &testEntity, &light);
}

extern void coreInputProcess(boolean* keyState, r32 deltaTime)
{
	r32 movementSpeed = 12.0f;
	r32 rotationSpeed = 3.0f;

	if (keyState[GLFW_KEY_LEFT_SHIFT])
		movementSpeed = 0.5f;
	if (keyState[GLFW_KEY_RIGHT_SHIFT])
		movementSpeed = 0.1f;

	if (keyState[GLFW_KEY_W])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(movementSpeed * deltaTime, gmNormalizeVec4(camera.view))));
	if (keyState[GLFW_KEY_S])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(-movementSpeed * deltaTime, gmNormalizeVec4(camera.view))));
	if (keyState[GLFW_KEY_A])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(-movementSpeed * deltaTime, gmNormalizeVec4(camera.xAxis))));
	if (keyState[GLFW_KEY_D])
		cameraSetPosition(&camera, gmAddVec4(camera.position, gmScalarProductVec4(movementSpeed * deltaTime, gmNormalizeVec4(camera.xAxis))));
}

extern void coreMouseChangeProcess(r64 xPos, r64 yPos)
{
	static r64 xPosOld, yPosOld;
	// This constant is basically the mouse sensibility.
	// @TODO: Allow mouse sensibility to be configurable.
	static const r32 cameraMouseSpeed = 0.001f;

	r64 xDifference = xPos - xPosOld;
	r64 yDifference = yPos - yPosOld;

	r32 pitchAngle = -cameraMouseSpeed * (float)xDifference;
	r32 yawAngle = cameraMouseSpeed * (float)yDifference;

	cameraIncPitch(&camera, pitchAngle);
	cameraIncYaw(&camera, yawAngle);

	xPosOld = xPos;
	yPosOld = yPos;
}

extern void coreMouseClickProcess(s32 button, s32 action, r64 xPos, r64 yPos)
{

}

extern void coreScrollChangeProcess(r64 xOffset, r64 yOffset)
{

}

extern void coreWindowResizeProcess(s32 width, s32 height)
{

}