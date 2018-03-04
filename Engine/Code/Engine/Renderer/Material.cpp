#include "Engine/Renderer/Material.hpp"
#include "Engine/Engine.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/RHI/ShaderProgram.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//----------------------------------------------------------------------
// Material
//----------------------------------------------------------------------

std::map<std::string, Material*> Material::s_database;
const Material* Material::DEBUG_MATERIAL = nullptr;

void Material::init()
{
    DEBUG_MATERIAL = find_or_create("Data/Materials/debug.mat");
}

const Material* Material::find_or_create(const char* mat_xml_file)
{
	std::map<std::string, Material*>::iterator found = s_database.find(mat_xml_file);
	if(found != s_database.end()){
		return found->second;
	}

    const Material* new_mat;
    if(Material::validate_xml(mat_xml_file)){
    	new_mat = new Material(mat_xml_file);
    }else{
        new_mat = Material::DEBUG_MATERIAL;
    }

	return new_mat;
}

bool Material::validate_xml(const char* xml_filename)
{
    FILE* f;
    errno_t error = fopen_s(&f, xml_filename, "r");

    if(0 == error){
        fclose(f);
        return true;
    }else{
        return false;
    }
}

Material::Material(const char* mat_xml_file)
{
	memset(&m_textures[0], 0, sizeof(RHITexture2D*) * MAX_NUM_TEXTURES);
	memset(&m_constant_buffers[0], 0, sizeof(ConstantBuffer*) * MAX_NUM_CONSTANT_BUFFERS);

	// validate xml
	XMLElement mat_xml = xml::parse_xml_file(mat_xml_file);
	xml::validate_xml_node(mat_xml, "shader, diffuse, normal, spec, texture", "", "shader", "");

	// find or create shader
	XMLElement shader_xml = xml::get_element_child(mat_xml, "shader");
	xml::validate_xml_node(shader_xml, "", "src", "", "src");
	std::string shader_src = xml::parse_xml_attribute(shader_xml, "src", shader_src);
	m_shader = Shader::find_or_create(shader_src.c_str());
	
	// create and save diffuse
	if(xml::element_has_child(mat_xml, "diffuse")){
		XMLElement diffuse_xml = xml::get_element_child(mat_xml, "diffuse");
		RHITexture2D* diffuse = create_texture_from_xml(diffuse_xml);
		set_diffuse(diffuse);
	}

	// create and save normal
	if(xml::element_has_child(mat_xml, "normal")){
		XMLElement normal_xml = xml::get_element_child(mat_xml, "normal");
		RHITexture2D* normal = create_texture_from_xml(normal_xml);
		set_normal(normal);
	}

	// create and save spec
	if(xml::element_has_child(mat_xml, "spec")){
		XMLElement spec_xml = xml::get_element_child(mat_xml, "spec");
		RHITexture2D* spec = create_texture_from_xml(spec_xml);
		set_spec(spec);
	}

	// loop through other textures
	int num_children = xml::get_element_num_children(mat_xml);
	for(int child_index = 0; child_index < num_children; ++child_index){
		XMLElement child = xml::get_element_child_by_index(mat_xml, child_index);
		if(xml::get_element_name(child) == "texture"){
			xml::validate_xml_node(child, "", "name, index, src, color", "", "");

			RHITexture2D* texture = create_texture_from_xml(child);

			int texture_index = INVALID_BIND_INDEX;
			if(xml::element_has_attribute(child, "name")){
				std::string name = xml::parse_xml_attribute(child, "name", name);
				texture_index = m_shader->find_bind_index_for_name(name.c_str());
			}
			else if(xml::element_has_attribute(child, "index")){
				texture_index = xml::parse_xml_attribute(child, "index", texture_index);
			}

			if(texture_index == INVALID_BIND_INDEX){
				continue;
			}
			
			set_texture(texture_index, texture);
		}
	}

	s_database[mat_xml_file] = this;
}

Material::Material(Shader* shader)
	:m_shader(shader)
{
	memset(&m_textures[0], 0, sizeof(RHITexture2D*) * MAX_NUM_TEXTURES);
	memset(&m_constant_buffers[0], 0, sizeof(ConstantBuffer*) * MAX_NUM_CONSTANT_BUFFERS);
}

Material::Material(const Material& mat_to_copy)
	:m_shader(mat_to_copy.m_shader)
{
	memcpy(&m_textures[0], &mat_to_copy.m_textures[0], sizeof(RHITexture2D*) * MAX_NUM_TEXTURES);
	memcpy(&m_constant_buffers[0], &mat_to_copy.m_constant_buffers[0], sizeof(ConstantBuffer*) * MAX_NUM_CONSTANT_BUFFERS);
}

Material::~Material()
{
    for(int i = 0; i < MAX_NUM_TEXTURES; i++){
        SAFE_DELETE(m_textures[i]);
    }

    for(int i = 0; i < MAX_NUM_CONSTANT_BUFFERS; i++){
        SAFE_DELETE(m_constant_buffers[i]);
    }
}

MaterialInstance* Material::create_instance() const 
{ 
	return new MaterialInstance(*this); 
}

RHITexture2D* Material::get_texture(int index) const
{
	return m_textures[index];
}

RHITexture2D* Material::get_diffuse() const { return get_texture(0); }
RHITexture2D* Material::get_normal() const { return get_texture(1); }
RHITexture2D* Material::get_spec() const { return get_texture(2); }

ConstantBuffer* Material::get_constant_buffer(int index) const
{
	return m_constant_buffers[index];
}

void Material::set_texture(RHITexture2D* texture){ set_texture(0, texture); }
void Material::set_texture(int index, RHITexture2D* texture){ m_textures[index] = texture; }

void Material::set_diffuse(RHITexture2D* diffuse){ set_texture(0, diffuse); }
void Material::set_normal(RHITexture2D* normal){ set_texture(1, normal); }
void Material::set_spec(RHITexture2D* spec){ set_texture(2, spec); }

void Material::set_constant_buffer(ConstantBuffer* constant_buffer){ set_constant_buffer(0, constant_buffer); }
void Material::set_constant_buffer(int index, ConstantBuffer* constant_buffer){ m_constant_buffers[index] = constant_buffer; }

RHITexture2D* Material::create_texture_from_xml(const XMLElement& texture_xml)
{
	xml::validate_xml_node(texture_xml, "", "name, src, color", "", "");

	if(xml::element_has_attribute(texture_xml, "color")){
		std::string color_string = xml::parse_xml_attribute(texture_xml, "color", color_string);
		Rgba color(color_string);
		return g_theRenderer->m_device->CreateRHITexture2DFromColor(color);
	}
	else{
		std::string texture_filename = xml::parse_xml_attribute(texture_xml, "src", texture_filename);
		return g_theRenderer->m_device->FindOrCreateRHITexture2DFromFile(texture_filename.c_str());
	}
}

//----------------------------------------------------------------------
// Material Instance
//----------------------------------------------------------------------

MaterialInstance::MaterialInstance(const Material& mat)
	:Material(mat)
{
}

MaterialInstance::~MaterialInstance()
{
}

void MaterialInstance::set_texture(const char* bind_name, RHITexture2D* texture)
{
	int bind_index = m_shader->find_bind_index_for_name(bind_name);

	std::string error = Stringf("Error: Could not find a bind index for resource with name [%s]", bind_name);
	ASSERT_OR_DIE(bind_index != INVALID_BIND_INDEX, error.c_str());

	set_texture(bind_index, texture);
}