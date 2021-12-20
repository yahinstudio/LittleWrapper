#pragma once
#include "../libs/cJSON-1.7.14/cJSON.h"
#include <iostream>
#include <vector>

struct file_struct_t
{
    bool is_file;
    bool read_only;
    bool hidden;

    uint64_t create;
    uint64_t access;
    uint64_t write;

    std::string name;
    uint64_t length;

    std::vector<file_struct_t> children;
};

/// <summary>
/// 生成文件夹结构
/// </summary>
/// <param name="path">要生成的路径</param>
/// <returns>文件夹结构信息</returns>
cJSON* dirstruct_to_jsonlist(std::vector<file_struct_t> file_struct);

/// <summary>
/// 生成文件夹架构
/// </summary>
/// <param name="path">要生成的路径</param>
/// <returns>文件列表</returns>
std::vector<file_struct_t> generate_dir_struct(std::string path);