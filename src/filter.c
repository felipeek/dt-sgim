#include "filter.h"
#include "domain_transform.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "gim.h"
#include <time.h>

#define SQRT3 1.7320508075f
#define SQRT2 1.4142135623f

// Used when recursive filter is activated, instead of domain transform
#define DEFAULT_SMOOTH_FACTOR 1.1f

// Tells if the correction step should be done when filtering
#define USE_CORRECTION

// Auxiliar function that changes each array item to be the product with all its ancestors
// Example: [2, 4, 3] -> [2, 8, 24]
static r32 preCalculateArrayProducts(r32* array, s32 size)
{
	r32 lastValue = 1.0f;
	for (s32 i = 0; i < size; ++i)
	{
		array[i] = lastValue * array[i];
		lastValue = array[i];
	}
}

// Filter pixel <x, y> using a recursive filter. Last pixel is given by lastPixel and its weight by recursiveFactor
static Vec3 filterIndividualPixelRecursive(GeometryImage* gim, s32 x, s32 y, r32 recursiveFactor, Vec3 lastPixel)
{
	Vec3 currentPixel = *(Vec3*)&gim->img.data[y * gim->img.width * gim->img.channels + x * gim->img.channels];
	Vec3 filteredPixel = gmAddVec3(gmScalarProductVec3(recursiveFactor, lastPixel), gmScalarProductVec3(1.0f - recursiveFactor, currentPixel));
	*(Vec3*)&gim->img.data[y * gim->img.width * gim->img.channels + x * gim->img.channels] = filteredPixel;
	return filteredPixel;
}

// H-Filter
static void filterHorizontalStep(
	const GeometryImage* originalGim,
	GeometryImage* filteredGim,
	const DomainTransform domainTransform,
	s32 numIterations,
	r32* rfCoefficients,
	s32 currentIteration,
	r32 spatialFactor,
	FilterMode filterMode)
{
	r32 recursiveFactor;

	// simpleRecursiveFactor is used as the recursive factor when in normal recursive filter mode
	r32 simpleRecursiveFactor = spatialFactor / (powf(DEFAULT_SMOOTH_FACTOR, currentIteration));

	// dtRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
	r32* dtRecursiveFactors = malloc(sizeof(r32) * 2.0f * (filteredGim->img.width - 1));

	for (s32 i = 1; i < filteredGim->img.height - 1; ++i)
	{
		/* ******************************************************* ********* *************************************************** */
		/* ******************************************************* FILTERING *************************************************** */
		/* ******************************************************* ********* *************************************************** */

		// dtRecursiveFactorIndex is used to perform the correction step when in distance or curvature filter mode
		s32 dtRecursiveFactorIndex = 0;

		// productOfRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
		r32 productOfRecursiveFactors = 1.0f;

		// If central line, avoid filtering process
		if (i == filteredGim->img.height / 2) continue;

		// Get the mirror Y position
		s32 mirrorYPosition = filteredGim->img.height - 1 - i;

		// Set lastPixel to be the 0 vector
		Vec3 lastPixel = (Vec3) { 0.0f, 0.0f, 0.0f };

		// Filter from (lBorder, i) to (rBorder, i)
		for (s32 j = 1; j < filteredGim->img.width; ++j)
		{
			if (filterMode == CURVATURE_FILTER)
			{
				r32 d = domainTransform.horizontal[i * originalGim->img.width + j];
				recursiveFactor = powf(rfCoefficients[currentIteration], d);
			}
			else
				recursiveFactor = simpleRecursiveFactor;

			dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
			productOfRecursiveFactors *= recursiveFactor;
			lastPixel = filterIndividualPixelRecursive(filteredGim, j, i, recursiveFactor, lastPixel);
		}

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYPosition * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;

		// Filter from (rBorder, mirrorY) to (lBorder, mirrorY)
		for (s32 j = filteredGim->img.width - 2; j >= 0; --j)
		{
			if (filterMode == CURVATURE_FILTER)
			{
				r32 d = domainTransform.horizontal[mirrorYPosition * originalGim->img.width + (j + 1)];
				recursiveFactor = powf(rfCoefficients[currentIteration], d);
			}
			else
				recursiveFactor = simpleRecursiveFactor;

			dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
			productOfRecursiveFactors *= recursiveFactor;
			lastPixel = filterIndividualPixelRecursive(filteredGim, j, mirrorYPosition, recursiveFactor, lastPixel);
		}

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels] = lastPixel;

		if (filterMode == CURVATURE_FILTER)
			assert(dtRecursiveFactorIndex == 2.0f * (filteredGim->img.width - 1));
#ifdef USE_CORRECTION
		/* ******************************************************* ********** ************************************************** */
		/* ******************************************************* CORRECTION ************************************************** */
		/* ******************************************************* ********** ************************************************** */

		Vec3 periodicBoundaryConstant = gmScalarProductVec3(1.0f / (1.0f - productOfRecursiveFactors), lastPixel);
		s32 n = 1;

		preCalculateArrayProducts(dtRecursiveFactors, 2.0f * (filteredGim->img.width - 1));

		// Correction pass from (lBorder, i) to (rBorder, i)	
		for (s32 j = 1; j < filteredGim->img.width; ++j)
		{
			Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
			Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
			*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] =
				gmAddVec3(correctionFactor, currentPixel);
		}

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYPosition * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] =
			*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];

		// Correction pass from (rBorder, mirrorY) to (lBorder, mirrorY)
		for (s32 j = filteredGim->img.width - 2; j > 0; --j)
		{
			Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[mirrorYPosition * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
			Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
			*(Vec3*)&filteredGim->img.data[mirrorYPosition * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] =
				gmAddVec3(correctionFactor, currentPixel);
		}

		// Manually sets last pixel
		*(Vec3*)&filteredGim->img.data[mirrorYPosition * filteredGim->img.width * filteredGim->img.channels] =
			periodicBoundaryConstant;

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels] = periodicBoundaryConstant;

		assert(n == 2.0f * (filteredGim->img.width - 1));
