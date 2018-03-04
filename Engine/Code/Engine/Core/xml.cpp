#include "Engine/Core/xml.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console.hpp"

//---------------------------------------------------------------
// helpers
//---------------------------------------------------------------

static void advance_to_next_token(std::string& text)
{
	while(text[0] == ' ' || text[0] == ',' || text[0] == '~'){
		text.erase(0, 1);
	}
}

static float parse_next_float(std::string& text)
{
	advance_to_next_token(text);

	int index = 0;
	while(text[index] != ' ' && text[index] != ',' && text[index] != '~' && text[index] != NULL){
		index++;
	}

	std::string float_text = text.substr(0, index);
	text.erase(0, index);

	return (float)atof(float_text.data());
}

static int parse_next_int(std::string& text)
{
	advance_to_next_token(text);

	int index = 0;
	while(text[index] != ' ' && text[index] != ',' && text[index] != '~' && text[index] != NULL){
		index++;
	}

	std::string int_text = text.substr(0, index);
	text.erase(0, index);

	return atoi(int_text.data());
}

static unsigned char parse_next_uchar(std::string& text)
{
	advance_to_next_token(text);

	int index = 0;
	while(text[index] != ' ' && text[index] != ',' && text[index] != NULL){
		index++;
	}

	std::string uchar_text = text.substr(0, index);
	text.erase(0, index);

	return (unsigned char)atoi(uchar_text.data());
}

static std::string parse_next_string(std::string& text)
{
	advance_to_next_token(text);

	int end_index = 0;

	// parsing in a parentheses item, we will keep the beginning and ending parentheses
	if(text[0] == '('){
		while(text[end_index] != ')' && text[end_index] != NULL){
			++end_index;
		}

		end_index++;

		std::string word = text.substr(0, end_index);
		text.erase(0, end_index);

		return word;
	}

	// parsing in a single quoted item, we will remove the quotes but keep the entire string
	if(text[0] == '\''){
		end_index = 1;

		while(text[end_index] != '\'' && text[end_index] != NULL){
			++end_index;
		}

		std::string word = text.substr(1, end_index - 1);
		text.erase(0, end_index + 1);

		return word;
	}

	// normal behavior
	while(text[end_index] != ' ' && text[end_index] != ',' && text[end_index] != NULL){
		++end_index;
	}

	std::string word = text.substr(0, end_index);
	text.erase(0, end_index);

	return word;
}

static std::vector<std::string> parse_list_to_vector(const char* list)
{
	std::vector<std::string> list_vector;

	std::string full_list(list);

	while(!full_list.empty()){
		std::string word = parse_next_string(full_list);
		list_vector.push_back(word);
	}

	return list_vector;
}

static Rgba parse_rgba_from_string(std::string& raw_rgba)
{
	if(raw_rgba == "white")  return Rgba::WHITE;
	if(raw_rgba == "red")    return Rgba::RED;
	if(raw_rgba == "green")  return Rgba::GREEN;
	if(raw_rgba == "blue")   return Rgba::BLUE;
	if(raw_rgba == "yellow") return Rgba::YELLOW;
	if(raw_rgba == "orange") return Rgba::ORANGE;
	if(raw_rgba == "pink")   return Rgba::PINK;
	if(raw_rgba == "purple") return Rgba::PURPLE;
	if(raw_rgba == "black")  return Rgba::BLACK;
	if(raw_rgba == "grey")   return Rgba::GREY;
	if(raw_rgba == "gray")   return Rgba::GREY;

	if(raw_rgba[0] == '('){
		char last_char = raw_rgba[raw_rgba.size() - 1];

		std::string error = Stringf("Error: invalid rgba syntax [%s] detected in xml", raw_rgba.c_str());
		ASSERT_OR_DIE(last_char == ')', error.c_str());

		// erase the beginning '(' and ending ')'
		raw_rgba.erase(0, 1);
		raw_rgba.erase(raw_rgba.size() - 1, 1);
	}
	
	unsigned char r = parse_next_uchar(raw_rgba);
	unsigned char g = parse_next_uchar(raw_rgba);
	unsigned char b = parse_next_uchar(raw_rgba);

	unsigned char a = 255;
	if(raw_rgba.size() != 0){
		a = parse_next_uchar(raw_rgba);
	}

	return Rgba(r, g, b, a);
}

static IntVector2 parse_int_vector2_from_string(std::string& raw_iv2)
{
	if(raw_iv2[0] == '('){
		char last_char = raw_iv2[raw_iv2.size() - 1];

		std::string error = Stringf("Error: invalid iv2 syntax [%s] detected in xml", raw_iv2.c_str());
		ASSERT_OR_DIE(last_char == ')', error.c_str());

		// erase the beginning '(' and ending ')'
		raw_iv2.erase(0, 1);
		raw_iv2.erase(raw_iv2.size() - 1, 1);
	}
	
	int x = parse_next_int(raw_iv2);
	int y = parse_next_int(raw_iv2);

	return IntVector2(x, y);
}

