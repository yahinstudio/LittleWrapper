#include "json_obj.h"
#include "exceptions/json_exceptions.h"
#include <string>

using namespace std;

json_obj::json_obj(std::string json)
{
	memory_alloced = true;
	root = cJSON_Parse(json.c_str());

	if (root == nullptr)
		throw json_parse_exception(json);
}

json_obj::json_obj(char* json) : json_obj(string(json))
{

}

json_obj::json_obj(bool isArray)
{
	memory_alloced = true;
	root = isArray ? cJSON_CreateArray() : cJSON_CreateObject();
}

json_obj::json_obj(cJSON* json)
{
	memory_alloced = false;
	root = json;
}

json_obj::~json_obj()
{
	if(memory_alloced)
		cJSON_Delete(root);
}

json_obj::json_obj(json_obj& other)
{
	memory_alloced = other.memory_alloced;

	if (memory_alloced)
	{
		root = cJSON_Duplicate(other.root, true);
	} else {
		root = other.root;
	}
}

json_obj::json_obj(json_obj&& other)
{
	memory_alloced = other.memory_alloced;

	if (memory_alloced)
	{
		root = other.root;
		other.root = nullptr;
	} else {
		root = other.root;
		other.root = nullptr;
	}
}

json_obj& json_obj::operator=(json_obj& other)
{
	if (memory_alloced)
	{
		if (other.memory_alloced)
		{
			root = cJSON_Duplicate(other.root, true);
		} else {
			cJSON_Delete(root);
			root = other.root;
			memory_alloced = false;
		}
	} else {
		if (other.memory_alloced)
		{
			root = new cJSON;
			root = cJSON_Duplicate(other.root, true);
			memory_alloced = true;
		} else {
			root = other.root;
		}
	}
	
	return *this;
}

json_obj& json_obj::operator=(json_obj&& other)
{
	if (memory_alloced)
	{
		cJSON_Delete(root);

		if (other.memory_alloced)
		{
			root = other.root;
			other.root = nullptr;
		} else {
			root = other.root;
			other.root = nullptr;
			memory_alloced = false;
		}
	} else {
		if (other.memory_alloced)
		{
			root = other.root;
			other.root = nullptr;
			memory_alloced = true;
		} else {
			root = other.root;
			other.root = nullptr;
		}
	}

	return *this;
}

json_obj json_obj::create_object()
{
	return json_obj(false);
}

json_obj json_obj::create_array()
{
	return json_obj(true);
}

cJSON* json_obj::get_object(string key)
{
	if (is_array())
		throw json_not_object_exception(this);

	cJSON* result = cJSON_GetObjectItem(root, key.c_str());

	if (result == nullptr)
		throw json_get_object_exception(this, key);

	return result;
}

cJSON* json_obj::get_item(int index)
{
	if (!is_array())
		throw json_not_array_exception(this);

	if (index >= get_array_size() || index < 0)
		throw json_array_index_out_of_bounds_exception(this, index);

	cJSON* result = cJSON_GetArrayItem(root, index);

	if (result == nullptr)
		throw json_get_item_exception(this, index);

	return result;
}

bool json_obj::has_object(std::string key)
{
	if (is_array())
		throw json_not_object_exception(this);

	return cJSON_HasObjectItem(root, key.c_str());
}

bool json_obj::is_array()
{
	return cJSON_IsArray(root);
}

int json_obj::get_array_size()
{
	if (!is_array())
		throw json_not_array_exception(this);

	return cJSON_GetArraySize(root);
}

double json_obj::get_object_double(std::string key)
{
	return check_type(get_object(key), cJSON_Number, "Double")->valuedouble;
}

int json_obj::get_object_int(std::string key)
{
	return check_type(get_object(key), cJSON_Number, "Int")->valueint;
}

string json_obj::get_object_string(std::string key)
{
	return check_type(get_object(key), cJSON_String, "String")->valuestring;
}

