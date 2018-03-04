#include "Engine/Renderer/SceneMeshes.hpp"

void Meshes::build_scene_axes(MeshBuilder& mb, const Matrix4& local_to_world, float axis_length)
{
	Matrix4 world_to_local = local_to_world.transposed();

	Vector3 origin = local_to_world.get_translation().xyz;

	mb.begin(PRIMITIVE_LINES, false);

	mb.set_color(Rgba::RED);
	mb.add_vertex(origin);
	mb.add_vertex(origin + world_to_local.apply_transformation(Vector4(Vector3::X_AXIS, 0.0f)).xyz * axis_length);

	mb.set_color(Rgba::GREEN);
	mb.add_vertex(origin);
	mb.add_vertex(origin + world_to_local.apply_transformation(Vector4(Vector3::Y_AXIS, 0.0f)).xyz * axis_length);

	mb.set_color(Rgba::BLUE);
	mb.add_vertex(origin);
	mb.add_vertex(origin + world_to_local.apply_transformation(Vector4(Vector3::Z_AXIS, 0.0f)).xyz * axis_length);

	mb.end();
}

void Meshes::build_scene_axes(MeshBuilder& mb, float axis_length)
{
	mb.begin(PRIMITIVE_LINES, false);

	mb.set_color(Rgba::RED);
	mb.add_vertex(0.0f, 0.0f, 0.0f);
	mb.add_vertex(axis_length, 0.0f, 0.0f);

	mb.set_color(Rgba::GREEN);
	mb.add_vertex(0.0f, 0.0f, 0.0f);
	mb.add_vertex(0.0f, axis_length, 0.0f);

	mb.set_color(Rgba::BLUE);
	mb.add_vertex(0.0f, 0.0f, 0.0f);
	mb.add_vertex(0.0f, 0.0f, axis_length);

	mb.end();
}

void Meshes::build_scene_grid_xz(MeshBuilder& mb,
								 float grid_size,
								 float major_unit_stride,
								 float minor_unit_stride,
								 const Rgba& major_unit_color,
								 const Rgba& minor_unit_color)
{
	mb.begin(PRIMITIVE_LINES, false);

	float grid_half_size = grid_size * 0.5f;

	Rgba grid_line_color;
	Vector3 start_pos;
	Vector3 end_pos;

	int major_unit_step_count = (int)(major_unit_stride / minor_unit_stride);

	// Build lines along the x axis
	int current_step = major_unit_step_count;
	for(float z = -grid_half_size; z <= grid_half_size; z += minor_unit_stride){
		start_pos = Vector3(-grid_half_size, 0.0f, z);
		end_pos = Vector3(grid_half_size, 0.0f, z);

		if(current_step % major_unit_step_count == 0){
			mb.set_color(major_unit_color);
		}
		else{
			mb.set_color(minor_unit_color);
		}

		mb.add_vertex(start_pos);
		mb.add_vertex(end_pos);

		current_step++;
	}

	// Build lines along the z axis
	current_step = major_unit_step_count;
	for(float x = -grid_half_size; x <= grid_half_size; x += minor_unit_stride){
		start_pos = Vector3(x, 0.0f, -grid_half_size);
		end_pos = Vector3(x, 0.0f, grid_half_size);

		if(current_step % major_unit_step_count == 0){
			mb.set_color(major_unit_color);
		}
		else{
			mb.set_color(minor_unit_color);
		}

		mb.add_vertex(start_pos);
		mb.add_vertex(end_pos);

		current_step++;
	}

	mb.end();
}