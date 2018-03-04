#include "Engine/RHI/Shader.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Core/xml.hpp"
#include "Engine/RHI/ShaderProgram.hpp"

//------------------------------------------------------------------------------
// Shader
//------------------------------------------------------------------------------

std::map<std::string, Shader*> Shader::s_database;

Shader* Shader::find_or_create(const char* shader_xml_file)
{
	std::map<std::string, Shader*>::iterator found = s_database.find(shader_xml_file);
	if(found != s_database.end()){
		return found->second;
	}

	Shader* new_shader = new Shader(shader_xml_file);
	return new_shader;
}

Shader::Shader(const char* shader_xml_file)
	:m_raster_state(SimpleRenderer::DEFAULT_RASTER_STATE)
	,m_depth_stencil_state(SimpleRenderer::DEFAULT_DEPTH_STENCIL_STATE)
	,m_blend_state(SimpleRenderer::DEFAULT_BLEND_STATE)
{
	XMLElement shader_xml = xml::parse_xml_file(shader_xml_file);
	xml::validate_xml_node(shader_xml, "shader_program, blend, raster, depth", "", "shader_program", "");

	// create shader program
	XMLElement shader_program_xml = xml::get_element_child(shader_xml, "shader_program");
	m_shader_program = create_shader_program_from_xml(shader_program_xml);

	// create raster state if provided
	if(xml::element_has_child(shader_xml, "raster")){
		XMLElement raster_xml = xml::get_element_child(shader_xml, "raster");
		m_raster_state = create_raster_state_from_xml(raster_xml);
	}

	// create depth state if provided
	if(xml::element_has_child(shader_xml, "depth")){
		XMLElement depth_xml = xml::get_element_child(shader_xml, "depth");
		m_depth_stencil_state = create_depth_state_from_xml(depth_xml);
	}

	// create blend state if provided
	if(xml::element_has_child(shader_xml, "blend")){
		XMLElement blend_xml = xml::get_element_child(shader_xml, "blend");
		m_blend_state = create_blend_state_from_xml(blend_xml);
	}

	s_database[shader_xml_file] = this;
}

Shader::Shader(ShaderProgram* shader_program,
			   RasterState* rs,
			   DepthStencilState* dss,
			   BlendState* bs)
	:m_shader_program(shader_program)
	,m_raster_state(rs)
	,m_depth_stencil_state(dss)
	,m_blend_state(bs)
{
}

Shader::~Shader()
{
}

Shader* Shader::clone() const
{
	return new Shader(m_shader_program, m_raster_state, m_depth_stencil_state, m_blend_state);
}

void Shader::set_shader_program(ShaderProgram* shader_program){ m_shader_program = shader_program; }

void Shader::set_raster_state(RasterState* raster_state) { m_raster_state = raster_state; }
void Shader::set_depth_stencil_state(DepthStencilState* depth_stencil_state) { m_depth_stencil_state = depth_stencil_state; }
void Shader::set_blend_state(BlendState* blend_state) { m_blend_state = blend_state; }

RasterState*		Shader::get_raster_state() const { return m_raster_state; }
DepthStencilState*	Shader::get_depth_stencil_state() const { return m_depth_stencil_state; }
BlendState*			Shader::get_blend_state() const { return m_blend_state; }

ShaderProgram* Shader::create_shader_program_from_xml(const XMLElement& shader_program_xml)
{
	xml::validate_xml_node(shader_program_xml, "vertex, fragment", "src", "", "");

	// read in single hlsl file using src attr
	if(xml::element_has_attribute(shader_program_xml, "src")){
		std::string single_src_filename = xml::parse_xml_attribute(shader_program_xml, "src", single_src_filename);
		return new ShaderProgram(g_theRenderer->m_device, single_src_filename.c_str());
	}
	// read in vertex and fragment children elements
	else{
		XMLElement vertex_xml = xml::get_element_child(shader_program_xml, "vertex");
		XMLElement fragment_xml = xml::get_element_child(shader_program_xml, "fragment");

		xml::validate_xml_node(vertex_xml, "", "src", "", "src");
		xml::validate_xml_node(fragment_xml, "", "src", "", "src");

		std::string vertex_filename = xml::parse_xml_attribute(vertex_xml, "src", vertex_filename);
		std::string fragment_filename = xml::parse_xml_attribute(fragment_xml, "src", fragment_filename);

        std::string geometry_filename_xml;
        if(xml::element_has_child(shader_program_xml, "geometry")){
    		XMLElement geometry_xml = xml::get_element_child(shader_program_xml, "geometry");
    		xml::validate_xml_node(geometry_xml, "", "src", "", "src");
    		geometry_filename_xml = xml::parse_xml_attribute(geometry_xml, "src", geometry_filename_xml);
        }
        const char* geometry_filename = geometry_filename_xml.empty() ? nullptr : geometry_filename_xml.c_str(); 

		return new ShaderProgram(g_theRenderer->m_device, vertex_filename.c_str(), geometry_filename, fragment_filename.c_str());
	}
}

RasterState* Shader::create_raster_state_from_xml(const XMLElement& raster_xml)
{
	xml::validate_xml_node(raster_xml, "", "cull, fill", "", "cull, fill");

	std::string cull_str = xml::parse_xml_attribute(raster_xml, "cull", cull_str);
	std::string fill_str = xml::parse_xml_attribute(raster_xml, "fill", fill_str);

	return new RasterState(g_theRenderer->m_device, cull_str, fill_str);
}

DepthStencilState* Shader::create_depth_state_from_xml(const XMLElement& depth_xml)
{
	xml::validate_xml_node(depth_xml, "", "write, test", "", "write, test");

	std::string write_str = xml::parse_xml_attribute(depth_xml, "write", write_str);
	std::string test_str = xml::parse_xml_attribute(depth_xml, "test", test_str);

	return new DepthStencilState(g_theRenderer->m_device, write_str, test_str);
}

BlendState* Shader::create_blend_state_from_xml(const XMLElement& blend_xml)
{
	xml::validate_xml_node(blend_xml, "color, alpha", "", "color, alpha", "");

	XMLElement color_xml = xml::get_element_child(blend_xml, "color");
	std::string color_src_str = xml::parse_xml_attribute(color_xml, "src", color_src_str);
	std::string color_dest_str = xml::parse_xml_attribute(color_xml, "dest", color_dest_str);
	std::string color_op_str = xml::parse_xml_attribute(color_xml, "op", color_op_str);

	XMLElement alpha_xml = xml::get_element_child(blend_xml, "alpha");
	std::string alpha_src_str = xml::parse_xml_attribute(alpha_xml, "src", alpha_src_str);
	std::string alpha_dest_str = xml::parse_xml_attribute(alpha_xml, "dest", alpha_dest_str);
	std::string alpha_op_str = xml::parse_xml_attribute(alpha_xml, "op", alpha_op_str);

	return new BlendState(g_theRenderer->m_device, color_src_str, color_dest_str, color_op_str, 
												   alpha_src_str, alpha_dest_str, alpha_op_str);
}

int Shader::find_bind_index_for_name(const char* bind_name) const
{
	return m_shader_program->find_bind_index_for_name(bind_name);
}