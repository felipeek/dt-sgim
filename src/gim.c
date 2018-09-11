#define DYNAMIC_ARRAY_IMPLEMENT
#include "gim.h"
#include "float.h"
#include <stdio.h>

// Parses a .gim file into a GeometryImage
extern GeometryImage gimParseGeometryImageFile(const u8* path)
{
	FILE* file = fopen(path, "rb");
	GeometryImage gim = {0};

	gim.img.channels = 3;
	fread(&gim.img.width, sizeof(s32), 1, file);
	fread(&gim.img.height, sizeof(s32), 1, file);
	gim.img.data = malloc(sizeof(r32) * gim.img.width * gim.img.height * gim.img.channels);
	fread(gim.img.data, sizeof(r32) * gim.img.width * gim.img.height * gim.img.channels, 1, file);
	fclose(file);

	return gim;
}

// This function updates geometry image's vertices and indexes based on its img
extern void gimGeometryImageUpdate3D(GeometryImage* gim)
{
	// Release old indexes and vertices
	if (gim->indexes)
		array_release(gim->indexes);
	if (gim->vertices)
		array_release(gim->vertices);

	// vertexMap is a temporary array which links each vertex to its index inside the vertex array
	s32* vertexMap = malloc(sizeof(s32) * gim->img.width * gim->img.height);

	// Create arrays
	gim->vertices = array_create(Vertex, 1);
	gim->indexes = array_create(u32, 1);
	gim->normals = malloc(sizeof(Vec4) * gim->img.width * gim->img.height);

	// Initializes vertexMap with -1
	for (s32 i = 0; i < gim->img.width * gim->img.height; ++i)
		vertexMap[i] = -1;

	// Fill vertex array
	for (s32 i = 0; i < gim->img.width * gim->img.height; ++i)
	{
		// Get next vertex
		Vec3 vertexPosition = *(Vec3*)&gim->img.data[i * gim->img.channels];

		// If vertex was not put inside array yet
		if (vertexMap[i] == -1)
		{
			Vertex newVertex;

			s32 x = i % gim->img.width;
			s32 y = i / gim->img.width;

			// Push vertex
			newVertex.position = (Vec4) { vertexPosition.x, vertexPosition.y, vertexPosition.z, 1.0f };
			newVertex.normal = (Vec4) { 0.0f, 0.0f, 0.0f, 0.0f };
			newVertex.textureCoordinates.x = (r32)x / (gim->img.width - 1);
			newVertex.textureCoordinates.y = (r32)y / (gim->img.height - 1);
			vertexMap[i] = array_push(gim->vertices, &newVertex);

			// If this is a border vertex, we check if there is other equal vertices
			if (x == 0 || x == gim->img.width - 1 || y == 0 || y == gim->img.height - 1)
			{
				for (s32 j = i + 1; j < gim->img.width * gim->img.height; ++j)
				{
					Vec3 currentVertexPosition = *(Vec3*)&gim->img.data[j * gim->img.channels];

					// If an equal vertex was found, we make sure the same vertex will be used
					if (vertexPosition.x == currentVertexPosition.x &&
						vertexPosition.y == currentVertexPosition.y &&
						vertexPosition.z == currentVertexPosition.z)
						vertexMap[j] = vertexMap[i];
				}
			}
		}
	}

	// Fill indexes array
	for (s32 i = 0; i < gim->img.height - 1; ++i)
		for (s32 j = 0; j < gim->img.width - 1; ++j)
		{
			// Get surrounding vertices
			s32 bottomLeftVertexIndex = vertexMap[i * gim->img.width + j];
			s32 topLeftVertexIndex = vertexMap[(i + 1) * gim->img.width + j];
			s32 bottomRightVertexIndex = vertexMap[i * gim->img.width + (j + 1)];
			s32 topRightVertexIndex = vertexMap[(i + 1) * gim->img.width + (j + 1)];
			Vec4 bottomLeftVertex = (gim->vertices[bottomLeftVertexIndex]).position;
			Vec4 topLeftVertex = (gim->vertices[topLeftVertexIndex]).position;
			Vec4 bottomRightVertex = (gim->vertices[bottomRightVertexIndex]).position;
			Vec4 topRightVertex = (gim->vertices[topRightVertexIndex]).position;

			r32 bottomLeftTopRightDiagonal = gmLengthVec4(gmSubtractVec4(bottomLeftVertex, topRightVertex));
			r32 bottomRightTopLeftDiagonal = gmLengthVec4(gmSubtractVec4(bottomRightVertex, topLeftVertex));

			// @TODO: Check this
			//if (bottomLeftTopRightDiagonal < bottomRightTopLeftDiagonal)
			if (1)
			{
				array_push(gim->indexes, &bottomLeftVertexIndex);
				array_push(gim->indexes, &topRightVertexIndex);
				array_push(gim->indexes, &topLeftVertexIndex);

				array_push(gim->indexes, &bottomLeftVertexIndex);
				array_push(gim->indexes, &bottomRightVertexIndex);
				array_push(gim->indexes, &topRightVertexIndex);
			}
			else
			{
				array_push(gim->indexes, &bottomRightVertexIndex);
				array_push(gim->indexes, &topLeftVertexIndex);
				array_push(gim->indexes, &bottomLeftVertexIndex);

				array_push(gim->indexes, &bottomRightVertexIndex);
				array_push(gim->indexes, &topRightVertexIndex);
				array_push(gim->indexes, &topLeftVertexIndex);
			}
		}

	// Calculate normals
	size_t indexesLength = array_get_length(gim->indexes);
	for (size_t i = 0; i < indexesLength; i += 3)
	{
		Vertex* vertexA, *vertexB, *vertexC;
		DiscreteVec3 index = *(DiscreteVec3*)&gim->indexes[i];

		// Find vertices
		vertexA = gim->vertices + index.x;
		vertexB = gim->vertices + index.y;
		vertexC = gim->vertices + index.z;

		// Manually calculate triangle's normal
		Vec3 A = (Vec3) {vertexA->position.x, vertexA->position.y, vertexA->position.z};
		Vec3 B = (Vec3) {vertexB->position.x, vertexB->position.y, vertexB->position.z};
		Vec3 C = (Vec3) {vertexC->position.x, vertexC->position.y, vertexC->position.z};
		Vec3 firstEdge = gmSubtractVec3(B, A);
		Vec3 secondEdge = gmSubtractVec3(C, A);
		Vec3 _normal = gmCrossProduct(firstEdge, secondEdge);
		Vec4 normal = (Vec4) { _normal.x, _normal.y, _normal.z, 0.0f };

		// Assign normals
		vertexA->normal = gmAddVec4(vertexA->normal, normal);
		vertexB->normal = gmAddVec4(vertexB->normal, normal);
		vertexC->normal = gmAddVec4(vertexC->normal, normal);
	}

	// Normalize normals
	size_t verticesLength = array_get_length(gim->vertices);
	for (s32 i = 0; i < verticesLength; ++i)
		gim->vertices[i].normal = gmNormalizeVec4(gim->vertices[i].normal);

	// Fill gim's normals
	for (s32 i = 0; i < gim->img.height; ++i)
		for (s32 j = 0; j < gim->img.width; ++j)
			gim->normals[i * gim->img.width + j] = gim->vertices[vertexMap[i * gim->img.width + j]].normal;

	// Free vertexMap
	free(vertexMap);
}

