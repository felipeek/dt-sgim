#include "parametrization.h"
#include <dynamic_array.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>

static Vec3 convertToBarycentricCoordinates3D(Vec3 a, Vec3 b, Vec3 c, Vec3 p)
{
	Vec3 result;
	Vec3 v0 = gmSubtractVec3(b, a);
	Vec3 v1 = gmSubtractVec3(c, a);
	Vec3 v2 = gmSubtractVec3(p, a);
    r32 d00 = gmDotProductVec3(v0, v0);
    r32 d01 = gmDotProductVec3(v0, v1);
    r32 d11 = gmDotProductVec3(v1, v1);
    r32 d20 = gmDotProductVec3(v2, v0);
    r32 d21 = gmDotProductVec3(v2, v1);
    r32 denom = d00 * d11 - d01 * d01;
    result.y = (d11 * d20 - d01 * d21) / denom;
    result.z = (d00 * d21 - d01 * d20) / denom;
    result.x = 1.0f - result.y - result.z;
	return result;
}

static Vec3 convertToBarycentricCoordinates2D(Vec2 a, Vec2 b, Vec2 c, Vec2 p)
{
	Vec3 a3D = (Vec3){a.x, a.y, 0.0f};
	Vec3 b3D = (Vec3){b.x, b.y, 0.0f};
	Vec3 c3D = (Vec3){c.x, c.y, 0.0f};
	Vec3 p3D = (Vec3){p.x, p.y, 0.0f};
	return convertToBarycentricCoordinates3D(a3D, b3D, c3D, p3D);
}

