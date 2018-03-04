#pragma once

#include "Engine/RHI/Shader.hpp"
#include "Engine/Core/xml.hpp"

#include <map>
#include <string>

#define MAX_NUM_TEXTURES D3D11_COMMONSHADER_INPUT_RESOURCE_REGISTER_COUNT
#define MAX_NUM_CONSTANT_BUFFERS D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT

class MaterialInstance;

class Material
{
friend class SimpleRenderer;

public:
    static void init();
	static const Material* find_or_create(const char* mat_xml_file);
    static bool validate_xml(const char* xml_filename);

	static std::map<std::string, Material*> s_database;
    static const Material* DEBUG_MATERIAL;

public:
	Material(Shader* shader);
	Material(const Material& mat_to_copy);
	virtual ~Material();

	MaterialInstance*	create_instance() const;

	RHITexture2D*		get_texture(int index) const;
	RHITexture2D*		get_diffuse() const;
	RHITexture2D*		get_normal() const;
	RHITexture2D*		get_spec() const;

	ConstantBuffer*		get_constant_buffer(int index) const;

protected:
	Shader*				m_shader;

	RHITexture2D*		m_textures[MAX_NUM_TEXTURES];
	ConstantBuffer*		m_constant_buffers[MAX_NUM_CONSTANT_BUFFERS];

	void set_texture(RHITexture2D* texture);
	void set_texture(int index, RHITexture2D* texture);

	void set_diffuse(RHITexture2D* diffuse);
	void set_normal(RHITexture2D* normal);
	void set_spec(RHITexture2D* spec);

	void set_constant_buffer(ConstantBuffer* constant_buffer);
	void set_constant_buffer(int index, ConstantBuffer* constant_buffer);

private:
	Material(const char* mat_xml_file);

	RHITexture2D* create_texture_from_xml(const XMLElement& texture_xml);
};

class MaterialInstance : public Material
{
public:
	MaterialInstance(const Material& mat);
	~MaterialInstance();

	using Material::set_texture;
	using Material::set_diffuse;
	using Material::set_normal;
	using Material::set_spec;
	using Material::set_constant_buffer;

	void set_texture(const char* bind_name, RHITexture2D* texture);
};