#include <GLFW/glfw3.h>
#include "core.h"
#include "filter.h"
#include "domain_transform.h"
#include "menu.h"

#define PHONG_VERTEX_SHADER_PATH "./shaders/phong_shader.vs"
#define PHONG_FRAGMENT_SHADER_PATH "./shaders/phong_shader.fs"
#define GIM_ENTITY_COLOR (Vec4) {1.0f, 1.0f, 1.0f, 1.0f}

static GeometryImage originalGim, noisyGim, filteredGim;
static Entity gimEntity;
static Shader phongShader;
static PerspectiveCamera camera;
static Light* lights;

static void updateFilteredGimMesh()
{
	Mesh m = gimGeometryImageToMesh(&filteredGim, GIM_ENTITY_COLOR);
	graphicsEntityMeshReplace(&gimEntity, m, false, false);
}

static void filterRecursiveCallback(r32 ss, s32 n)
{
	gimFreeGeometryImage(&filteredGim);
	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, 0.0f, RECURSIVE_FILTER, 0);
	gimGeometryImageUpdate3D(&filteredGim);
	updateFilteredGimMesh();
}

static void filterCurvatureCallback(r32 ss, r32 sr, s32 n, s32 curvBlurMode, r32 curvBlurSS, r32 curvBlurSR,
	s32 normalsBlurMode, r32 normalsBlurSS, r32 normalsBlurSR)
{
	// Fill blur information
	BlurInformation blurInformation = {0};
	if (curvBlurMode != 0)
		blurInformation.blurCurvatures = true;
	if (normalsBlurMode != 0)
		blurInformation.blurNormals = true;

	if (curvBlurMode == 1)
		blurInformation.blurCurvaturesMode = RECURSIVE_FILTER;
	else if (curvBlurMode == 2)
		blurInformation.blurCurvaturesMode = DISTANCE_FILTER;
	
	if (normalsBlurMode == 1)
		blurInformation.blurNormalsMode = RECURSIVE_FILTER;
	else if (normalsBlurMode == 2)
		blurInformation.blurNormalsMode = DISTANCE_FILTER;

	blurInformation.curvatureBlurSS = curvBlurSS;
	blurInformation.curvatureBlurSR = curvBlurSR;

	blurInformation.normalsBlurSS = normalsBlurSS;
	blurInformation.normalsBlurSR = normalsBlurSR;

	gimFreeGeometryImage(&filteredGim);
	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, sr, CURVATURE_FILTER, &blurInformation);
	gimGeometryImageUpdate3D(&filteredGim);
	updateFilteredGimMesh();
}

static void filterDistanceCallback(r32 ss, r32 sr, s32 n)
{
	gimFreeGeometryImage(&filteredGim);
	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, sr, DISTANCE_FILTER, 0);
	gimGeometryImageUpdate3D(&filteredGim);
	updateFilteredGimMesh();
}

static void textureChangeSolidCallback()
{
	graphicsMeshChangeColor(&gimEntity.mesh, GIM_ENTITY_COLOR, false);
}

static void textureChangeDistanceCallback(r32 distanceSpatialFactor, r32 distanceRangeFactor)
{
	// Fill blur information
	BlurInformation blurInformation = {0};
	blurInformation.blurCurvatures = false;
	blurInformation.blurNormals = false;

	FloatImageData curvatureImage = dtGenerateDomainTransformsImage(&noisyGim, DISTANCE_FILTER, distanceSpatialFactor,
		distanceRangeFactor, &blurInformation);
	FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
	graphicsFloatImageSave("./res/distances.bmp", &normalizedCurvatureImage);
	graphicsFloatImageFree(&curvatureImage);
	s32 currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
	//gimCheckGeometryImage(&normalizedCurvatureImage);
	graphicsFloatImageFree(&normalizedCurvatureImage);
	graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, true);
}

