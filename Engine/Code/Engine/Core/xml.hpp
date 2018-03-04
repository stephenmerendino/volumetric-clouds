#pragma once

#include "ThirdParty/XMLParser/XMLParser.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <string>
#include <vector>

typedef XMLNode XMLElement;

namespace xml
{
	bool			is_file_xml(const char* filename);
	XMLElement		parse_xml_file(const char* filename);

	bool			element_has_child(const XMLElement& parent_element, const char* child_name);
	XMLElement		get_element_child(const XMLElement& parent_element, const char* child_name);

	bool			element_has_attribute(const XMLElement& element, const char* attribute);

	int				get_element_num_attributes(const XMLElement& element);
	int				get_element_num_children(const XMLElement& element);

	std::string		get_element_name(const XMLElement& element);
	std::string		get_element_contents(const XMLElement& element);

	std::string		get_element_attribute_name_by_index(const XMLElement& element, int index);
	std::string		get_element_attribute_value_by_index(const XMLElement& element, int index);
	XMLElement		get_element_child_by_index(const XMLElement& element, int index);

	int				parse_xml_attribute(const XMLElement& element, const char* name, int default_value);
	unsigned int	parse_xml_attribute(const XMLElement& element, const char* name, unsigned int default_value);
	char			parse_xml_attribute(const XMLElement& element, const char* name, char default_value);
	bool			parse_xml_attribute(const XMLElement& element, const char* name, bool default_value);
	float			parse_xml_attribute(const XMLElement& element, const char* name, float default_value);
	Rgba			parse_xml_attribute(const XMLElement& element, const char* name, const Rgba& default_value);
	Vector2			parse_xml_attribute(const XMLElement& element, const char* name, const Vector2& default_value);
	Vector3			parse_xml_attribute(const XMLElement& element, const char* name, const Vector3& default_value);
	IntVector2		parse_xml_attribute(const XMLElement& element, const char* name, const IntVector2& default_value);
	IntRange 		parse_xml_attribute(const XMLElement& element, const char* name, const IntRange& default_value);
	FloatRange 		parse_xml_attribute(const XMLElement& element, const char* name, const FloatRange& default_value);
	std::string 	parse_xml_attribute(const XMLElement& element, const char* name, const std::string& default_value);
	std::string 	parse_xml_attribute(const XMLElement& element, const char* name, const char* default_value);

	std::vector<std::string> parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<std::string>& default_value);
	std::vector<Rgba> parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<Rgba>& default_value);
	std::vector<char> parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<char>& default_value);
	std::vector<IntVector2> parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<IntVector2>& default_value);

	// lists are passed in as comma-delimited strings
	void			validate_xml_node(const XMLElement& element, 
									  const char* whitelisted_children = "", 
									  const char* whitelisted_attributes = "",
									  const char* required_children = "", 
									  const char* required_attributes = "");
}