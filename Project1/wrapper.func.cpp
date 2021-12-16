#include "wrapper.h"
#include "iostream"
#include "fstream"
#include "libs/cJSON-1.7.14/cJSON.h"
#include "utils.h"
#include "project.h"
#include "debug.h"
#include "vector"
#include "dir_utils.h"
#include "direct.h" // mkdir
#include "archive.h"
#include "magic.h"
#include "libs/md5/md5.h"
#include "single_ins.h"

using namespace std;

// 检查出所有的文件夹的路径
// file_array：本地文件结构信息
// relativePath：当前相对路径
// 返回：所有目录的相对路径
static cJSON* get_dir_paths(cJSON* file_array, string relativePath = "")
{
    cJSON* result = cJSON_CreateArray();

    for (int i = 0; i < cJSON_GetArraySize(file_array); i++)
    {
        cJSON* item = cJSON_GetArrayItem(file_array, i);

        char* name = cJSON_GetObjectItem(item, "name")->valuestring;
        bool is_file = cJSON_IsTrue(cJSON_GetObjectItem(item, "is_file"));
        cJSON* children = cJSON_GetObjectItem(item, "children");

        if (!is_file)
        {
            cJSON* t = cJSON_CreateString((relativePath + name).c_str());
            cJSON_AddItemToArray(result, t);

            cJSON* r = get_dir_paths(children, relativePath + name + "/");
            for (int j = 0; j < cJSON_GetArraySize(r); j++)
                cJSON_AddItemToArray(result, cJSON_GetArrayItem(r, j));
        }
    }

    return result;
}

// 压缩文件夹
// file_array：本地文件结构信息
// source_dir：源目录的根路径
// temp_dir：临时目录，用来存放压缩好的数据
// relativePath：当前相对路径
// 返回所有压缩好的临时的.zlib文件的路径
static vector<string> compress_dir(cJSON* file_array, std::string source_dir, string temp_compressed_dir, string relativePath = "")
{
    vector<string> result;

    for (int i = 0; i < cJSON_GetArraySize(file_array); i++)
    {
        cJSON* item = cJSON_GetArrayItem(file_array, i);

        char* name = cJSON_GetObjectItem(item, "name")->valuestring;
        bool is_file = cJSON_IsTrue(cJSON_GetObjectItem(item, "is_file"));

        if (is_file)
        {
            string relative_file = relativePath + name;
            result.push_back(relative_file);

            printf("compressing: %s\n", relative_file.c_str());

            string source = source_dir + "\\" + relative_file;
            string dest = temp_compressed_dir + "\\" + get_string_md5(relative_file);
            deflate_file(source, dest);
        }
        else {
            cJSON* children = cJSON_GetObjectItem(item, "children");
            auto r = compress_dir(children, source_dir, temp_compressed_dir, relativePath + name + "\\");
            for (auto it = r.begin(); it != r.end(); ++it)
                result.push_back(*it);
        }
    }

    return result;
}

// 生成jumpdata
static string generate_jumpdata(size_t offset, size_t len)
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "offset", (double)offset);
    cJSON_AddNumberToObject(json, "len", (double)len);

    char* printed = cJSON_PrintUnformatted(json);
    string result = printed;

    free(printed);
    cJSON_Delete(json);

    return result;
}

// 打包数据
// 返回pair<元数据的地址, 元数据的长度>
static pair<size_t, size_t> pack_binaries(fstream& fout, string source_dir, string temp_compressed_dir, optiondata& optdata)
{
    auto metadata_addr = fout.tellp();

    cJSON* file_array = dir_struct_to_json_in_list(generate_dir_struct(source_dir)); // 文件目录结构数据
    cJSON* metadata = cJSON_CreateObject();

    cJSON_AddBoolToObject(metadata, "check_hash", optdata.check_hash);
    cJSON_AddStringToObject(metadata, "exec", optdata.exec.c_str());
    cJSON_AddBoolToObject(metadata, "show_console", optdata.show_console);

    cJSON_AddItemToObject(metadata, "directories", get_dir_paths(file_array));
    cJSON* address_table = cJSON_AddObjectToObject(metadata, "address_table");

    // 压缩文件
    auto t = compress_dir(file_array, source_dir, temp_compressed_dir);

    // 生成地址表
    size_t addr_offset = 0;
    for (int i = 0; i < t.size(); i++)
    {
        auto filename = t[i];
        auto hashed_filename = get_string_md5(filename);
        auto compressed_file = temp_compressed_dir + "\\" + hashed_filename;
        auto offset = addr_offset;
        auto len = get_file_length(compressed_file);
        auto hash = get_file_md5(compressed_file);
        auto raw_file = source_dir + "\\" + filename;
        auto raw_length = get_file_length(raw_file);
        auto raw_hash = get_file_md5(raw_file);

        cJSON* record = cJSON_AddObjectToObject(address_table, hashed_filename.c_str());
        cJSON_AddStringToObject(record, "raw_path", filename.c_str());
        cJSON_AddNumberToObject(record, "raw_size", (double)raw_length);
        cJSON_AddStringToObject(record, "raw_hash", raw_hash.c_str());
        cJSON_AddNumberToObject(record, "offset", (double)offset);
        cJSON_AddNumberToObject(record, "len", (double)len);
        cJSON_AddStringToObject(record, "hash", hash.c_str());

        addr_offset += len + split_block_len;
    }

    // 1.写出整个metadata到文件
    char* metadata_text = cJSON_PrintUnformatted(metadata);
    size_t metadata_len = strlen(metadata_text);
    fout.write(metadata_text, strlen(metadata_text));
    fout.write((char*)split_block, split_block_len);
    free(metadata_text);
    cJSON_Delete(metadata);

    // 2.开始打包数据
    for (int i = 0; i < t.size(); i++)
    {
        auto filename = t[i];
        auto hashed_filename = get_string_md5(filename);
        auto compressed_file = temp_compressed_dir + "\\" + hashed_filename;
        std::fstream fin(compressed_file, std::fstream::in | std::fstream::binary);

        error_check(!fin.fail(), "write_binaries: could not open the target-file to extract: " + compressed_file);

        int buf_len = 16 * 1024;
        uint8_t* buf = new uint8_t[buf_len];

        streampos readBytes = 0;
        do {
            fin.read((char*)buf, buf_len);
            error_check(!fin.bad(), "write_binaries: could not read the target-file: " + compressed_file);
            readBytes = fin.gcount();

            fout.write((char*)buf, readBytes);
            error_check(!fout.bad(), "write_binaries: could not write the binary");
        } while (readBytes > 0);

        fout.write((char*)split_block, split_block_len);
        error_check(!fout.bad(), "write_binaries: could not write the splite data to the binary");
        fin.close();
    }

    return pair(metadata_addr, metadata_len);
}