static void textureChangeCurvatureCallback(r32 curvatureSpatialFactor, r32 curvatureRangeFactor, s32 curvatureBlurMode,
	r32 curvatureBlurSpatialFactor, r32 curvatureBlurRangeFactor, s32 normalsBlurMode, r32 normalsBlurSpatialFactor, r32 normalsBlurRangeFactor)
{
	// Fill blur information
	BlurInformation blurInformation = {0};
	if (curvatureBlurMode != 0)
		blurInformation.blurCurvatures = true;
	if (normalsBlurMode != 0)
		blurInformation.blurNormals = true;

	if (curvatureBlurMode == 1)
		blurInformation.blurCurvaturesMode = RECURSIVE_FILTER;
	else if (curvatureBlurMode == 2)
		blurInformation.blurCurvaturesMode = DISTANCE_FILTER;
	
	if (normalsBlurMode == 1)
		blurInformation.blurNormalsMode = RECURSIVE_FILTER;
	else if (normalsBlurMode == 2)
		blurInformation.blurNormalsMode = DISTANCE_FILTER;

	blurInformation.curvatureBlurSS = curvatureBlurSpatialFactor;
	blurInformation.curvatureBlurSR = curvatureBlurRangeFactor;

	blurInformation.normalsBlurSS = normalsBlurSpatialFactor;
	blurInformation.normalsBlurSR = normalsBlurRangeFactor;

	FloatImageData curvatureImage = dtGenerateDomainTransformsImage(&noisyGim, CURVATURE_FILTER, curvatureSpatialFactor,
		curvatureRangeFactor, &blurInformation);
	FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
	graphicsFloatImageSave("./res/curvatures.bmp", &normalizedCurvatureImage);
	graphicsFloatImageFree(&curvatureImage);
	s32 currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
	//gimCheckGeometryImage(&normalizedCurvatureImage);
	graphicsFloatImageFree(&normalizedCurvatureImage);
	graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, true);
}

static void textureChangeNormalsCallback(s32 normalsBlurMode, r32 normalsBlurSpatialFactor, r32 normalsBlurRangeFactor)
{
	boolean shouldBlurNormals = normalsBlurMode > 0;
	FilterMode blurMode = (normalsBlurMode == 1) ? RECURSIVE_FILTER : DISTANCE_FILTER;
	FloatImageData curvatureImage = dtGenerateNormalImage(&noisyGim, shouldBlurNormals, blurMode, normalsBlurSpatialFactor, normalsBlurRangeFactor);
	FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
	graphicsFloatImageSave("./res/normals.bmp", &normalizedCurvatureImage);
	graphicsFloatImageFree(&curvatureImage);
	s32 currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
	//gimCheckGeometryImage(&normalizedCurvatureImage);
	graphicsFloatImageFree(&normalizedCurvatureImage);
	graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, true);
}

static void noiseGeneratorCallback(r32 intensity)
{
	gimFreeGeometryImage(&noisyGim);
	gimFreeGeometryImage(&filteredGim);
	noisyGim = gimAddNoise(&originalGim, intensity);
	//gimCheckGeometryImage(&noisyGim.img);
	gimGeometryImageUpdate3D(&noisyGim);
	filteredGim = gimCopyGeometryImage(&noisyGim, true);
	updateFilteredGimMesh();
}

static void exportWavefrontCallback()
{
	gimExportToObjFile(&filteredGim, "./res/output.obj");
}

static void exportPointCloudCallback()
{
	gimExportToPointCloudFile(&filteredGim, "./res/point_cloud.txt");
}

static void registerMenuCallbacks()
{
	menuRegisterRecursiveFilterCallBack(filterRecursiveCallback);
	menuRegisterCurvatureFilterCallBack(filterCurvatureCallback);
	menuRegisterDistanceFilterCallBack(filterDistanceCallback);
	menuRegisterTextureChangeSolidCallBack(textureChangeSolidCallback);
	menuRegisterTextureChangeDistanceCallBack(textureChangeDistanceCallback);
	menuRegisterTextureChangeCurvatureCallBack(textureChangeCurvatureCallback);
	menuRegisterTextureChangeNormalsCallBack(textureChangeNormalsCallback);
	menuRegisterNoiseGeneratorCallBack(noiseGeneratorCallback);
	menuRegisterExportWavefrontCallBack(exportWavefrontCallback);
	menuRegisterExportPointCloudCallBack(exportPointCloudCallback);
}

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