// Creates a mesh ready to render based on the geometry image
extern Mesh gimGeometryImageToMesh(const GeometryImage* gim, Vec4 color)
{
	Mesh mesh;

	NormalMappingInfo normalInfo;
	normalInfo.useNormalMap = false;

	mesh = graphicsMeshCreateWithColor(
		gim->vertices,
		array_get_length(gim->vertices),
		gim->indexes,
		array_get_length(gim->indexes),
		&normalInfo,
		color);

	return mesh;
}

extern void gimExportToObjFile(const GeometryImage* gim, const s8* objPath)
{
	FILE *fp = fopen(objPath, "w+");
	size_t verticesSize = array_get_length(gim->vertices);
	size_t indexesSize = array_get_length(gim->indexes);

	for (s32 i = 0; i < verticesSize; ++i)
		fprintf(fp, "v %f %f %f\n",
			gim->vertices[i].position.x,
			gim->vertices[i].position.y,
			gim->vertices[i].position.z);

	for (s32 i = 0; i < indexesSize; i+=3)
		fprintf(fp, "f %d %d %d\n",
			gim->indexes[i] + 1, 
			gim->indexes[i+1] + 1, 
			gim->indexes[i+2] + 1);

	fclose(fp);
}

static r32 normalize(r32 v, r32 min, r32 max)
{
	return (v - min) / (max - min);
}

