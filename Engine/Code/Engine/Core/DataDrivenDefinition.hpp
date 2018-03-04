#pragma once

#include "Engine/Core/xml.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <string>
#include <map>

//--------------------------------------
// Class Decl
//--------------------------------------

template<typename T>
class DataDrivenDefinition
{
public:
	std::string m_name;

public:
	static const T* register_new_definition(const XMLElement& def_xml);

	static const size_t get_num_definitions();
	static const T* get_definition_by_index(unsigned int def_index);
	static const T* find_definition(const std::string& def_name);

	static void clear_all();

protected:
	DataDrivenDefinition(const XMLElement& def_xml);
	virtual ~DataDrivenDefinition();

	static std::map<std::string, T*> s_registry;
};

//--------------------------------------
// Class Impl
//--------------------------------------

template<typename T>
const T* DataDrivenDefinition<T>::register_new_definition(const XMLElement& def_xml)
{
	T* new_def = new T(def_xml);

	// make sure there isn't already a definition with this name
	std::string error = Stringf("Error: Duplicate definition [%s] registered", new_def->m_name.c_str());
	ASSERT_OR_DIE(!find_definition(new_def->m_name), error.c_str());

	s_registry[new_def->m_name] = new_def;

	return new_def;
}

template<typename T>
const size_t DataDrivenDefinition<T>::get_num_definitions()
{
	return s_registry.size();
}

template<typename T>
const T* DataDrivenDefinition<T>::get_definition_by_index(unsigned int def_index)
{
	std::map<std::string, T*>::iterator it = s_registry.begin();

	while(def_index > 0){
		++it;
		--def_index;
	}

	return it->second;
}

template<typename T>
const T* DataDrivenDefinition<T>::find_definition(const std::string& def_name)
{
	std::map<std::string, T*>::iterator it = s_registry.find(def_name);

	if(it == s_registry.end()){
		return nullptr;
	}

	return it->second;
}

template<typename T>
void DataDrivenDefinition<T>::clear_all()
{
	std::map<std::string, T*>::iterator it;
	for(it = s_registry.begin(); it != s_registry.end(); ++it){
		T*& def = it->second;
		if(def){
			delete def;
			def = nullptr;
		}
	}

	s_registry.clear();
}

template<typename T>
DataDrivenDefinition<T>::DataDrivenDefinition(const XMLElement & def_xml)
{
	m_name = xml::parse_xml_attribute(def_xml, "name", m_name);
}

template<typename T>
DataDrivenDefinition<T>::~DataDrivenDefinition()
{
	clear_all();
}

template<typename T>
std::map<std::string, T*> DataDrivenDefinition<T>::s_registry;