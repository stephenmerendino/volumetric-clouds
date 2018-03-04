#include "Engine/Renderer/SurfacePatchMeshes.hpp"

void Meshes::build_surface_patch(MeshBuilder& mb, 
								 HeightFunc heightFunc, 
								 float width, 
								 float length, 
								 unsigned int slices, 
								 float uv_density)
{
	mb.begin(PRIMITIVE_TRIANGLES, true);

	float width_step = width * (1.0f / ((float)slices + 1));
	float length_step = length * (1.0f / ((float)slices + 1));

	// Generate verts
	for(float x = 0.0f; x <= width; x += width_step){
		for(float z = 0.0f; z <= length; z += length_step){
			float y = heightFunc(x, z);

			Vector3 position(x, y, z);

			float u = (x / width) * uv_density;
			float v = (1.0f - (z / length)) * uv_density;
			mb.set_uv(u, v);

			// Calculate tangent, bitangent, and normal
			Vector3 leftPos = Vector3(x - width_step, heightFunc(x - width_step, z), z);
			Vector3 rightPos = Vector3(x + width_step, heightFunc(x + width_step, z), z);

			Vector3 upPos = Vector3(x, heightFunc(x, z + length_step), z + length_step);
			Vector3 downPos = Vector3(x, heightFunc(x, z - length_step), z - length_step);

			Vector3 prevDu = position - leftPos;
			Vector3 nextDu = rightPos - position;

			Vector3 prevDv = position - downPos;
			Vector3 nextDv = upPos - position;

			Vector3 tangent = (prevDu + nextDu) / 2.0f;
			Vector3 biTangent = (prevDv + nextDv) / 2.0f;

			tangent.Normalize();
			biTangent.Normalize();
			Vector3 normal = CrossProduct(biTangent, tangent);

			mb.set_normal(normal);
			mb.set_tangent(tangent);
			mb.set_bitangent(biTangent);

			mb.add_vertex(position);
		}
	}

	// Generate the index buffer so they are properly indexed
	unsigned int num_verts_per_row = slices + 2;
	for(unsigned int vertIndex = num_verts_per_row; vertIndex < mb.m_vertexes.size(); ++vertIndex){
		if((vertIndex + 1) % num_verts_per_row == 0){
			continue;
		}

		int bottomLeftVertex = vertIndex;
		int bottomRightVertx = bottomLeftVertex + 1;
		int topLeftVertex = bottomLeftVertex - num_verts_per_row;
		int topRightVertex = topLeftVertex + 1;

		mb.add_index(bottomLeftVertex);
		mb.add_index(topRightVertex);
		mb.add_index(topLeftVertex);
		mb.add_index(bottomLeftVertex);
		mb.add_index(bottomRightVertx);
		mb.add_index(topRightVertex);
	}

	mb.end();
}