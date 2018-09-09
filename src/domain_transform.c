#include "domain_transform.h"
#include "gim.h"

// Blur domain transforms
static void blurDomainTransform(const GeometryImage* gim, DomainTransform domainTransform, r32 ss, r32 sr)
{
	// filter properties currently being used to perform the blur
	const FilterMode blurMode = RECURSIVE_FILTER;
	const s32 blurIterations = 10;

	// To properly use the filter, we need to wrap the domain transforms using a "fake geometry image"
	GeometryImage dtGim = {0};
	dtGim.img.channels = 3;
	dtGim.img.width = gim->img.width;
	dtGim.img.height = gim->img.height;
	dtGim.img.data = malloc(sizeof(r32) * gim->img.width * gim->img.height * 3);

	// Fill the fake geometry image with the horizontal domain transforms
	// We will fill all three channels with the domain transform value
	for (s32 i = 0; i < dtGim.img.height; ++i)
		for (s32 j = 0; j < dtGim.img.width; ++j)
		{
			r32 v = domainTransform.horizontal[i * dtGim.img.width + j];
			*(Vec3*)&dtGim.img.data[i * dtGim.img.width * dtGim.img.channels + j * dtGim.img.channels] = (Vec3) { v, v, v };
		}

	// Blur the fake geometry image 
	GeometryImage horizontalResultGim = filterGeometryImageFilter(&dtGim, blurIterations, ss, sr, blurMode);

	// Store results
	for (s32 i = 0; i < dtGim.img.height; ++i)
		for (s32 j = 0; j < dtGim.img.width; ++j)
			domainTransform.horizontal[i * dtGim.img.width + j] =
				horizontalResultGim.img.data[i * horizontalResultGim.img.width * horizontalResultGim.img.channels + j * horizontalResultGim.img.channels];

	// Fill the fake geometry image with the vertical domain transforms
	// We will fill all three channels with the domain transform value
	for (s32 i = 0; i < dtGim.img.height; ++i)
		for (s32 j = 0; j < dtGim.img.width; ++j)
		{
			r32 v = domainTransform.vertical[i * dtGim.img.width + j];
			*(Vec3*)&dtGim.img.data[i * dtGim.img.width * dtGim.img.channels + j * dtGim.img.channels] = (Vec3) { v, v, v };
		}

	// Blur the fake geometry image 
	GeometryImage verticalResultGim = filterGeometryImageFilter(&dtGim, blurIterations, ss, sr, blurMode);

	// Store results
	for (s32 i = 0; i < dtGim.img.height; ++i)
		for (s32 j = 0; j < dtGim.img.width; ++j)
			domainTransform.vertical[i * dtGim.img.width + j] =
				verticalResultGim.img.data[i * verticalResultGim.img.width * verticalResultGim.img.channels + j * verticalResultGim.img.channels];

	free(dtGim.img.data);
}

