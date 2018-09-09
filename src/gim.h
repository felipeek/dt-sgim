#ifndef DT_MESH_SMOOTHING_GIM_H
#define DT_MESH_SMOOTHING_GIM_H
#include "graphics.h"
#include "dynamic_array.h"

typedef struct GeometryImage GeometryImage;

struct GeometryImage
{
	FloatImageData img;
	Vertex* vertices; // Vertices have the same order as the img
	u32* indexes;
};

extern GeometryImage gimParseGeometryImageFile(const u8* path);
extern void gimGeometryImageUpdate3D(GeometryImage* gim);
extern Mesh gimGeometryImageToMesh(const GeometryImage* gim, Vec4 color);
extern void gimExportToObjFile(const GeometryImage* gim, const s8* objPath);
extern FloatImageData gimNormalizeForVisualization(const GeometryImage* gim);
extern void gimNormalizeAndSave(const GeometryImage* gim, const s8* imagePath);
extern void gimFreeGeometryImage(GeometryImage* gim);

#endif