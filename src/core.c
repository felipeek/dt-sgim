#include <GLFW/glfw3.h>
#include "core.h"
#include "graphics.h"
#include "gim.h"

#define PHONG_VERTEX_SHADER_PATH "./shaders/phong_shader.vs"
#define PHONG_FRAGMENT_SHADER_PATH "./shaders/phong_shader.fs"

static GeometryImage gim;
static Entity gimEntity;
static Shader phongShader;
static PerspectiveCamera camera;
static Light light;

static PerspectiveCamera createCamera()
{
	PerspectiveCamera camera;
	Vec4 cameraPosition = (Vec4) {0.0f, 0.0f, 2.0f, 1.0f};
	Vec4 cameraUp = (Vec4) {0.0f, 1.0f, 0.0f, 1.0f};
	Vec4 cameraView = (Vec4) {0.0f, 0.0f, -1.0f, 0.0f};
	r32 cameraNearPlane = -0.01f;
	r32 cameraFarPlane = -1000.0f;
	r32 cameraFov = 45.0f;
	cameraInit(&camera, cameraPosition, cameraUp, cameraView, cameraNearPlane, cameraFarPlane, cameraFov);
	return camera;
}

static Light createLight()
{
	Light light;
	Vec4 lightPosition = (Vec4) {1.0f, 0.0f, 0.0f, 1.0f};
	Vec4 ambientColor = (Vec4) {0.4f, 0.4f, 0.4f, 1.0f};
	Vec4 diffuseColor = (Vec4) {1.0f, 1.0f, 1.0f, 1.0f};
	Vec4 specularColor = (Vec4) {1.0f, 1.0f, 1.0f, 1.0f};
	graphicsLightCreate(&light, lightPosition, ambientColor, diffuseColor, specularColor);
	return light;
}

extern void coreInit(s8* gimPath)
{
	// Create shader
	phongShader = graphicsShaderCreate(PHONG_VERTEX_SHADER_PATH, PHONG_FRAGMENT_SHADER_PATH);
	// Create camera
	camera = createCamera();
	// Load geometry image	
	gim = gimParseGeometryImageFile(gimPath);
	// Update geometry image 3d properties
	gimGeometryImageUpdate3D(&gim);
	// Transform geometry image to a mesh
	Mesh m = gimGeometryImageToMesh(&gim, (Vec4) {0.0f, 0.0f, 1.0f, 1.0f});
	// Create gim's entity
	graphicsEntityCreate(&gimEntity, m, (Vec4){0.0f, 0.0f, 0.0f, 1.0f}, (Vec3){0.0f, 0.0f, 0.0f}, (Vec3){1.0f, 1.0f, 1.0f});
	// Create light
	light = createLight();
}

extern void coreDestroy()
{
	gimFreeGeometryImage(&gim);
}

extern void coreUpdate(r32 deltaTime)
{

}

extern void coreRender()
{
	graphicsEntityRenderPhongShader(phongShader, &camera, &gimEntity, &light);
}

extern void coreInputProcess(boolean* keyState, r32 deltaTime)
{
	r32 movementSpeed = 3.0f;
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