// Calculates and stores the domain transform of pixel 'currentPixel'
static r32 fillDomainTransform(
	const GeometryImage* gim,
	r32* dt,
    DiscreteVec2 currentPixel,
    DiscreteVec2 lastPixel,
    DiscreteVec2 penultPixel,
	r32 spatialFactor,
	r32 rangeFactor,
	FilterMode filterMode)
{
    Vec3 currentPixelVal, lastPixelVal, penultPixelVal, nextPixelVal;
    Vec4 currentNormal, lastNormal;
	Vec3 lastTangent, currentTangent, nextTangent;
	Vec3 lastTangentAverage, nextTangentAverage;
    r32 curvatureValue, distanceValue, d;

    if (filterMode == DISTANCE_FILTER)
    {
        currentPixelVal = *(Vec3*)&gim->img.data[currentPixel.y * gim->img.width * gim->img.channels + currentPixel.x * gim->img.channels];
        lastPixelVal = *(Vec3*)&gim->img.data[lastPixel.y * gim->img.width * gim->img.channels + lastPixel.x * gim->img.channels];

        // Get the distanceValue - this is proportional to the distance between currentPixel and lastPixel
        d = gmAbsolute(currentPixelVal.r - lastPixelVal.r) + 
            gmAbsolute(currentPixelVal.g - lastPixelVal.g) +
            gmAbsolute(currentPixelVal.b - lastPixelVal.b);
    }
    else if (filterMode == CURVATURE_FILTER)
    {
        currentPixelVal = *(Vec3*)&gim->img.data[currentPixel.y * gim->img.width * gim->img.channels + currentPixel.x * gim->img.channels];
        lastPixelVal = *(Vec3*)&gim->img.data[lastPixel.y * gim->img.width * gim->img.channels + lastPixel.x * gim->img.channels];

        // Get normals
		currentNormal = gim->normals[currentPixel.y * gim->img.width + currentPixel.x];
        lastNormal = gim->normals[lastPixel.y * gim->img.width + lastPixel.x];

        // Normalize normals
        currentNormal = gmNormalizeVec4(currentNormal);
        lastNormal = gmNormalizeVec4(lastNormal);

        // Get the curvatureValue - this is the length of the difference between normals
        d = gmLengthVec4(gmSubtractVec4(currentNormal, lastNormal));
    }

    dt[currentPixel.y * gim->img.width + currentPixel.x] = d;

	// Copy border if needed
	// If corner pixel
	if ((currentPixel.x == 0 && currentPixel.y == 0) ||
		(currentPixel.x == 0 && currentPixel.y == gim->img.height - 1) ||
		(currentPixel.x == gim->img.width - 1 && currentPixel.y == 0) ||
		(currentPixel.x == gim->img.width - 1 && currentPixel.y == gim->img.height - 1))
	{
		dt[0 * gim->img.width + 0] = d;
		dt[0 * gim->img.width + (gim->img.width - 1)] = d;
		dt[(gim->img.height - 1) * gim->img.width + 0] = d;
		dt[(gim->img.height - 1) * gim->img.width + (gim->img.width - 1)] = d;
	}
	// If left border pixel
	else if (currentPixel.x == 0)
	{
		s32 mirrorYPosition = gim->img.height - currentPixel.y - 1;
		dt[mirrorYPosition * gim->img.width + 0] = d;
	}
	// If right border pixel
	else if (currentPixel.x == gim->img.width - 1)
	{
		s32 mirrorYPosition = gim->img.height - currentPixel.y - 1;
		dt[mirrorYPosition * gim->img.width + (gim->img.width - 1)] = d;
	}
	// If top border pixel
	else if (currentPixel.y == 0)
	{
		s32 mirrorXPosition = gim->img.width - currentPixel.x - 1;
		dt[0 * gim->img.width + mirrorXPosition] = d;
	}
	// If bottom border pixel
	else if (currentPixel.y == gim->img.height - 1)
	{
		s32 mirrorXPosition = gim->img.width - currentPixel.x - 1;
		dt[(gim->img.height - 1) * gim->img.width + mirrorXPosition] = d;
	}

    return d;
}

