#include "dir_utils.h"
#include <iostream>
#include "project.h"
#include "windows.h"
#include "utils.h"
#include "env.h"
#include "libs/cJSON-1.7.14/cJSON.h"
#include "io.h" // _finddata_t, _findfirst, _A_SUBDIR, A_ARDONLY, A_HIDEEN, _findnext, _findclose

using namespace std;

// 生成文件夹结构
vector<file_info_t> generate_dir_struct(std::string path)
{
    vector<file_info_t> result;

    _finddata_t find;
    intptr_t handle = _findfirst((path + "\\*.*").c_str(), &find);

    if (handle != -1)
    {
        do {
            if (strcmp(find.name, ".") != 0 && strcmp(find.name, "..") != 0)
            {
                //cout <<"-"<< find.name << "-   " << (find.attrib & _A_SUBDIR) << endl;

                file_info_t finfo;
                finfo.is_file = !(find.attrib & _A_SUBDIR);
                finfo.read_only = find.attrib & _A_RDONLY;
                finfo.hidden = find.attrib & _A_HIDDEN;
                finfo.create = find.time_create;
                finfo.access = find.time_access;
                finfo.write = find.time_write; 
                finfo.name = find.name;
                finfo.length = find.size;
                if (!finfo.is_file)
                    finfo.children = generate_dir_struct(path + "\\" + finfo.name);
                
                result.push_back(finfo);
            }
        } while (_findnext(handle, &find) != -1);
        _findclose(handle);
    }

    return result;
}

// 将文件夹结构转换成cJSON*格式
cJSON* dir_struct_to_json_in_list(vector<file_info_t> file_struct)
{
    cJSON* arr = cJSON_CreateArray();
    
    for (auto it = file_struct.begin(); it != file_struct.end(); ++it)
    {
        cJSON* obj = cJSON_CreateObject();

        cJSON_AddBoolToObject(obj, "is_file", it->is_file);
        //cJSON_AddBoolToObject(obj, "read_only", it->read_only);
        //cJSON_AddBoolToObject(obj, "hidden", it->hidden);

        //cJSON_AddNumberToObject(obj, "create", it->create);
        //cJSON_AddNumberToObject(obj, "access", it->access);
        //cJSON_AddNumberToObject(obj, "write", it->write);

        cJSON_AddStringToObject(obj, "name", it->name.c_str());
        cJSON_AddNumberToObject(obj, "length", (double)it->length);

        cJSON* children = dir_struct_to_json_in_list(it->children);
        cJSON_AddItemToObject(obj, "children", children);
        
        cJSON_AddItemToArray(arr, obj);
    }

    return arr;
}

string get_temp_directory()
{
    char temp_path[PATH_MAX];
    int result = GetTempPathA(PATH_MAX, temp_path);
    if (result > MAX_PATH || result == 0)
        return "";
    return string(temp_path);
}