#pragma once
#include "libs/cJSON-1.7.14/cJSON.h"
#include "exceptions/exceptions.h"
#include <string>

class json_obj
{
private:
	bool memory_alloced;
	cJSON* root;

	cJSON* get_object(std::string key);

	cJSON* get_item(int index);

	void add_item_to_array(cJSON* item);

	cJSON* check_type(cJSON* json, int cJsonTypesExpected, std::string typeNameComment = "");

	std::string cjson_type_to_string(int cjson_type);

public:
	json_obj(std::string json);

	json_obj(bool isArray = false);

	json_obj(cJSON* json);

	~json_obj();

	json_obj(json_obj& other);

	json_obj(json_obj&& other);

	json_obj& operator=(json_obj& other);

	json_obj& operator=(json_obj&& other);

	bool has_object(std::string key);

	bool is_array();

	int get_array_size();

	double get_object_double(std::string key);

	int get_object_int(std::string key);

	std::string get_object_string(std::string key);

	bool get_object_bool(std::string key);

	double get_item_double(int index);

	int get_item_int(int index);

	std::string get_item_string(int index);

	bool get_item_bool(int index);

	std::string to_string(bool formatted = true);

	void set_object(std::string key, bool value);

	void set_object(std::string key, std::string value);

	void set_object(std::string key, int value);

	void set_object(std::string key, double value);

	void set_object(std::string key, json_obj value);

	void add_item(bool item);

	void add_item(std::string item);

	void add_item(int item);

	void add_item(double item);

	void add_item(json_obj item);
	
	cJSON* operator-> ();

	json_obj operator[] (int index);

	json_obj operator[] (std::string key);
};
