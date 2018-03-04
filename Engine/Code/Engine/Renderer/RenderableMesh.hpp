#pragma once

#include "Engine/Renderer/Renderable.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Renderer/SkeletalTransformHierarchy.hpp"

class Mesh;
class Material;

class RenderableMesh : public Renderable
{
public:
	Mesh* m_mesh;
	const Material* m_material;
    // Transform m_transform;

public:
	RenderableMesh();
	RenderableMesh(Mesh* mesh, const Material* material);
	~RenderableMesh();

	void set_mesh(Mesh* mesh);
	void set_material(const Material* material);
	void set_material(const char* material_name);
	void set_material_instance(const Material* material);
	void set_material_instance(const char* material_name);

	void draw() override;
};