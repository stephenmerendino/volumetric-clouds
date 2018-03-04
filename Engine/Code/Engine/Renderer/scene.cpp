#include "Engine/Renderer/scene.h"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/CubeMeshes.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/RenderableMesh.hpp"

Scene::Scene()
{
}

Scene::~Scene()
{
}

RenderableMesh* Scene::add_cube()
{
    MeshBuilder mb;
    Meshes::build_cube_3d(mb, Vector3(0.0f, 0.0f, 0.0f), 1.0f);

    Mesh* mesh = new Mesh();
    mb.copy_to_mesh(mesh);

    const Material* cube_mat = Material::DEBUG_MATERIAL;

    RenderableMesh* render_mesh = new RenderableMesh(mesh, cube_mat);
    m_renderable_meshes.push_back(render_mesh);

    return render_mesh;
}