//---------------------------------------------------------------
// public api
//---------------------------------------------------------------

bool xml::is_file_xml(const char* filename)
{
	XMLResults parse_results;
	XMLNode::parseFile(filename, 0, &parse_results);
	return parse_results.error == eXMLErrorNone;
}

XMLElement xml::parse_xml_file(const char* filename)
{
	return XMLNode::parseFile(filename);
}

bool xml::element_has_child(const XMLElement& parent_element, const char* child_name)
{
	for(int i = 0; i < get_element_num_children(parent_element); ++i){
		XMLElement child = get_element_child_by_index(parent_element, i);
		if(get_element_name(child) == child_name){
			return true;
		}
	}

	return false;
}

XMLElement xml::get_element_child(const XMLElement& parent_element, const char* child_name)
{
	for(int i = 0; i < get_element_num_children(parent_element); ++i){
		XMLElement child = get_element_child_by_index(parent_element, i);
		if(get_element_name(child) == child_name){
			return child;
		}
	}

	std::string no_child_error = Stringf("Error: xml element [%s] has not child with name [%s]", parent_element.getName(), child_name);
	DIE(no_child_error.c_str());
}

bool xml::element_has_attribute(const XMLElement& element, const char* attribute)
{
	for(int i = 0; i < get_element_num_attributes(element); ++i){
		std::string attribute_name = get_element_attribute_name_by_index(element, i);
		if(attribute_name == attribute){
			return true;
		}
	}

	return false;
}

int xml::get_element_num_attributes(const XMLElement& element)
{
	return element.nAttribute();
}

int xml::get_element_num_children(const XMLElement& element)
{
	return element.nChildNode();
}

std::string xml::get_element_name(const XMLElement& element)
{
	return element.getName();
}

std::string xml::get_element_contents(const XMLElement& element)
{
	return element.getText() != nullptr ? element.getText() : "";
}

std::string xml::get_element_attribute_name_by_index(const XMLElement& element, int index)
{
	return element.getAttributeName(index);
}

std::string xml::get_element_attribute_value_by_index(const XMLElement& element, int index)
{
	return element.getAttributeValue(index);
}

XMLElement xml::get_element_child_by_index(const XMLElement& element, int index)
{
	return element.getChildNode(index);
}

int xml::parse_xml_attribute(const XMLElement& element, const char* name, int default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	return atoi(raw_value);
}

unsigned int xml::parse_xml_attribute(const XMLElement& element, const char* name, unsigned int default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	return strtoul(raw_value, nullptr, 0);
}

char xml::parse_xml_attribute(const XMLElement& element, const char* name, char default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	return *raw_value;
}

bool xml::parse_xml_attribute(const XMLElement& element, const char* name, bool default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	if(AreStringsEqualNonCaseSensitive(raw_value, "true")){
		return true;
	}
	else if(AreStringsEqualNonCaseSensitive(raw_value, "false")){
		return false;
	}

	return default_value;
}

float xml::parse_xml_attribute(const XMLElement& element, const char* name, float default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	return (float)atof(raw_value);
}

Rgba xml::parse_xml_attribute(const XMLElement& element, const char* name, const Rgba& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::string rgba_as_std_string = std::string(raw_value, 0, strlen(raw_value));

	return parse_rgba_from_string(rgba_as_std_string);
}

Vector2 xml::parse_xml_attribute(const XMLElement& element, const char* name, const Vector2& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::string vector_text(raw_value);
	float x = parse_next_float(vector_text);
	float y = parse_next_float(vector_text);
	return Vector2(x, y);
}

Vector3 xml::parse_xml_attribute(const XMLElement& element, const char* name, const Vector3& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::string vector_text(raw_value);
	float x = parse_next_float(vector_text);
	float y = parse_next_float(vector_text);
	float z = parse_next_float(vector_text);
	return Vector3(x, y, z);
}

IntVector2 xml::parse_xml_attribute(const XMLElement& element, const char* name, const IntVector2& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::string vector_text(raw_value);
	int x = parse_next_int(vector_text);
	int y = parse_next_int(vector_text);
	return IntVector2(x, y);
}

IntRange xml::parse_xml_attribute(const XMLElement& element, const char* name, const IntRange& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::string range_text(raw_value);
	int min = parse_next_int(range_text);

	int max = min;
	if(!range_text.empty()){
		max = parse_next_int(range_text);
	}

	return IntRange(min, max);
}

FloatRange xml::parse_xml_attribute(const XMLElement& element, const char* name, const FloatRange& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::string range_text(raw_value);
	float min = parse_next_float(range_text);

	float max = min;
	if(!range_text.empty()){
		max = parse_next_float(range_text);
	}

	return FloatRange(min, max);
}

