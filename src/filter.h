#ifndef DT_MESH_SMOOTHING_FILTER_H
#define DT_MESH_SMOOTHING_FILTER_H
#include "gim.h"

typedef enum FilterMode FilterMode;
typedef struct BlurNormalsInformation BlurNormalsInformation;

enum FilterMode
{
	RECURSIVE_FILTER = 0,
	CURVATURE_FILTER = 1,
	NOISE_GENERATOR = 2,
};

struct BlurNormalsInformation
{
	boolean shouldBlur;
	r32 blurSS;
	r32 blurSR;
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