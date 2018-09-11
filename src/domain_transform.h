#ifndef DT_MESH_SMOOTHING_DOMAIN_TRANSFORM_H
#define DT_MESH_SMOOTHING_DOMAIN_TRANSFORM_H
#include "filter.h"

typedef struct DomainTransform DomainTransform;

struct DomainTransform
{
	r32* vertical;
	r32* horizontal;
};

extern DomainTransform dtGenerateDomainTransforms(
	const GeometryImage* gim,
	FilterMode filterMode,
	r32 spatialFactor,
	r32 rangeFactor,
	const BlurInformation* blurInformation);

extern FloatImageData dtGenerateCurvatureImage(
	const GeometryImage* gim,
	r32 spatialFactor,
	r32 rangeFactor,
	const BlurInformation* blurInformation);

extern void dtDeleteDomainTransforms(DomainTransform dt);

#endif