// This function will calculate both horizontal and vertical domain transforms of geometry image 'gim'
// filterMode tells which method should be used to calculate the transforms
extern DomainTransform dtFillDomainTransforms(
	const GeometryImage* gim,
	FilterMode filterMode,
	r32 spatialFactor,
	r32 rangeFactor)
{
	DomainTransform domainTransform;
    DiscreteVec2 nextPixel, currentPixel, lastPixel, penultPixel;
    r32 lastValue;

	domainTransform.vertical = malloc(sizeof(r32) * gim->img.width * gim->img.height);
	domainTransform.horizontal = malloc(sizeof(r32) * gim->img.width * gim->img.height);

    // HORIZONTAL STEP
	for (s32 i = 1; i < gim->img.height - 1; ++i)
	{
		// If central line, avoid filtering process
		if (i == gim->img.height / 2) continue;

		// Get the mirror Y position
		s32 mirrorYPosition = gim->img.height - 1 - i;

        // Fill initial conditions
        currentPixel = (DiscreteVec2) {0, i};
        lastPixel = (DiscreteVec2) {1, mirrorYPosition};
        penultPixel = (DiscreteVec2) {2, mirrorYPosition};
        
		// Filter from (lBorder, i) to (rBorder, i)
		// Note: border pixels will have their value set "two times" (one for each match, but fillDomainTransform copy the border)
		// However, this is not a problem, since their normals must be exactly the same, because they are the same vertex
		for (s32 j = 0; j < gim->img.width; ++j)
		{
            currentPixel = (DiscreteVec2) {j, i};
            lastValue = fillDomainTransform(gim, domainTransform.horizontal, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
            penultPixel = lastPixel;
            lastPixel = currentPixel;
		}
	}

    // VERTICAL STEP
	for (s32 j = 1; j < gim->img.width - 1; ++j)
	{
		// If central line, avoid filtering process
		if (j == gim->img.width / 2) continue;

		// Get the mirror X position
		s32 mirrorXPosition = gim->img.width - 1 - j;

        // Fill initial conditions
        currentPixel = (DiscreteVec2) {j, 0};
        lastPixel = (DiscreteVec2) {mirrorXPosition, 1};
        penultPixel = (DiscreteVec2) {mirrorXPosition, 2};

		// Filter from (j, tBorder) to (j, bBorder)
		for (s32 i = 0; i < gim->img.height; ++i)
		{
            currentPixel = (DiscreteVec2) {j, i};
            lastValue = fillDomainTransform(gim, domainTransform.vertical, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
            penultPixel = lastPixel;
            lastPixel = currentPixel;
		}
    }

    // C STEP
	s32 halfWidth = gim->img.width / 2;

    // Fill initial conditions
    currentPixel = (DiscreteVec2) {halfWidth, 0};
    lastPixel = (DiscreteVec2) {halfWidth + 1, 0};
    penultPixel = (DiscreteVec2) {halfWidth + 2, 0};

	// Filter from (half, tBorder) to (half, bBorder)
	for (s32 i = 0; i < gim->img.height; ++i)
	{
        currentPixel = (DiscreteVec2) {halfWidth, i};
        lastValue = fillDomainTransform(gim, domainTransform.vertical, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
        penultPixel = lastPixel;
        lastPixel = currentPixel;
	}

	// Filter from (half, bBorder) to (rBorder, bBorder)
	for (s32 j = halfWidth + 1; j < gim->img.width; ++j)
	{
		s32 mirrorXBorder = gim->img.width - 1 - j;

        currentPixel = (DiscreteVec2) {j, gim->img.height - 1};
        lastValue = fillDomainTransform(gim, domainTransform.horizontal, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
        penultPixel = lastPixel;
        lastPixel = currentPixel;
	}

	// Filter from (rBorder, tBorder) to (half, tBorder)
	for (s32 j = gim->img.width - 2; j > halfWidth; --j)
	{
		s32 mirrorXBorder = gim->img.width - 1 - j;

        currentPixel = (DiscreteVec2) {j, 0};
        lastValue = fillDomainTransform(gim, domainTransform.horizontal, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
        penultPixel = lastPixel;
        lastPixel = currentPixel;
	}

    // PI STEP
	s32 halfHeight = gim->img.height / 2;

    // Fill initial conditions
    currentPixel = (DiscreteVec2) {0, halfHeight};
    lastPixel = (DiscreteVec2) {0, halfHeight + 1};
    penultPixel = (DiscreteVec2) {0, halfHeight + 2};

	// Filter from (lBorder, half) to (rBorder, half)
	for (s32 j = 0; j < gim->img.width; ++j)
	{
        currentPixel = (DiscreteVec2) {j, halfHeight};
        lastValue = fillDomainTransform(gim, domainTransform.horizontal, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
        penultPixel = lastPixel;
        lastPixel = currentPixel;
	}

	// Filter from (rBorder, half) to (rBorder, bBorder)
	for (s32 i = halfHeight + 1; i < gim->img.height; ++i)
	{
		s32 mirrorYBorder = gim->img.height - 1 - i;

        currentPixel = (DiscreteVec2) {gim->img.width - 1, i};
        lastValue = fillDomainTransform(gim, domainTransform.vertical, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
        penultPixel = lastPixel;
        lastPixel = currentPixel;
	}

	// Filter from (lBorder, bBorder) to (lBorder, half)
	for (s32 i = gim->img.height - 2; i > halfHeight; --i)
	{
		s32 mirrorYBorder = gim->img.height - 1 - i;

        currentPixel = (DiscreteVec2) {0, i};
        lastValue = fillDomainTransform(gim, domainTransform.vertical, currentPixel, lastPixel, penultPixel, spatialFactor, rangeFactor, filterMode);
        penultPixel = lastPixel;
        lastPixel = currentPixel;
	}

    // BLUR STEP
    if (filterMode == CURVATURE_FILTER)
        blurDomainTransform(gim, domainTransform, DT_BLUR_SS, DT_BLUR_SR);

    // FINAL STEP
	for (s32 i = 0; i < gim->img.height; ++i)
		for (s32 j = 0; j < gim->img.width; ++j)
		{
			domainTransform.horizontal[i * gim->img.width + j] = 1.0f + (spatialFactor / rangeFactor) * domainTransform.horizontal[i * gim->img.width + j];
			domainTransform.vertical[i * gim->img.width + j] = 1.0f + (spatialFactor / rangeFactor) * domainTransform.vertical[i * gim->img.width + j];
        }

	return domainTransform;
}