#endif
	}

	free(dtRecursiveFactors);
}

// V-Filter
static void filterVerticalStep(
	const GeometryImage* originalGim,
	GeometryImage* filteredGim,
	const DomainTransform domainTransform,
	s32 numIterations,
	r32* rfCoefficients,
	s32 currentIteration,
	r32 spatialFactor,
	FilterMode filterMode)
{
	r32 recursiveFactor;

	// simpleRecursiveFactor is used as the recursive factor when in normal recursive filter mode
	r32 simpleRecursiveFactor = spatialFactor / (powf(DEFAULT_SMOOTH_FACTOR, currentIteration));

	// dtRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
	r32* dtRecursiveFactors = malloc(sizeof(r32) * 2.0f * (filteredGim->img.height - 1));

	for (s32 j = 1; j < filteredGim->img.width - 1; ++j)
	{
		/* ******************************************************* ********* *************************************************** */
		/* ******************************************************* FILTERING *************************************************** */
		/* ******************************************************* ********* *************************************************** */

		// dtRecursiveFactorIndex is used to perform the correction step when in distance or curvature filter mode
		s32 dtRecursiveFactorIndex = 0;

		// productOfRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
		r32 productOfRecursiveFactors = 1.0f;

		// If central line, avoid filtering process
		if (j == filteredGim->img.width / 2) continue;

		// Get the mirror X position
		s32 mirrorXPosition = filteredGim->img.width - 1 - j;

		// Set the last pixel to be the 0 vector
		Vec3 lastPixel = (Vec3) { 0.0f, 0.0f, 0.0f };

		// Filter from (j, tBorder) to (j, bBorder)
		for (s32 i = 1; i < filteredGim->img.height; ++i)
		{
			if (filterMode == CURVATURE_FILTER)
			{
				r32 d = domainTransform.vertical[i * originalGim->img.width + j];
				recursiveFactor = powf(rfCoefficients[currentIteration], d);
			}
			else
				recursiveFactor = simpleRecursiveFactor;

			dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
			productOfRecursiveFactors *= recursiveFactor;
			lastPixel = filterIndividualPixelRecursive(filteredGim, j, i, recursiveFactor, lastPixel);
		}
		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + mirrorXPosition * filteredGim->img.channels] = lastPixel;

		// Filter from (mirrorX, bBorder) to (mirrorX, tBorder)
		for (s32 i = filteredGim->img.height - 2; i >= 0; --i)
		{
			if (filterMode == CURVATURE_FILTER)
			{
				r32 d = domainTransform.vertical[(i + 1) * originalGim->img.width + mirrorXPosition];
				recursiveFactor = powf(rfCoefficients[currentIteration], d);
			}
			else
				recursiveFactor = simpleRecursiveFactor;

			dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
			productOfRecursiveFactors *= recursiveFactor;
			lastPixel = filterIndividualPixelRecursive(filteredGim, mirrorXPosition, i, recursiveFactor, lastPixel);
		}

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels] = lastPixel;

		if (filterMode == CURVATURE_FILTER)
			assert(dtRecursiveFactorIndex == 2.0f * (filteredGim->img.height - 1));