extern FloatImageData gimNormalizeImageForVisualization(const FloatImageData* gimImage)
{
	r32 rMin = FLT_MAX, gMin = FLT_MAX, bMin = FLT_MAX;
	r32 rMax = -FLT_MAX, gMax = -FLT_MAX, bMax = -FLT_MAX;

	for (s32 i = 0; i < gimImage->height; ++i)
		for (s32 j = 0; j < gimImage->width; ++j)
		{
			r32 r = gimImage->data[gimImage->width * gimImage->channels * i + gimImage->channels * j + 0];
			r32 g = gimImage->data[gimImage->width * gimImage->channels * i + gimImage->channels * j + 1];
			r32 b = gimImage->data[gimImage->width * gimImage->channels * i + gimImage->channels * j + 2];

			if (r < rMin) rMin = r;
			if (g < gMin) gMin = g;
			if (b < bMin) bMin = b;
			if (r > rMax) rMax = r;
			if (g > gMax) gMax = g;
			if (b > bMax) bMax = b;
		}

	FloatImageData result;
	result.data = malloc(gimImage->width * gimImage->height * 4 * sizeof(r32));
	result.height = gimImage->height;
	result.width = gimImage->width;
	result.channels = 4;

	for (s32 i = 0; i < gimImage->height; ++i)
		for (s32 j = 0; j < gimImage->width; ++j)
		{
			r32 r = gimImage->data[gimImage->width * gimImage->channels * i + gimImage->channels * j + 0];
			r32 g = gimImage->data[gimImage->width * gimImage->channels * i + gimImage->channels * j + 1];
			r32 b = gimImage->data[gimImage->width * gimImage->channels * i + gimImage->channels * j + 2];

			result.data[result.width * result.channels * i + result.channels * j + 0] = normalize(r, rMin, rMax);
			result.data[result.width * result.channels * i + result.channels * j + 1] = normalize(g, gMin, gMax);
			result.data[result.width * result.channels * i + result.channels * j + 2] = normalize(b, bMin, bMax);
			result.data[result.width * result.channels * i + result.channels * j + 3] = 1.0f;
		}

	return result;
}

extern void gimNormalizeAndSave(const GeometryImage* gim, const s8* imagePath)
{
	FloatImageData normalizedTexture = gimNormalizeImageForVisualization(&gim->img);
	graphicsFloatImageSave(imagePath, &normalizedTexture);
	graphicsFloatImageFree(&normalizedTexture);
}
/*
extern FloatImageData gimAddNoise(const FloatImageData* gim, r32 noiseWeight, r32 noiseThreshold, const Vec4* meshNormals)
{
	FloatImageData noisedGim = graphicsFloatImageCopy(gim);

	r32* hdt = malloc(sizeof(r32) * gim->width * gim->height);
	r32* vdt = malloc(sizeof(r32) * gim->width * gim->height);
	r32 noiseWeightNormalized = noiseWeight / 1000.0f;
	dtFillDomainTransforms(gim, meshNormals, CURVATURE_FILTER, vdt, hdt, 100.0f, 1.0f);

	for (s32 i = 0; i < noisedGim.height; ++i)
		for (s32 j = 0; j < noisedGim.width; ++j)
		{
			r32 horizontalCurvature = hdt[i * noisedGim.width + j];
			r32 verticalCurvature = vdt[i * noisedGim.width + j];
			r32 curvatureValue = (horizontalCurvature + verticalCurvature) / 2.0f;

			if (curvatureValue < noiseThreshold)
			{
				Vec3 vertex = *(Vec3*)&noisedGim.data[i * noisedGim.width * noisedGim.channels + j * noisedGim.channels];
				vertex.x += getRandomNumberN() * noiseWeightNormalized;
				vertex.y += getRandomNumberN() * noiseWeightNormalized;
				vertex.z += getRandomNumberN() * noiseWeightNormalized;
				*(Vec3*)&noisedGim.data[i * noisedGim.width * noisedGim.channels + j * noisedGim.channels] = vertex;
			}
		}

	free(hdt);
	free(vdt);
	return noisedGim;
}*/

