#pragma once
#include "cJSON-1.7.14/cJSON.h"
#include <iostream>
#include "vector"

struct file_info_t
{
    bool is_file;
    bool read_only;
    bool hidden;

    uint64_t create;
    uint64_t access;
    uint64_t write;

    std::string name;
    uint64_t length;

    std::vector<file_info_t> children;
};

cJSON* dir_struct_to_json_in_list(std::vector<file_info_t> file_struct);

std::vector<file_info_t> generate_dir_struct(std::string path);

std::string get_temp_directory();