#ifdef USE_CORRECTION
		/* ******************************************************* ********** ************************************************** */
		/* ******************************************************* CORRECTION ************************************************** */
		/* ******************************************************* ********** ************************************************** */

		Vec3 periodicBoundaryConstant = gmScalarProductVec3(1.0f / (1.0f - productOfRecursiveFactors), lastPixel);
		s32 n = 1;

		preCalculateArrayProducts(dtRecursiveFactors, 2.0f * (filteredGim->img.height - 1));

		// Correction pass from (j, tBorder) to (j, bBorder)
		for (s32 i = 1; i < filteredGim->img.height; ++i)
		{
			Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
			Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
			*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] =
				gmAddVec3(correctionFactor, currentPixel);
		}

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + mirrorXPosition * filteredGim->img.channels] =
			*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];

		// Correction pass from (mirrorX, bBorder) to (mirrorX, tBorder)
		for (s32 i = filteredGim->img.height - 2; i > 0; --i)
		{
			Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + mirrorXPosition * filteredGim->img.channels];
			Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
			*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + mirrorXPosition * filteredGim->img.channels] =
				gmAddVec3(correctionFactor, currentPixel);
		}

		// Manually sets last pixel
		*(Vec3*)&filteredGim->img.data[mirrorXPosition * filteredGim->img.channels] = periodicBoundaryConstant;

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels] = periodicBoundaryConstant;

		assert(n == 2.0f * (filteredGim->img.height - 1));
#endif
	}

	free(dtRecursiveFactors);
}

// C-Filter
static void filterCStep(
	const GeometryImage* originalGim,
	GeometryImage* filteredGim,
	const DomainTransform domainTransform,
	s32 numIterations,
	r32* rfCoefficients,
	s32 currentIteration,
	r32 spatialFactor,
	FilterMode filterMode)
{
	// recursiveFactor is used as the recursive factor when in normal recursive filter mode
	r32 recursiveFactor;

	// simpleRecursiveFactor is used as the recursive factor when in normal recursive filter mode
	r32 simpleRecursiveFactor = spatialFactor / (powf(DEFAULT_SMOOTH_FACTOR, currentIteration));

	// dtRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
	r32* dtRecursiveFactors = malloc(sizeof(r32) * (r32)((filteredGim->img.height - 1) + (filteredGim->img.width - 1)));
	
	// dtRecursiveFactorIndex is used to perform the correction step when in distance or curvature filter mode
	s32 dtRecursiveFactorIndex = 0;

	// productOfRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
	r32 productOfRecursiveFactors = 1.0f;

	s32 halfWidth = filteredGim->img.width / 2;

	/* ******************************************************* ********* *************************************************** */
	/* ******************************************************* FILTERING *************************************************** */
	/* ******************************************************* ********* *************************************************** */

	// Set the last pixel to be the 0 vector
	Vec3 lastPixel = (Vec3) { 0.0f, 0.0f, 0.0f };

	// Filter from (half, tBorder) to (half, bBorder)
	for (s32 i = 0; i < filteredGim->img.height; ++i)
	{
		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = (i == 0) ? domainTransform.horizontal[i * originalGim->img.width + halfWidth] : domainTransform.vertical[i * originalGim->img.width + halfWidth];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, halfWidth, i, recursiveFactor, lastPixel);
	}

	// Filter from (half, bBorder) to (rBorder, bBorder)
	for (s32 j = halfWidth + 1; j < filteredGim->img.width; ++j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.horizontal[(filteredGim->img.height - 1) * originalGim->img.width + j];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, j, filteredGim->img.height - 1, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + mirrorXBorder * filteredGim->img.channels] = lastPixel;
	}


	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[0] = lastPixel;
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;

	// Filter from (rBorder, tBorder) to (half, tBorder)
	for (s32 j = filteredGim->img.width - 2; j > halfWidth; --j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.horizontal[0 * originalGim->img.width + (j + 1)];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, j, 0, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorXBorder * filteredGim->img.channels] = lastPixel;
	}

	if (filterMode == CURVATURE_FILTER)	
		assert(dtRecursiveFactorIndex == (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

#ifdef USE_CORRECTION
	/* ******************************************************* ********** ************************************************** */
	/* ******************************************************* CORRECTION ************************************************** */
	/* ******************************************************* ********** ************************************************** */

	Vec3 periodicBoundaryConstant = gmScalarProductVec3(1.0f / (1.0f - productOfRecursiveFactors), lastPixel);
	s32 n = 1;

	preCalculateArrayProducts(dtRecursiveFactors, (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

	// Correction pass from (half, tBorder) to (half, bBorder)
	for (s32 i = 0; i < filteredGim->img.height; ++i)
	{
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + halfWidth * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + halfWidth * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);
	}

	// Correction pass from (half, bBorder) to (rBorder, bBorder)
	for (s32 j = halfWidth + 1; j < filteredGim->img.width; ++j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + mirrorXBorder * filteredGim->img.channels] =
			*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[0] = *(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1) * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];

	// Correction pass from (rBorder, tBorder) to (half, tBorder)
	for (s32 j = filteredGim->img.width - 2; j > halfWidth + 1; --j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels] = gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorXBorder * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels] = gmAddVec3(correctionFactor, currentPixel);
	}

	// Manually sets last pixel
	*(Vec3*)&filteredGim->img.data[(halfWidth + 1) * filteredGim->img.channels] = periodicBoundaryConstant;

	// Copy border pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1 - (halfWidth + 1)) * filteredGim->img.channels] = periodicBoundaryConstant;

	assert(n == (r32)((filteredGim->img.height - 1) + (filteredGim->img.width - 1)));
