#include "Engine/Renderer/RenderableMesh.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/SimpleRenderer.hpp"

RenderableMesh::RenderableMesh()
	:Renderable()
	,m_mesh(nullptr)
	,m_material(nullptr)
{
}

RenderableMesh::RenderableMesh(Mesh* mesh, const Material* material)
	:Renderable()
	,m_mesh(mesh)
	,m_material(material)
{
}

RenderableMesh::~RenderableMesh()
{
	Renderable::~Renderable();
}

void RenderableMesh::set_mesh(Mesh* mesh)
{
	m_mesh = mesh;
}

void RenderableMesh::set_material(const Material* material)
{
	m_material = material;
}

void RenderableMesh::set_material(const char* material_name)
{
    const Material* mat = Material::find_or_create(material_name);
    if(nullptr != mat){
        m_material = mat;
    }else{
        m_material = Material::DEBUG_MATERIAL;
    }
}

void RenderableMesh::set_material_instance(const Material* material)
{
    if(nullptr != material){
        m_material = material->create_instance();
    }else{
        m_material = Material::DEBUG_MATERIAL->create_instance();
    }
}

void RenderableMesh::set_material_instance(const char* material_name)
{
    const Material* mat = Material::find_or_create(material_name);
    if(nullptr != mat){
        m_material = mat->create_instance();
    }else{
        m_material = Material::DEBUG_MATERIAL->create_instance();
    }
}

void RenderableMesh::draw()
{
	g_theRenderer->set_material(m_material);
	g_theRenderer->draw_mesh(m_mesh);
}