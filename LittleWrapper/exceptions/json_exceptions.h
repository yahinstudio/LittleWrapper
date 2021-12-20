#pragma once
#include "exceptions.h"
#include "../json_obj.h"
#include <string>

class json_base_exception : public lw_base_exception
{
public:
	std::string reason;

	json_base_exception(json_obj* json, std::string reason) : lw_base_exception(reason)
	{
		this->reason = reason;

		if(json != nullptr)
			set_additional_message("\n==========Json Start==========\n" + json->to_string(true) + "\n==========Json End==========");
	}

	void set_additional_message(std::string message)
	{
		this->message = std::string(reason) + message;
	}
};

class json_parse_exception : public json_base_exception
{
public:
	std::string json_text;

	json_parse_exception(std::string json)
		: json_base_exception(nullptr, "json string failed to parse")
	{ 
		set_additional_message("\n==========Json Start==========\n" + json + "\n==========Json End==========");
	}
};

class json_get_object_exception : public json_base_exception
{
public:
	json_get_object_exception(json_obj* json, std::string key)
		: json_base_exception(json, "json get object failed. key: " + key)
	{ }
};

class json_get_item_exception : public json_base_exception
{
public:
	json_get_item_exception(json_obj* json, int index)
		: json_base_exception(json, "json get item failed. index: " + index)
	{ }
};

class json_type_cast_exception : public json_base_exception
{
public:
	json_type_cast_exception(json_obj* json, std::string typeFrom, std::string typeTo)
		: json_base_exception(json, "failed to cast type from '" + typeFrom + "' to '" + typeTo + "' of json")
	{ }
};

class json_not_array_exception : public json_base_exception
{
public:
	json_not_array_exception(json_obj* json)
		: json_base_exception(json, "json object was not a json-array")
	{ }
};

class json_not_object_exception : public json_base_exception
{
public:
	json_not_object_exception(json_obj* json)
		: json_base_exception(json, "json object was not a json-object")
	{ }
};

class json_array_index_out_of_bounds_exception : public json_base_exception
{
public:
	json_array_index_out_of_bounds_exception(json_obj* json, int indexOutOfBounds)
		: json_base_exception(json, "attempted to access a item with index (" + std::to_string(indexOutOfBounds) + ") out of bounds")
	{ }
};