#endif
	/* ******************************************************* ********* *************************************************** */
	/* ******************************************************* FILTERING *************************************************** */
	/* ******************************************************* ********* *************************************************** */

	// Reset correction variables
	dtRecursiveFactorIndex = 0;
	productOfRecursiveFactors = 1.0f;

	// Set the last pixel to be the 0 vector
	lastPixel = (Vec3) { 0.0f, 0.0f, 0.0f };

	// Filter from (half, bBorder) to (half, tBorder)
	for (s32 i = filteredGim->img.height - 1; i >= 0; --i)
	{
		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = (i == originalGim->img.height - 1) ? domainTransform.horizontal[i * originalGim->img.width + (halfWidth + 1)] : domainTransform.vertical[(i + 1) * originalGim->img.width + halfWidth];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, halfWidth, i, recursiveFactor, lastPixel);
	}

	// Filter from (half, tBorder) to (rBorder, tBorder)
	for (s32 j = halfWidth + 1; j < filteredGim->img.width; ++j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.horizontal[0 * originalGim->img.width + j];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, j, 0, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorXBorder * filteredGim->img.channels] = lastPixel;
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels] = lastPixel;
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;

	// Filter from (rBorder, bBorder) to (half, bBorder)
	for (s32 j = filteredGim->img.width - 2; j > halfWidth; --j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.horizontal[(filteredGim->img.height - 1) * originalGim->img.width + (j + 1)];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, j, filteredGim->img.height - 1, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + mirrorXBorder * filteredGim->img.channels] = lastPixel;
	}

	if (filterMode == CURVATURE_FILTER)
		assert(dtRecursiveFactorIndex == (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

#ifdef USE_CORRECTION
	/* ******************************************************* ********** ************************************************** */
	/* ******************************************************* CORRECTION ************************************************** */
	/* ******************************************************* ********** ************************************************** */

	periodicBoundaryConstant = gmScalarProductVec3(1.0f / (1.0f - productOfRecursiveFactors), lastPixel);
	n = 1;

	preCalculateArrayProducts(dtRecursiveFactors, (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

	// Correction pass from (half, bBorder) to (half, tBorder)
	for (s32 i = filteredGim->img.height - 1; i >= 0; --i)
	{
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + halfWidth * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + halfWidth * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);
	}

	// Correction pass from (half, tBorder) to (rBorder, tBorder)
	for (s32 j = halfWidth + 1; j < filteredGim->img.width; ++j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels] = gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorXBorder * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[j * filteredGim->img.channels];
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1) * filteredGim->img.channels];
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1) * filteredGim->img.channels];

	// Correction pass from (rBorder, bBorder) to (half, bBorder)
	for (s32 j = filteredGim->img.width - 2; j > halfWidth + 1; --j)
	{
		s32 mirrorXBorder = filteredGim->img.width - 1 - j;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + mirrorXBorder * filteredGim->img.channels] =
			*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
	}

	// Manually sets last pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (halfWidth + 1) * filteredGim->img.channels] = periodicBoundaryConstant;

	// Copy border pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1 - (halfWidth + 1)) * filteredGim->img.channels] = periodicBoundaryConstant;

	assert(n == (r32)((filteredGim->img.height - 1) + (filteredGim->img.width - 1)));