static boolean intersectionRayTriangle(Vec3 rayOrigin, Vec3 rayVector, Vec3 vertex0, Vec3 vertex1,
	Vec3 vertex2, Vec3* outIntersectionPoint)
{
	const r32 EPSILON = 0.0000001f;
	Vec3 edge1, edge2, h, s, q;
	r32 a, f, u, v;

	edge1 = gmSubtractVec3(vertex1, vertex0);
	edge2 = gmSubtractVec3(vertex2, vertex0);
	h = gmCrossProduct(rayVector, edge2);
	a = gmDotProductVec3(edge1, h);

	if (a > -EPSILON && a < EPSILON) return false;

	f = 1 / a;
	s = gmSubtractVec3(rayOrigin, vertex0);
	u = f * (gmDotProductVec3(s, h));

	if (u < 0.0 || u > 1.0) return false;

	q = gmCrossProduct(s, edge1);
	v = f * gmDotProductVec3(rayVector, q);

	if (v < 0.0 || u + v > 1.0) return false;

	// At this stage we can compute t to find out where the intersection point is on the line.
	r32 t = f * gmDotProductVec3(edge2, q);

	if (t > EPSILON) // ray intersection
	{
		*outIntersectionPoint = gmAddVec3(rayOrigin, gmScalarProductVec3(t, rayVector));
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}

static boolean getGimPixelBySamplingMesh(Vec3* parametrizedVertices, Vertex* originalVertices, s32* indexes, Vec3 pointInSpace,
	Vec3* pixelColor)
{
	Vec3 rayVector = gmNormalizeVec3(pointInSpace);

	for (s32 i = 0; i < array_get_length(indexes); i += 3) {
		s32 index1 = indexes[i + 0];
		s32 index2 = indexes[i + 1];
		s32 index3 = indexes[i + 2];
		Vec3 v1 = parametrizedVertices[index1];
		Vec3 v2 = parametrizedVertices[index2];
		Vec3 v3 = parametrizedVertices[index3];

		Vec3 intersectionPoint;
		if (intersectionRayTriangle((Vec3){0.0f, 0.0f, 0.0f}, rayVector, v1, v2, v3, &intersectionPoint)) {
			Vec3 barycentricCoordinates = convertToBarycentricCoordinates3D(v1, v2, v3, intersectionPoint);

			Vec4 v1OriginalVertex = originalVertices[index1].position;
			Vec4 v2OriginalVertex = originalVertices[index2].position;
			Vec4 v3OriginalVertex = originalVertices[index3].position;
			Vec3 v1Contribution = gmScalarProductVec3(barycentricCoordinates.x, (Vec3){v1OriginalVertex.x, v1OriginalVertex.y, v1OriginalVertex.z});
			Vec3 v2Contribution = gmScalarProductVec3(barycentricCoordinates.y, (Vec3){v2OriginalVertex.x, v2OriginalVertex.y, v2OriginalVertex.z});
			Vec3 v3Contribution = gmScalarProductVec3(barycentricCoordinates.z, (Vec3){v3OriginalVertex.x, v3OriginalVertex.y, v3OriginalVertex.z});
			*pixelColor = gmAddVec3(gmAddVec3(v1Contribution, v2Contribution), v3Contribution);

			return true;
		}
	}

	return false;
}

static r32 scaleToRange(r32 value, r32 inMax, r32 inMin, r32 outMin, r32 outMax) {
	return ((outMax - outMin) * (value - inMin)) / (inMax - inMin) + outMin;
}

static Vec3 convertFromBarycentricToEuler3D(Vec3 a, Vec3 b, Vec3 c, Vec3 barycentricCoords) {
	Vec3 result;
	result.x = barycentricCoords.x * a.x + barycentricCoords.y * b.x + barycentricCoords.z * c.x;
	result.y = barycentricCoords.x * a.y + barycentricCoords.y * b.y + barycentricCoords.z * c.y;
	result.z = barycentricCoords.x * a.z + barycentricCoords.y * b.z + barycentricCoords.z * c.z;
	return result;
}

extern int paramObjToGeometryImage(u32* indexes, Vertex* vertices, const s8* outPath)
{
	assert(array_get_length(indexes) % 3 == 0);
	u32 numberOfFaces = array_get_length(indexes) / 3;
	u32 numberOfVertices = array_get_length(vertices);
	DiscreteVec2* E = array_create(DiscreteVec2, numberOfFaces * 3);
	DiscreteVec2* aux;

	// E = [faces([1 2],:) faces([2 3],:) faces([3 1],:)];
	for (size_t i = 0; i < numberOfFaces; ++i)
	{
		DiscreteVec2 v;
		v = (DiscreteVec2){indexes[i * 3 + 0], indexes[i * 3 + 1]};
		array_push(E, &v);
		v = (DiscreteVec2){indexes[i * 3 + 1], indexes[i * 3 + 2]};
		array_push(E, &v);
		v = (DiscreteVec2){indexes[i * 3 + 2], indexes[i * 3 + 0]};
		array_push(E, &v);
	}

	assert(array_get_length(E) == numberOfFaces * 3);
	
	// E = [E E(2:-1:1,:)]
	for (u32 i = 0; i < numberOfFaces * 3; ++i)
	{
		DiscreteVec2 current = E[i];
		current = (DiscreteVec2){current.y, current.x};
		array_push(E, &current);
	}

	// W = make_sparse( E(1,:), E(2,:), ones(size(E,2),1) );
	u32* W = calloc(sizeof(s32), numberOfVertices * numberOfVertices);
	for (u32 i = 0; i < array_get_length(E); ++i)
	{
		DiscreteVec2 current = E[i];
		W[current.y * numberOfVertices + current.x] = 1;
	}

	array_release(E);

	// d = full( sum(W,1) );
	u32* d = array_create(u32, numberOfVertices);
	for (u32 i = 0; i < numberOfVertices; ++i)
	{
		u32 currentSum = 0;
		for (u32 j = 0; j < numberOfVertices; ++j)
			if (W[i * numberOfVertices + j] == 1)
				++currentSum;
		array_push(d, &currentSum);
	}

	// tW = iD * W;
	r32* tW = calloc(sizeof(r32), numberOfVertices * numberOfVertices);
	for (u32 i = 0; i < numberOfVertices; ++i)
		for (u32 j = 0; j < numberOfVertices; ++j)
			if (W[i * numberOfVertices + j] == 1)
				tW[i * numberOfVertices + j] = 1.0f / d[i];

	array_release(d);
	free(W);
	/*
		Perform Smoothing and Projection
	*/

	Vec3* verticesTmp = array_create(Vec3, numberOfVertices);
	array_allocate(verticesTmp, numberOfVertices);
	for (u32 i = 0; i < numberOfVertices; ++i)
		verticesTmp[i] = (Vec3){vertices[i].position.x, vertices[i].position.y, vertices[i].position.z};

	// vertex1 = vertex1 - repmat( mean(vertex1,2), [1 n] );
	Vec3 mean = (Vec3){0.0f, 0.0f, 0.0f};
	for (u32 i = 0; i < numberOfVertices; ++i)
		mean = gmAddVec3(mean, verticesTmp[i]);
	mean = gmScalarProductVec3(1.0f / numberOfVertices, mean);
	for (u32 i = 0; i < numberOfVertices; ++i)
		verticesTmp[i] = gmSubtractVec3(verticesTmp[i], mean);

	assert(array_get_length(verticesTmp) == numberOfVertices);

	// vertex1 = vertex1 ./ repmat( sqrt(sum(vertex1.^2,1)), [3 1] );
	for (u32 i = 0; i < numberOfVertices; ++i)
	{
		Vec3 current = verticesTmp[i];
		r32 v = sqrtf(current.x * current.x + current.y * current.y + current.z * current.z);
		verticesTmp[i] = gmScalarProductVec3(1.0f / v, current);
	}

	u32 numberOfIterations = 50;

	Vec3* result = malloc(sizeof(Vec3) * numberOfVertices);
	for (u32 n = 0; n < numberOfIterations; ++n)
	{
		printf("Spherical Parametrization: Running iteration %d/%d...\n", n + 1, numberOfIterations);

		//vertex1 = vertex1*tW';
		for (u32 i = 0; i < numberOfVertices; ++i)
		{
			Vec3 current = (Vec3){0.0f, 0.0f, 0.0f};
			for (u32 j = 0; j < numberOfVertices; ++j)
			{
				current.x += verticesTmp[j].x * tW[i * numberOfVertices + j];
				current.y += verticesTmp[j].y * tW[i * numberOfVertices + j];
				current.z += verticesTmp[j].z * tW[i * numberOfVertices + j];
			}
			result[i] = current;
		}
		memcpy(verticesTmp, result, sizeof(Vec3) * numberOfVertices);

		// vertex1 = vertex1 ./ repmat( sqrt(sum(vertex1.^2,1)), [3 1] );
		for (u32 i = 0; i < numberOfVertices; ++i)
		{
			Vec3 current = verticesTmp[i];
			r32 v;

			// @TODO: check this...
			if (current.x == 0.0f && current.y == 0.0f && current.z == 0.0f)
				v = 1.0f;
			else
				v = sqrtf(current.x * current.x + current.y * current.y + current.z * current.z);
			
			verticesTmp[i] = gmScalarProductVec3(1.0f / v, current);
		}
	}
	free(result);
	free(tW);

	// GIM Sampling

	// ! MUST BE ODD TO PRESERVE BORDER PIXELS SYMMETRY ! //
	u32 gimSize = 15;
	// ! MUST BE ODD ! //

	Vec3* gimData = calloc(1, sizeof(Vec3) * gimSize * gimSize);
	Vec3 pixelColor;
	for (u32 y = 0; y < gimSize; ++y)
	{
		printf("Geometry Image Sampling: Running iteration %d/%d...\n", y + 1, gimSize);
		for (u32 x = 0; x < gimSize; ++x)
		{
			r32 yNormalized = scaleToRange(x, 0, gimSize - 1, -1.0f, 1.0f);// + (2.0f / gimSize) * 0.5f;
			r32 xNormalized = scaleToRange(y, 0, gimSize - 1, -1.0f, 1.0f);// + (2.0f / gimSize) * 0.5f;

			if (yNormalized >= 0.0f && xNormalized >= 0.0f)
			{
				if (xNormalized + yNormalized >= 1.0f)
				{
					// TOP-RIGHT BLUE
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){0.0f, 1.0f},
						(Vec2){1.0f, 0.0f},
						(Vec2){1.0f, 1.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){0.0f, 1.0f, 0.0f},
						(Vec3){1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, 0.0f, -1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
				else
				{
					// TOP-RIGHT RED
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){1.0f, 0.0f},
						(Vec2){0.0f, 1.0f},
						(Vec2){0.0f, 0.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, 1.0f, 0.0f},
						(Vec3){0.0f, 0.0f, 1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
			}
			else if (yNormalized >= 0.0f && xNormalized <= 0.0f)
			{
				if (yNormalized >= xNormalized + 1.0f)
				{
					// TOP-LEFT ORANGE
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){0.0f, 1.0f},
						(Vec2){-1.0f, 0.0f},
						(Vec2){-1.0f, 1.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){0.0f, 1.0f, 0.0f},
						(Vec3){-1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, 0.0f, -1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
				else
				{
					// TOP-LEFT GREEN
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){-1.0f, 0.0f},
						(Vec2){0.0f, 1.0f},
						(Vec2){0.0f, 0.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){-1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, 1.0f, 0.0f},
						(Vec3){0.0f, 0.0f, 1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
			}
			else if (yNormalized <= 0.0f && xNormalized >= 0.0f)
			{
				if (yNormalized + 1.0f >= xNormalized)
				{
					// BOTTOM-RIGHT ORANGE
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){1.0f, 0.0f},
						(Vec2){0.0f, -1.0f},
						(Vec2){0.0f, 0.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, -1.0f, 0.0f},
						(Vec3){0.0f, 0.0f, 1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
				else
				{
					// BOTTOM-RIGHT GREEN
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){0.0f, -1.0f},
						(Vec2){1.0f, 0.0f},
						(Vec2){1.0f, -1.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){0.0f, -1.0f, 0.0f},
						(Vec3){1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, 0.0f, -1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
			}
			else if (yNormalized <= 0.0f && xNormalized <= 0.0f)
			{
				if (xNormalized + yNormalized >= -1.0f)
				{
					// BOTTOM-LEFT BLUE
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){-1.0f, 0.0f},
						(Vec2){0.0f, -1.0f},
						(Vec2){0.0f, 0.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){-1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, -1.0f, 0.0f},
						(Vec3){0.0f, 0.0f, 1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
				else
				{
					// BOTTOM-LEFT RED
					Vec3 barycentricCoordinates = convertToBarycentricCoordinates2D(
						(Vec2){0.0f, -1.0f},
						(Vec2){-1.0f, 0.0f},
						(Vec2){-1.0f, -1.0f},
						(Vec2){xNormalized, yNormalized});

					Vec3 pointInSpace = convertFromBarycentricToEuler3D(
						(Vec3){0.0f, -1.0f, 0.0f},
						(Vec3){-1.0f, 0.0f, 0.0f},
						(Vec3){0.0f, 0.0f, -1.0f},
						barycentricCoordinates);

					assert(getGimPixelBySamplingMesh(verticesTmp, vertices, indexes, pointInSpace, &pixelColor));
				}
			}

			// Here, if we are dealing with a border pixel, we manually copy it to all its matches.
			// Theoretically, the algorithm above already takes care of it, but we need to recopy here to avoid
			// problems because of floating-point precision (xNormalized and yNormalized varies a bit and it causes
			// inconsistency in the sampling)
			if (x == 0 || x == gimSize - 1) gimData[(gimSize - y - 1) * gimSize + x] = pixelColor;
			if (y == 0 || y == gimSize - 1) gimData[y * gimSize + (gimSize - x - 1)] = pixelColor;
			if ((x == 0 && y == 0) || (x == gimSize - 1 && y == gimSize - 1) ||
				(x == 0 && y == gimSize - 1) || (x == gimSize - 1 && y == 0))
				gimData[(gimSize - y - 1) * gimSize + (gimSize - x - 1)] = pixelColor;
			
			gimData[y * gimSize + x] = pixelColor;
		}
	}

	FloatImageData fid;
	fid.data = (r32*)gimData;
	fid.channels = 3;
	fid.height = gimSize;
	fid.width = gimSize;

	GeometryImage gim;
	gim.img = fid;

	// @TEMPORARY
	gimNormalizeAndSave(&gim, "./res/result.bmp");

	gimExportToGimFile(&gim, outPath);

	free(gimData);

#if 0
	for (u32 i = 0; i < numberOfVertices; ++i) {
		vertices[i].position = (Vec4){verticesTmp[i].x, verticesTmp[i].y, verticesTmp[i].z, 1.0f};
	}
#endif

	array_release(verticesTmp);

	return 0;
}