std::string xml::parse_xml_attribute(const XMLElement& element, const char* name, const std::string& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return std::string(default_value);
	}

	return std::string(raw_value);
}

std::string xml::parse_xml_attribute(const XMLElement& element, const char* name, const char* default_value)
{
	return parse_xml_attribute(element, name, std::string(default_value));
}

std::vector<std::string> xml::parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<std::string>& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	return parse_list_to_vector(raw_value);
}

std::vector<Rgba> xml::parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<Rgba>& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::vector<std::string> raw_color_strings = parse_list_to_vector(raw_value);

	std::vector<Rgba> colors;
	for(std::string& raw_color : raw_color_strings){
		colors.push_back(parse_rgba_from_string(raw_color));
	}

	return colors;
}

std::vector<char> xml::parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<char>& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::vector<std::string> raw_char_strings = parse_list_to_vector(raw_value);

	std::vector<char> chars;
	for(std::string& raw_char : raw_char_strings){
		std::string error = Stringf("Error: more than one char found when trying to parse single char for element [%s, %s]", element.getName(), name);
		ASSERT_OR_DIE(raw_char.size() == 1, error.c_str());
		chars.push_back(raw_char[0]);
	}

	return chars;
}

std::vector<IntVector2> xml::parse_xml_attribute(const XMLElement& element, const char* name, const std::vector<IntVector2>& default_value)
{
	const char* raw_value = element.getAttribute(name);

	if(raw_value == NULL){
		return default_value;
	}

	std::vector<std::string> raw_iv_strings = parse_list_to_vector(raw_value);

	std::vector<IntVector2> ivs;
	for(std::string& raw_iv : raw_iv_strings){
		ivs.push_back(parse_int_vector2_from_string(raw_iv));
	}

	return ivs;
}

void xml::validate_xml_node(const XMLElement& element,
					   const char* whitelisted_children,
					   const char* whitelisted_attributes,
					   const char* required_children,
					   const char* required_attributes)
{
	std::string element_name = get_element_name(element);

	int num_children = get_element_num_children(element);
	int num_attributes = get_element_num_attributes(element);

	std::vector<std::string> parsed_whitelisted_children = parse_list_to_vector(whitelisted_children);
	std::vector<std::string> parsed_whitelisted_attributes = parse_list_to_vector(whitelisted_attributes);

	std::vector<std::string> parsed_required_children = parse_list_to_vector(required_children);
	std::vector<std::string> parsed_required_attributes = parse_list_to_vector(required_attributes);

	// whitelist children
	for(int child_index = 0; child_index < num_children; ++child_index){
		XMLElement child = get_element_child_by_index(element, child_index);
		std::string child_name = get_element_name(child);

		bool is_whitelisted = false;
		for(const std::string& name : parsed_whitelisted_children){
			if(child_name == name){
				is_whitelisted = true;
				break;
			}
		}

		// make sure its whitelisted
		if(!is_whitelisted){
			std::string error = Stringf("Error: non-whitelisted child [%s] found in element [%s]", child_name.c_str(), element_name.c_str());
			DIE(error.c_str());
		}

		// remove it from the required list if it is required
		for(size_t req_name_index = 0; req_name_index < parsed_required_children.size(); ++req_name_index){
			if(child_name == parsed_required_children[req_name_index]){
				parsed_required_children.erase(parsed_required_children.begin() + req_name_index);
				break;
			}
		}
	}

	// make sure we found all of the required children
	if(!parsed_required_children.empty()){
		std::string error = Stringf("Error: missing required child [%s] in element [%s]", parsed_required_children[0].c_str(), element_name.c_str());
		DIE(error.c_str());
	}

	// whitelist attributes
	for(int attr_index = 0; attr_index < num_attributes; ++attr_index){
		std::string attr_name = get_element_attribute_name_by_index(element, attr_index);

		bool is_whitelisted = false;
		for(const std::string& name : parsed_whitelisted_attributes){
			if(attr_name == name){
				is_whitelisted = true;
				break;
			}
		}

		// make sure its whitelisted
		if(!is_whitelisted){
			std::string error = Stringf("Error: non-whitelisted attrbute [%s] found in element [%s]", attr_name.c_str(), element_name.c_str());
			DIE(error.c_str());
		}

		// remove it from the required list if it is required
		for(size_t req_attr_index = 0; req_attr_index < parsed_required_attributes.size(); ++req_attr_index){
			if(attr_name == parsed_required_attributes[req_attr_index]){
				parsed_required_attributes.erase(parsed_required_attributes.begin() + req_attr_index);
				break;
			}
		}
	}

	// make sure we found all of the required attributes
	if(!parsed_required_attributes.empty()){
		std::string error = Stringf("Error: missing required attribute [%s] in element [%s]", parsed_required_attributes[0].c_str(), element_name.c_str());
		DIE(error.c_str());
	}
}