#endif

	free(dtRecursiveFactors);
}

// Pi-Filter
static void filterPiStep(
	const GeometryImage* originalGim,
	GeometryImage* filteredGim,
	const DomainTransform domainTransform,
	s32 numIterations,
	r32* rfCoefficients,
	s32 currentIteration,
	r32 spatialFactor,
	FilterMode filterMode)
{
	// recursiveFactor is used as the recursive factor when in normal recursive filter mode
	r32 recursiveFactor;

	// simpleRecursiveFactor is used as the recursive factor when in normal recursive filter mode
	r32 simpleRecursiveFactor = spatialFactor / (powf(DEFAULT_SMOOTH_FACTOR, currentIteration));

	// dtRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
	r32* dtRecursiveFactors = malloc(sizeof(r32) * (r32)((filteredGim->img.height - 1) + (filteredGim->img.width - 1)));

	// dtRecursiveFactorIndex is used to perform the correction step when in distance or curvature filter mode
	s32 dtRecursiveFactorIndex = 0;

	// productOfRecursiveFactors is used to perform the correction step when in distance or curvature filter mode
	r32 productOfRecursiveFactors = 1.0f;

	s32 halfHeight = filteredGim->img.height / 2;

	/* ******************************************************* ********* *************************************************** */
	/* ******************************************************* FILTERING *************************************************** */
	/* ******************************************************* ********* *************************************************** */

	// Set the last pixel to be the 0 vector
	Vec3 lastPixel = (Vec3) { 0.0f, 0.0f, 0.0f };

	// Filter from (lBorder, half) to (rBorder, half)
	for (s32 j = 0; j < filteredGim->img.width; ++j)
	{
		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = (j == 0) ? domainTransform.vertical[halfHeight * originalGim->img.width + j] : domainTransform.horizontal[halfHeight * originalGim->img.width + j];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, j, halfHeight, recursiveFactor, lastPixel);
	}

	// Filter from (rBorder, half) to (rBorder, bBorder)
	for (s32 i = halfHeight + 1; i < filteredGim->img.height; ++i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.vertical[i * originalGim->img.width + (filteredGim->img.width - 1)];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, filteredGim->img.width - 1, i, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[0] = lastPixel;
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels] = lastPixel;

	// Filter from (lBorder, bBorder) to (lBorder, half)
	for (s32 i = filteredGim->img.height - 2; i > halfHeight; --i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.vertical[(i + 1) * originalGim->img.width + 0];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, 0, i, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels] = lastPixel;
	}

	if (filterMode == CURVATURE_FILTER)
		assert(dtRecursiveFactorIndex == (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

#ifdef USE_CORRECTION
	/* ******************************************************* ********** ************************************************** */
	/* ******************************************************* CORRECTION ************************************************** */
	/* ******************************************************* ********** ************************************************** */

	Vec3 periodicBoundaryConstant = gmScalarProductVec3(1.0f / (1.0f - productOfRecursiveFactors), lastPixel);
	s32 n = 1;

	preCalculateArrayProducts(dtRecursiveFactors, (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

	// Correction pass from (lBorder, half) to (rBorder, half)
	for (s32 j = 0; j < filteredGim->img.width; ++j)
	{
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[halfHeight * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[halfHeight * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);
	}

	// Correction pass from (rBorder, half) to (rBorder, bBorder)
	for (s32 i = halfHeight + 1; i < filteredGim->img.height; ++i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] =
			*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[0] = *(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels] =
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];

	// Correction pass from (lBorder, bBorder) to (lBorder, half)
	for (s32 i = filteredGim->img.height - 2; i > halfHeight + 1; --i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels];
	}

	// Manually sets last pixel
	*(Vec3*)&filteredGim->img.data[(halfHeight + 1) * filteredGim->img.width * filteredGim->img.channels] = periodicBoundaryConstant;

	// Copy border pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1 - (halfHeight + 1)) * filteredGim->img.width * filteredGim->img.channels] = lastPixel;

	assert(n == (r32)((filteredGim->img.height - 1) + (filteredGim->img.width - 1)));
#endif
	/* ******************************************************* ********* *************************************************** */
	/* ******************************************************* FILTERING *************************************************** */
	/* ******************************************************* ********* *************************************************** */

	// Reset correction variables
	dtRecursiveFactorIndex = 0;
	productOfRecursiveFactors = 1.0f;

	// Set the last pixel to be the 0 vector
	lastPixel = (Vec3) { 0.0f, 0.0f, 0.0f };

	// Filter from (rBorder, half) to (lBorder, half)
	for (s32 j = filteredGim->img.width - 1; j >= 0; --j)
	{
		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = (j == originalGim->img.width - 1) ? domainTransform.vertical[(halfHeight + 1) * originalGim->img.width + j] :
				domainTransform.horizontal[halfHeight * originalGim->img.width + (j + 1)];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, j, halfHeight, recursiveFactor, lastPixel);
	}

	// Filter from (lBorder, half) to (lBorder, bBorder)
	for (s32 i = halfHeight + 1; i < filteredGim->img.height; ++i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.vertical[i * originalGim->img.width + 0];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, 0, i, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels] = lastPixel;
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;

	// Filter from (rBorder, bBorder) to (rBorder, half)
	for (s32 i = filteredGim->img.height - 2; i > halfHeight; --i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;

		if (filterMode == CURVATURE_FILTER)
		{
			r32 d = domainTransform.vertical[(i + 1) * originalGim->img.width + (filteredGim->img.width - 1)];
			recursiveFactor = powf(rfCoefficients[currentIteration], d);
		}
		else
			recursiveFactor = simpleRecursiveFactor;

		dtRecursiveFactors[dtRecursiveFactorIndex++] = recursiveFactor;
		productOfRecursiveFactors *= recursiveFactor;
		lastPixel = filterIndividualPixelRecursive(filteredGim, filteredGim->img.width - 1, i, recursiveFactor, lastPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;
	}

	if (filterMode == CURVATURE_FILTER)
		assert(dtRecursiveFactorIndex == (filteredGim->img.height - 1) + (filteredGim->img.width - 1));

#ifdef USE_CORRECTION
	/* ******************************************************* ********** ************************************************** */
	/* ******************************************************* CORRECTION ************************************************** */
	/* ******************************************************* ********** ************************************************** */

	periodicBoundaryConstant = gmScalarProductVec3(1.0f / (1.0f - productOfRecursiveFactors), lastPixel);
	n = 1;

	preCalculateArrayProducts(dtRecursiveFactors, 2.0f * (filteredGim->img.height - 1));

	// Correction pass from (rBorder, half) to (lBorder, half)
	for (s32 j = filteredGim->img.width - 1; j >= 0; --j)
	{
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[halfHeight * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[halfHeight * filteredGim->img.width * filteredGim->img.channels + j * filteredGim->img.channels] = 
			gmAddVec3(correctionFactor, currentPixel);
	}

	// Correction pass from (lBorder, half) to (lBorder, bBorder)
	for (s32 i = halfHeight + 1; i < filteredGim->img.height; ++i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels];
	}

	// Copy Corner Pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.width - 1) * filteredGim->img.channels] = *(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels];
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] =
		*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1) * filteredGim->img.width * filteredGim->img.channels];

	// Correction pass from (rBorder, bBorder) to (rBorder, half)
	for (s32 i = filteredGim->img.height - 2; i > halfHeight + 1; --i)
	{
		s32 mirrorYBorder = filteredGim->img.height - 1 - i;
		Vec3 currentPixel = *(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];
		Vec3 correctionFactor = gmScalarProductVec3(dtRecursiveFactors[n++ - 1], periodicBoundaryConstant);
		*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] =
			gmAddVec3(correctionFactor, currentPixel);

		// Copy border pixel
		*(Vec3*)&filteredGim->img.data[mirrorYBorder * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] =
			*(Vec3*)&filteredGim->img.data[i * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels];
	}

	// Manually sets last pixel
	*(Vec3*)&filteredGim->img.data[(halfHeight + 1) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = periodicBoundaryConstant;

	// Copy border pixel
	*(Vec3*)&filteredGim->img.data[(filteredGim->img.height - 1 - (halfHeight + 1)) * filteredGim->img.width * filteredGim->img.channels + (filteredGim->img.width - 1) * filteredGim->img.channels] = lastPixel;

	assert(n == (r32)((filteredGim->img.height - 1) + (filteredGim->img.width - 1)));
#endif

	free(dtRecursiveFactors);
}

