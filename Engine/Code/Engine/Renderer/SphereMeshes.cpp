#include "Engine/Renderer/SphereMeshes.hpp"

void Meshes::build_uv_sphere(MeshBuilder& mb, const Vector3& center, float radius, const Rgba& color, unsigned int slices)
{
	mb.begin(PRIMITIVE_TRIANGLES, true);
	mb.set_color(color);

	unsigned int numVertexes = (slices + 1) * (slices + 1);

	unsigned int vertCount = 0;

	float sliceSize = 1.0f / (float)slices;
	for(float v = 0.0f; v <= 1.0f; v += sliceSize){
		for(float u = 0.0f; u <= 1.0f; u += sliceSize){
			float x = sinf(M_PI * v) * cosf(2 * M_PI * u);
			float y = cosf(M_PI * v);
			float z = sinf(M_PI * v) * sinf(2 * M_PI * u);

			Vector3 normal(x, y, z);

			Vector3 position = center + (normal * radius);

			mb.set_uv(u, v);

			// Calculate tangent space 
			// d/du
			float du_x = sinf(M_PI * v) * -sinf(2 * M_PI * u);
			float du_y = 0.0f;
			float du_z = sinf(M_PI * v) * cosf(2 * M_PI * u);

			Vector3 tangent = Vector3(du_x, du_y, du_z);
			tangent.Normalize();

			mb.set_normal(normal);
			mb.set_tangent(tangent);
			mb.set_bitangent(CrossProduct(tangent, normal));

			mb.add_vertex(position);
			
			// Add indexes to index buffer, but only for the quads that are on the slice
			if(vertCount > slices && vertCount < numVertexes && u < 1.0f ){
				mb.add_index(vertCount);
				mb.add_index(vertCount - slices);
				mb.add_index(vertCount - slices - 1);

				mb.add_index(vertCount);
				mb.add_index(vertCount + 1);
				mb.add_index(vertCount - slices);
			}

			vertCount++;
		}
	}

	mb.end();
}