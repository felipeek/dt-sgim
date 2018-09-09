#ifndef GIMMESH_GRAPHICS_H
#define GIMMESH_GRAPHICS_H
#include "graphics_math.h"
#include "camera.h"

typedef u32 Shader;
typedef struct VertexStruct Vertex;
typedef struct NormalMappingInfoStruct NormalMappingInfo;
typedef struct MeshStruct Mesh;
typedef struct EntityStruct Entity;
typedef struct LightStruct Light;
typedef struct ImageDataStruct ImageData;
typedef struct FloatImageDataStruct FloatImageData;
typedef struct DiffuseInfoStruct DiffuseInfo;

#pragma pack(push, 1)
struct VertexStruct
{
	Vec4 position;
	Vec4 normal;
	Vec2 textureCoordinates;
};
#pragma pack(pop)

struct NormalMappingInfoStruct
{
	boolean useNormalMap;
	boolean tangentSpace;		// @TODO: Not implemented yet.
	u32 normalMapTexture;
};

struct DiffuseInfoStruct
{
	boolean useDiffuseMap;
	u32 diffuseMap;
	Vec4 diffuseColor;
};

struct MeshStruct
{
	u32 VAO, VBO, EBO;
	s32 indexesSize;
	NormalMappingInfo normalInfo;
	DiffuseInfo diffuseInfo;
};

struct EntityStruct
{
	Mesh mesh;
	Vec4 worldPosition;
	Vec3 worldRotation;
	Vec3 worldScale;
	Mat4 modelMatrix;
};

struct LightStruct
{
	Vec4 position;
	Vec4 ambientColor;
	Vec4 diffuseColor;
	Vec4 specularColor;
};

struct ImageDataStruct
{
	u8* data;
	s32 width, height, channels;
};

struct FloatImageDataStruct
{
	r32* data;
	s32 width, height, channels;
};

extern ImageData graphicsImageLoad(const s8* imagePath);
extern FloatImageData graphicsFloatImageLoad(const s8* imagePath);
extern FloatImageData graphicsFloatImageCopy(const FloatImageData* imageData);
extern void graphicsImageFree(ImageData* imageData);
extern void graphicsFloatImageFree(FloatImageData* imageData);
extern void graphicsImageSave(const s8* imagePath, const ImageData* imageData);
extern void graphicsFloatImageSave(const s8* imagePath, const FloatImageData* imageData);
extern Shader graphicsShaderCreate(const s8* vertexShaderPath, const s8* fragmentShaderPath);
extern Mesh graphicsQuadCreateWithTexture(u32 texture);
extern Mesh graphicsQuadCreateWithColor(Vec4 color);
extern Mesh graphicsMeshCreateWithColor(Vertex* vertices, s32 verticesSize, u32* indices, s32 indicesSize, NormalMappingInfo* normalInfo, Vec4 diffuseColor);
extern Mesh graphicsMeshCreateWithTexture(Vertex* vertices, s32 verticesSize, u32* indices, s32 indicesSize, NormalMappingInfo* normalInfo, u32 diffuseMap);
// This function must be re-done. Temporary implementation.
extern Mesh graphicsMeshCreateFromObj(const s8* objPath);
extern void graphicsMeshRender(Shader shader, Mesh mesh);
// If mesh already has a diffuse map, the older diffuse map will be deleted if deleteDiffuseMap is true.
// If mesh has a color instead of a diffuse map, the mesh will lose the color and be set to use the diffuse map.
extern void graphicsMeshChangeDiffuseMap(Mesh* mesh, u32 diffuseMap, boolean deleteDiffuseMap);
// If the mesh already has a color, the older color will be deleted.
// If mesh has a diffuse map instead of a color, the diffuse map will be deleted if deleteDiffuseMap is true
// The mesh will be set to use the color.
extern void graphicsMeshChangeColor(Mesh* mesh, Vec4 color, boolean deleteDiffuseMap);
extern void graphicsEntityCreate(Entity* entity, Mesh mesh, Vec4 worldPosition, Vec3 worldRotation, Vec3 worldScale);
extern void graphicsEntityMeshReplace(Entity* entity, Mesh mesh, boolean deleteNormalMap, boolean deleteDiffuseMap);
extern void graphicsEntitySetPosition(Entity* entity, Vec4 worldPosition);
extern void graphicsEntitySetRotation(Entity* entity, Vec3 worldRotation);
extern void graphicsEntitySetScale(Entity* entity, Vec3 worldScale);
extern void graphicsEntityRenderBasicShader(Shader shader, const PerspectiveCamera* camera, const Entity* entity);
extern void graphicsEntityRenderPhongShader(Shader shader, const PerspectiveCamera* camera, const Entity* entity, const Light* light);
extern void graphicsEntityRenderGuiShader(Shader shader, const Entity* entity, s32 orthoWidth, s32 orthoHeight);
extern void graphicsLightCreate(Light* light, Vec4 position, Vec4 ambientColor, Vec4 diffuseColor, Vec4 specularColor);
extern u32 graphicsTextureCreate(const s8* texturePath);
extern u32 graphicsTextureCreateFromData(const ImageData* imageData);
extern u32 graphicsTextureCreateFromFloatData(const FloatImageData* imageData);
extern void graphicsTextureDelete(u32 textureId);
extern FloatImageData graphicsImageDataToFloatImageData(ImageData* imageData, r32* memory);
extern ImageData graphicsFloatImageDataToImageData(const FloatImageData* floatImageData, u8* memory);

#endif