// Filters a generic geometry image
// originalGim: The geometry image to be filtered
// numIterations: Number of iterations used in the filtering process
// spatialFactor: Filter's parameter - proportional to filter's intensity
// rangeFactor: Filter's parameter - it depends on the chosen filterMode
// filterMode: Filter's type:
//		- RECURSIVE_FILTER: Mesh will be filtered ignoring rangeFactor
//		- DISTANCE_FILTER: The distance from vertex to vertex will limit the filter
//		- CURVATURE_FILTER: The mesh's curvature will limit the filter
extern GeometryImage filterGeometryImageFilter(
	const GeometryImage* originalGim,
	s32 numIterations,
	r32 spatialFactor,
	r32 rangeFactor,
	FilterMode filterMode,
	const BlurNormalsInformation* blurNormalsInformation,
	boolean printTime)
{
	GeometryImage filteredGim = {0};
	filteredGim.img = graphicsFloatImageCopy(&originalGim->img);

	printf("Filtering process started...\n");
	printf("Allocating memory...\n");

	// Memory Allocation
	r32* rfCoefficients = (r32*)malloc(sizeof(r32) * numIterations);

	clock_t t = clock();

	// Calculate domain transforms
	DomainTransform domainTransform;
	if (filterMode == CURVATURE_FILTER)
	{
		printf("Calculating domain transforms...\n");
		domainTransform = dtGenerateDomainTransforms(originalGim, spatialFactor, rangeFactor, blurNormalsInformation);
	}

	/* ************************ */
	printf("Calculating RF feedback coefficients...\n");

	// Pre-calculate RF feedback coefficients
	// @TODO: This must be updated
	for (s32 i = 0; i < numIterations; ++i)
	{
		// calculating RF feedback coefficient 'a' from desired variance
		// 'a' will change each iteration while the domain transform will remain constant
		r32 current_standard_deviation = spatialFactor * SQRT3 * (powf(2.0f, (r32)(numIterations - (i + 1))) / sqrtf(powf(4.0f, (r32)numIterations) - 1));
		r32 a = expf(-SQRT2 / current_standard_deviation);

		rfCoefficients[i] = a;
	}

	// Filter
	for (s32 i = 0; i < numIterations; i++)
	{
		printf("Filtering... [%d/%d]\n", i+1, numIterations);
		filterHorizontalStep(originalGim, &filteredGim, domainTransform, numIterations, rfCoefficients, i, spatialFactor, filterMode);
		filterCStep(originalGim, &filteredGim, domainTransform, numIterations, rfCoefficients, i, spatialFactor, filterMode);
		filterVerticalStep(originalGim, &filteredGim, domainTransform, numIterations, rfCoefficients, i, spatialFactor, filterMode);
		filterPiStep(originalGim, &filteredGim, domainTransform, numIterations, rfCoefficients, i, spatialFactor, filterMode);
	}

	t = clock() - t;

	free(rfCoefficients);
	if (filterMode == CURVATURE_FILTER)
		dtDeleteDomainTransforms(domainTransform);

	if (printTime) printf("Time elapsed filtering: %f\n", ((double)t)/CLOCKS_PER_SEC);

	return filteredGim;
}