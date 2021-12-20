#pragma once
#include "exceptions.h"
#include <string>

//class json_base_exception : public lw_base_exception
//{
//public:
//	inline json_base_exception(std::string json_in_string, std::string reson)
//		: lw_base_exception("json get object failed. key: " + key)
//	{ }
//};

class json_parse_exception : public lw_base_exception
{
public:
	json_parse_exception(std::string json_text)
		: lw_base_exception("json string failed to parse: " + json_text.substr(0, 100))
	{ }
};

class json_get_object_exception : public lw_base_exception
{
public:
	json_get_object_exception(std::string key)
		: lw_base_exception("json get object failed. key: " + key)
	{ }
};

class json_get_item_exception : public lw_base_exception
{
public:
	json_get_item_exception(int index)
		: lw_base_exception("json get item failed. index: " + index)
	{ }
};

class json_type_cast_exception : public lw_base_exception
{
public:
	json_type_cast_exception(std::string typeFrom, std::string typeTo)
		: lw_base_exception("failed to cast type from '" + typeFrom + "' to '" + typeTo + "' of json")
	{ }
};

class json_not_array_exception : public lw_base_exception
{
public:
	json_not_array_exception(std::string json_text)
		: lw_base_exception("json object was not a json-array: " + json_text.substr(0, 100))
	{ }
};

class json_not_object_exception : public lw_base_exception
{
public:
	json_not_object_exception(std::string json_text)
		: lw_base_exception("json object was not a json-object: " + json_text.substr(0, 100))
	{ }
};

class json_array_index_out_of_bounds_exception : public lw_base_exception
{
public:
	json_array_index_out_of_bounds_exception(std::string json_text, int indexOutOfBounds)
		: lw_base_exception("attempted to access a item with index (" + std::to_string(indexOutOfBounds) + ") out of bounds: " + json_text.substr(0, 100))
	{ }
};