#ifndef DT_MESH_SMOOTHING_DOMAIN_TRANSFORM_H
#define DT_MESH_SMOOTHING_DOMAIN_TRANSFORM_H
#include "filter.h"

// @TODO: this should be adjustable (UI)
#define DT_BLUR_SS 0.8f
#define DT_BLUR_SR 1.0f

typedef struct DomainTransform DomainTransform;

struct DomainTransform
{
	r32* vertical;
	r32* horizontal;
};

extern DomainTransform dtFillDomainTransforms(
	const GeometryImage* gim,
	FilterMode filterMode,
	r32 spatialFactor,
	r32 rangeFactor);

#endif