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
	r32 spatialFactor,
	r32 rangeFactor,
	const BlurNormalsInformation* blurInformation);

extern FloatImageData dtGenerateDomainTransformsImage(
	const GeometryImage* gim,
	r32 spatialFactor,
	r32 rangeFactor,
	const BlurNormalsInformation* blurInformation);

extern FloatImageData dtGenerateNormalImage(
	const GeometryImage* gim,
	boolean blurNormals,
	r32 blurSS);

extern void dtDeleteDomainTransforms(DomainTransform dt);

#endif