static Light* createLights()
{
	Light light;
	Light* lights = array_create(Light, 1);

	Vec4 lightPosition = (Vec4) {1.0f, 0.0f, 0.0f, 1.0f};
	Vec4 ambientColor = (Vec4) {0.1f, 0.1f, 0.1f, 1.0f};
	Vec4 diffuseColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	Vec4 specularColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	graphicsLightCreate(&light, lightPosition, ambientColor, diffuseColor, specularColor);
	array_push(lights, &light);

	lightPosition = (Vec4) {-1.0f, 0.0f, 0.0f, 1.0f};
	ambientColor = (Vec4) {0.1f, 0.1f, 0.1f, 1.0f};
	diffuseColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	specularColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	graphicsLightCreate(&light, lightPosition, ambientColor, diffuseColor, specularColor);
	array_push(lights, &light);

	lightPosition = (Vec4) {0.0f, 0.0f, 1.0f, 1.0f};
	ambientColor = (Vec4) {0.1f, 0.1f, 0.1f, 1.0f};
	diffuseColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	specularColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	graphicsLightCreate(&light, lightPosition, ambientColor, diffuseColor, specularColor);
	array_push(lights, &light);

	lightPosition = (Vec4) {0.0f, 0.0f, -1.0f, 1.0f};
	ambientColor = (Vec4) {0.1f, 0.1f, 0.1f, 1.0f};
	diffuseColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	specularColor = (Vec4) {0.5f, 0.5f, 0.5f, 1.0f};
	graphicsLightCreate(&light, lightPosition, ambientColor, diffuseColor, specularColor);
	array_push(lights, &light);
	return lights;
}

static void loadGeometryImage(const s8* gimPath)
{
	// Parse the original geometry image
	originalGim = gimParseGeometryImageFile(gimPath);
	// Update 3d information
	gimGeometryImageUpdate3D(&originalGim);
	// Copy original gim to noisy gim
	noisyGim = gimCopyGeometryImage(&originalGim, true);
	// Copy original gim to filtered gim
	filteredGim = gimCopyGeometryImage(&originalGim, true);

	// Transform filtered geometry image to a mesh
	Mesh m = gimGeometryImageToMesh(&filteredGim, GIM_ENTITY_COLOR);
	// Create filteredGim's entity
	graphicsEntityCreate(&gimEntity, m, (Vec4){0.0f, 0.0f, 0.0f, 1.0f}, (Vec3){0.0f, 0.0f, 0.0f}, (Vec3){1.0f, 1.0f, 1.0f});
}

extern void coreInit(const s8* gimPath)
{
	// Register menu callbacks
	registerMenuCallbacks();
	// Create shader
	phongShader = graphicsShaderCreate(PHONG_VERTEX_SHADER_PATH, PHONG_FRAGMENT_SHADER_PATH);
	// Create camera
	camera = createCamera();
	// Load geometry image	
	loadGeometryImage(gimPath);
	// Create light
	lights = createLights();
}

extern void coreDestroy()
{
	gimFreeGeometryImage(&originalGim);
	gimFreeGeometryImage(&noisyGim);
	gimFreeGeometryImage(&filteredGim);
	array_release(lights);
}

extern void coreUpdate(r32 deltaTime)
{

}

extern void coreRender()
{
	graphicsEntityRenderPhongShader(phongShader, &camera, &gimEntity, lights);
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
	if (keyState[GLFW_KEY_X])
	{
		if (keyState[GLFW_KEY_LEFT_SHIFT] || keyState[GLFW_KEY_RIGHT_SHIFT])
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.x -= rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
		else
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.x += rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
	}
	if (keyState[GLFW_KEY_Y])
	{
		if (keyState[GLFW_KEY_LEFT_SHIFT] || keyState[GLFW_KEY_RIGHT_SHIFT])
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.y += rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
		else
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.y -= rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
	}
	if (keyState[GLFW_KEY_Z])
	{
		if (keyState[GLFW_KEY_LEFT_SHIFT] || keyState[GLFW_KEY_RIGHT_SHIFT])
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.z += rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
		else
		{
			Vec3 rotation = gimEntity.worldRotation;
			rotation.z -= rotationSpeed * deltaTime;
			graphicsEntitySetRotation(&gimEntity, rotation);
		}
	}
	if (keyState[GLFW_KEY_L])
	{
		static boolean wireframe = false;

		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		wireframe = !wireframe;
		keyState[GLFW_KEY_L] = false;
	}
}

extern void coreMouseChangeProcess(boolean reset, r64 xPos, r64 yPos)
{
	static r64 xPosOld, yPosOld;
	// This constant is basically the mouse sensibility.
	// @TODO: Allow mouse sensibility to be configurable.
	static const r32 cameraMouseSpeed = 0.001f;

	if (!reset)
	{
		r64 xDifference = xPos - xPosOld;
		r64 yDifference = yPos - yPosOld;

		r32 pitchAngle = -cameraMouseSpeed * (float)xDifference;
		r32 yawAngle = cameraMouseSpeed * (float)yDifference;

		cameraIncPitch(&camera, pitchAngle);
		cameraIncYaw(&camera, yawAngle);
	}

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