extern void gimFreeGeometryImage(GeometryImage* gim)
{
	graphicsFloatImageFree(&gim->img);
	if (gim->indexes)
		array_release(gim->indexes);
	if (gim->vertices)
		array_release(gim->vertices);
	if (gim->normals)
		free(gim->normals);
}

static void checkNumberOfMatches(const FloatImageData* gimImage, s32 x, s32 y)
{
	DiscreteVec2* matches = array_create(DiscreteVec2, 1);
	Vec3 vertex = *(Vec3*)&gimImage->data[y * gimImage->width * gimImage->channels + x * gimImage->channels];

	for (s32 i = 0; i < gimImage->height; ++i)
	{
		DiscreteVec2 currentPosition = (DiscreteVec2) {i, 0};
		Vec3 currentVertex = *(Vec3*)&gimImage->data[currentPosition.y * gimImage->width * gimImage->channels + currentPosition.x * gimImage->channels];
		if (gmEqualVec3(currentVertex, vertex))
			array_push(matches, &currentPosition);

		currentPosition = (DiscreteVec2) {i, gimImage->width - 1};
		currentVertex = *(Vec3*)&gimImage->data[currentPosition.y * gimImage->width * gimImage->channels + currentPosition.x * gimImage->channels];
		if (gmEqualVec3(currentVertex, vertex))
			array_push(matches, &currentPosition);
	}

	for (s32 j = 1; j < gimImage->width - 1; ++j)
	{
		DiscreteVec2 currentPosition = (DiscreteVec2) {0, j};
		Vec3 currentVertex = *(Vec3*)&gimImage->data[currentPosition.y * gimImage->width * gimImage->channels + currentPosition.x * gimImage->channels];
		if (gmEqualVec3(currentVertex, vertex))
			array_push(matches, &currentPosition);

		currentPosition = (DiscreteVec2) {gimImage->height - 1, j};
		currentVertex = *(Vec3*)&gimImage->data[currentPosition.y * gimImage->width * gimImage->channels + currentPosition.x * gimImage->channels];
		if (gmEqualVec3(currentVertex, vertex))
			array_push(matches, &currentPosition);
	}

	s32 numberOfMatches = array_get_length(matches);

	if (numberOfMatches != 2)
	{
		printf("Found Odd Pixel: <%d, %d>\n", x, y);
		printf("Matches:\n");
		for (s32 i = 0; i < numberOfMatches; ++i)
		{
			DiscreteVec2 match = matches[i];
			printf("\t<%d, %d>\n", match.x, match.y);
		}
	}

	array_release(matches);
}

// Auxiliar function to detect errors in a spherical geometry image
extern void gimCheckGeometryImage(const FloatImageData* gimImage)
{
	for (s32 i = 0; i < gimImage->height; ++i)
	{
		checkNumberOfMatches(gimImage, 0, i);
		checkNumberOfMatches(gimImage, gimImage->width - 1, i);
	}

	for (s32 j = 1; j < gimImage->width - 1; ++j)
	{
		checkNumberOfMatches(gimImage, j, 0);
		checkNumberOfMatches(gimImage, j, gimImage->height - 1);
	}
}

extern GeometryImage gimCopyGeometryImage(const GeometryImage* gim)
{
	GeometryImage copy = {0};
	copy.img = graphicsFloatImageCopy(&gim->img);
	if (gim->indexes)
	{
		copy.indexes = array_create(u32, 1);
		array_allocate(copy.indexes, array_get_length(gim->indexes));
		memcpy(copy.indexes, gim->indexes, array_get_length(gim->indexes) * sizeof(u32));
	}
	if (gim->vertices)
	{
		copy.vertices = array_create(Vertex, 1);
		array_allocate(copy.vertices, array_get_length(gim->vertices));
		memcpy(copy.vertices, gim->vertices, array_get_length(gim->vertices) * sizeof(Vertex));
	}
	if (gim->normals)
	{
		copy.normals = malloc(sizeof(Vec4) * gim->img.width * gim->img.height);
		memcpy(copy.normals, gim->normals, sizeof(Vec4) * gim->img.width * gim->img.height);
	}
	return copy;
}