bool json_obj::get_object_bool(string key)
{
	cJSON* value = get_object(key);
	check_type(value, cJSON_True | cJSON_False, "Bool");
	return cJSON_IsTrue(value);
}

double json_obj::get_item_double(int index)
{
	return check_type(get_item(index), cJSON_Number, "Double")->valuedouble;
}

int json_obj::get_item_int(int index)
{
	return check_type(get_item(index), cJSON_Number, "Int")->valueint;
}

string json_obj::get_item_string(int index)
{
	return check_type(get_item(index), cJSON_String, "String")->valuestring;
}

bool json_obj::get_item_bool(int index)
{
	cJSON* value = get_item(index);
	check_type(value, cJSON_True | cJSON_False, "Bool");
	return cJSON_IsTrue(value);
}

cJSON* json_obj::check_type(cJSON* json, int cJsonTypesExpected, string typeNameComment)
{
	if ((json->type & cJsonTypesExpected) == 0)
	{
		string typeNameInString = !typeNameComment.empty() ? typeNameComment : cjson_type_to_string(cJsonTypesExpected);
		throw json_type_cast_exception(this, cjson_type_to_string(json->type), typeNameInString);
	}

	return json;
}

string json_obj::cjson_type_to_string(int cjson_type)
{
	switch (cjson_type)
	{
		case cJSON_NULL:
			return "Null";
		case cJSON_Number:
			return "Number";
		case cJSON_String:
			return "String";
		case cJSON_Array:
			return "Array";
		case cJSON_Object:
			return "Object";
		case cJSON_Raw:
			return "Raw";
		case cJSON_Invalid:
		default:
			return "Invalid";
	}
}

string json_obj::to_string(bool formatted)
{
	return formatted ? cJSON_Print(root) : cJSON_PrintUnformatted(root);
}

void json_obj::set_object(string key, bool value)
{
	if (has_object(key))
	{
		check_type(get_object(key), cJSON_False | cJSON_True, "Bool");						
		cJSON_DeleteItemFromObject(root, key.c_str());
	}

	cJSON_AddBoolToObject(root, key.c_str(), value);
}

void json_obj::set_object(string key, string value)
{
	if (has_object(key))
	{
		check_type(get_object(key), cJSON_String, "String");
		cJSON_SetValuestring(root, key.c_str());
	} else {
		cJSON_AddStringToObject(root, key.c_str(), value.c_str());
	}
}

void json_obj::set_object(std::string key, int value)
{
	set_object(key, (double) value);
}

void json_obj::set_object(string key, double value)
{
	if (has_object(key))
	{
		check_type(get_object(key), cJSON_Number, "Double/Int");
		cJSON_SetNumberValue(root, value);
	} else {
		cJSON_AddNumberToObject(root, key.c_str(), value);
	}
}

void json_obj::set_object(string key, json_obj value)
{
	if (has_object(key))
		cJSON_ReplaceItemInObject(root, key.c_str(), value.root);
	else
		cJSON_AddItemToObject(root, key.c_str(), value.root);

	if (value.memory_alloced)
		value.memory_alloced = false;
}

void json_obj::add_item_to_array(cJSON* item)
{
	if (!is_array())
		throw json_not_array_exception(this);

	cJSON_AddItemToArray(root, item);
}

void json_obj::add_item(bool item)
{
	add_item_to_array(cJSON_CreateBool(item));
}

void json_obj::add_item(std::string item)
{
	add_item_to_array(cJSON_CreateString(item.c_str()));
}

void json_obj::add_item(int item)
{
	add_item((double) item);
}

void json_obj::add_item(double item)
{
	add_item_to_array(cJSON_CreateNumber(item));
}

void json_obj::add_item(json_obj item)
{
	add_item_to_array(item.root);

	if (item.memory_alloced)
		item.memory_alloced = false;
}

cJSON* json_obj::operator->()
{
	return root;
}

json_obj json_obj::operator[](int index)
{
	return json_obj(get_item(index));
}

json_obj json_obj::operator[] (string key)
{
	return json_obj(get_object(key));
}