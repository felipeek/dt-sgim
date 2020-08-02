#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include "graphics.h"
#include <dynamic_array.h>

extern "C" int objParse(const char* objPath, Vertex** vertices, u32** indexes)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, 0, &warn, &err, objPath, 0, true);

	if (!warn.empty()) {
		std::cout << warn << std::endl;
	}

		if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	*vertices = array_create(Vertex, 1);
	*indexes = array_create(u32, 1);

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		for (size_t f = 0; f < shapes[s].mesh.indices.size(); f++) {
			array_push(*indexes, &shapes[s].mesh.indices[f].vertex_index);
		}
	}

	// Loop over vertices
	for (size_t f = 0; f < attrib.vertices.size(); f += 3) {
		Vertex v;
		v.position.x = attrib.vertices[f];
		v.position.y = attrib.vertices[f + 1];
		v.position.z = attrib.vertices[f + 2];
		v.position.w = 1.0f;
		v.normal = (Vec4){1.0f, 0.0f, 0.0f, 0.0f};
		v.textureCoordinates = (Vec2) {0.0f, 0.0f};
		array_push(*vertices, &v);
	}

	// If normals were provided... fill them
	//for (size_t f = 0; f < attrib.normals.size(); f += 3) {
	//	Vertex* v = &(*vertices)[f / 3];
	//	v->normal.x = attrib.normals[f];
	//	v->normal.y = attrib.normals[f + 1];
	//	v->normal.z = attrib.normals[f + 2];
	//	v->normal.w = 0.0f;
	//}

	// If texCoords were provided... fill them
	for (size_t f = 0; f < attrib.texcoords.size(); f += 2) {
		Vertex* v = &(*vertices)[f / 2];
		v->textureCoordinates.x = attrib.texcoords[f];
		v->textureCoordinates.y = attrib.texcoords[f + 1];
	}

	return 0;
}