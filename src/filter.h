#ifndef GIMMESH_FILTER_H
#define GIMMESH_FILTER_H
#include "gim.h"

typedef enum FilterMode FilterMode;
typedef struct BlurNormalsInformation BlurNormalsInformation;

enum FilterMode
{
	RECURSIVE_FILTER = 0,
	CURVATURE_FILTER = 1,
};

struct BlurNormalsInformation
{
	boolean shouldBlur;
	r32 blurSS;
};

extern GeometryImage filterGeometryImageFilter(
	const GeometryImage* originalGim,
	s32 numIterations,
	r32 spatialFactor,
	r32 rangeFactor,
	FilterMode filterMode,
	const BlurNormalsInformation* blurNormalsInformation,
	boolean printTime);

#endif