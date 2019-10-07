#include <GLFW/glfw3.h>
#include "core.h"
#include "filter.h"
#include "domain_transform.h"
#include "menu.h"
#include "parametrization.h"
#include <math.h>
#include "obj.h"
#include <stdio.h>

#define PHONG_VERTEX_SHADER_PATH "./shaders/phong_shader.vs"
#define PHONG_FRAGMENT_SHADER_PATH "./shaders/phong_shader.fs"
#define GIM_PARAMETRIZATION_DEFAULT_PATH "./res/gim_result.gim"
#define GIM_ENTITY_COLOR (Vec4) {1.0f, 1.0f, 1.0f, 1.0f}

static GeometryImage originalGim, noisyGim, filteredGim;
static Entity gimEntity;
static Shader phongShader;
static PerspectiveCamera camera;
static Light* lights;

typedef enum {
	UNKNOWN,
	WAVEFRONT,
	GIM
} MeshFileExt;

static void updateFilteredGimMesh()
{
	Mesh m = gimGeometryImageToMesh(&filteredGim, GIM_ENTITY_COLOR);
	graphicsEntityMeshReplace(&gimEntity, m, false, false);
}

static void filterCurvatureCallback(r32 ss, r32 sr, s32 n, r32 blurSS)
{
	// Fill blur information
	BlurNormalsInformation blurNormalsInformation = {0};
	blurNormalsInformation.shouldBlur = true;

	blurNormalsInformation.blurSS = blurSS;

	// @TEMPORARY
	// Fix blurSS value depending on SR
	// Tests:
	// 10.0f = Excelent curvature preservation and very bad mesh preservation
	// 30.0f = Good curvature preservation and bad mesh preservation
	// 50.0f = Regular curvature preservation and regular mesh preservation
	// 70.0f+ = Bad curvature preservation and good mesh preservation
	r32 variance = 30.0f * sr;
	blurNormalsInformation.blurSS = expf(-sqrtf(2.0f) / variance);
	//printf("normalsBlurSS: %.4f\n", blurInformation.normalsBlurSS);

	gimFreeGeometryImage(&filteredGim);
	filteredGim = filterGeometryImageFilter(&noisyGim, n, ss, sr, CURVATURE_FILTER, &blurNormalsInformation, true);
	gimGeometryImageUpdate3D(&filteredGim);
	updateFilteredGimMesh();
}

static void textureChangeSolidCallback()
{
	graphicsMeshChangeColor(&gimEntity.mesh, GIM_ENTITY_COLOR, false);
}

static void textureChangeCurvatureCallback(r32 curvatureSpatialFactor, r32 curvatureRangeFactor, r32 normalsBlurSpatialFactor)
{
	// Fill blur information
	BlurNormalsInformation blurNormalsInformation = {0};
	blurNormalsInformation.shouldBlur = true;

	blurNormalsInformation.blurSS = normalsBlurSpatialFactor;

	FloatImageData curvatureImage = dtGenerateDomainTransformsImage(&noisyGim, curvatureSpatialFactor, curvatureRangeFactor, &blurNormalsInformation);
	FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
	graphicsFloatImageSave("./res/curvatures.bmp", &normalizedCurvatureImage);
	graphicsFloatImageFree(&curvatureImage);
	s32 currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
	//gimCheckGeometryImage(&normalizedCurvatureImage);
	graphicsFloatImageFree(&normalizedCurvatureImage);
	if (currentTexture != -1) graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, true);
}

static void textureChangeNormalsCallback(r32 normalsBlurSpatialFactor)
{
	FloatImageData curvatureImage = dtGenerateNormalImage(&noisyGim, true, normalsBlurSpatialFactor);
	FloatImageData normalizedCurvatureImage = gimNormalizeImageForVisualization(&curvatureImage);
	graphicsFloatImageSave("./res/normals.bmp", &normalizedCurvatureImage);
	graphicsFloatImageFree(&curvatureImage);
	s32 currentTexture = graphicsTextureCreateFromFloatData(&normalizedCurvatureImage);
	//gimCheckGeometryImage(&normalizedCurvatureImage);
	graphicsFloatImageFree(&normalizedCurvatureImage);
	if (currentTexture != -1) graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, true);
}

static void textureChangeCustomCallback(char* customTexturePath)
{
	s32 currentTexture = graphicsTextureCreate(customTexturePath);
	//gimCheckGeometryImage(&normalizedCurvatureImage);
	if (currentTexture != -1) graphicsMeshChangeDiffuseMap(&gimEntity.mesh, currentTexture, true);
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
	menuRegisterFilterCallBack(filterCurvatureCallback);
	menuRegisterTextureChangeSolidCallBack(textureChangeSolidCallback);
	menuRegisterTextureChangeCurvatureCallBack(textureChangeCurvatureCallback);
	menuRegisterTextureChangeNormalsCallBack(textureChangeNormalsCallback);
	menuRegisterTextureChangeCustomCallBack(textureChangeCustomCallback);
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

static int loadGeometryImage(const s8* gimPath)
{
	// Parse the original geometry image
	if (gimParseGeometryImageFile(&originalGim, gimPath))
		return -1;

	gimCheckGeometryImage(&originalGim.img);
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
	return 0;
}

static MeshFileExt findMeshPathExtension(const s8* gimPath)
{
	s32 pathLen = strlen(gimPath);
	if (pathLen < 4) return UNKNOWN;
	const s8* ext = gimPath + pathLen - 4;
	if (!strcmp(ext, ".obj")) {
		return WAVEFRONT;
	} else if (!strcmp(ext, ".gim")) {
		return GIM;
	}

	return UNKNOWN;
}

Entity e;
extern int coreInit(const s8* meshFilePath)
{
	// Register menu callbacks
	registerMenuCallbacks();
	// Create shader
	phongShader = graphicsShaderCreate(PHONG_VERTEX_SHADER_PATH, PHONG_FRAGMENT_SHADER_PATH);
	// Create camera
	camera = createCamera();
	// Create light
	lights = createLights();

	MeshFileExt gimPathExt = findMeshPathExtension(meshFilePath);
	const s8* gimPath;

	switch (gimPathExt) {
		case WAVEFRONT: {
			GeometryImage gim;
			Vertex* vertices;
			u32* indexes;
			if (objParse(meshFilePath, &vertices, &indexes))
				return -1;
			if (paramObjToGeometryImage(indexes, vertices, GIM_PARAMETRIZATION_DEFAULT_PATH))
			{
				array_release(vertices);
				array_release(indexes);
				return -1;
			}
			Mesh m = graphicsMeshCreateWithColor(vertices, array_get_length(vertices), indexes, array_get_length(indexes),
				NULL, (Vec4){1.0f, 0.0f, 0.0f, 1.0f});
			graphicsEntityCreate(&e, m, (Vec4){0.0f, 0.0f, 0.0f, 1.0f}, (Vec3){0.0f, 0.0f, 0.0f}, (Vec3){1.0f, 1.0f, 1.0f});
			array_release(vertices);
			array_release(indexes);
			gimPath = GIM_PARAMETRIZATION_DEFAULT_PATH;
		} break;
		case GIM: {
			gimPath = meshFilePath;
		} break;
		default: {
			fprintf(stderr, "Unrecognized mesh file format. (need .gim or .obj)");
			return -1;
		}
	}

	// Load geometry image	
	if (loadGeometryImage(gimPath))
		return -1;

	return 0;
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
	//graphicsEntityRenderPhongShader(phongShader, &camera, &e, lights);
	//graphicsEntityRenderPhongShader(phongShader, &camera, &entity, lights);
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