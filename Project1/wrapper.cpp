#include "wrapper.h"
#include "iostream"
#include "fstream"
#include "utils.h"
#include "cJSON-1.7.14/cJSON.h"
#include "project.h"
#include "debug.h"
#include "vector"
#include "dir_utils.h"
#include "direct.h" // mkdir
#include "archive.h"
#include "magic.h"
#include "md5/md5.h"

constexpr int split_data_len = 8;
constexpr uint8_t split_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

using namespace std;

// 提取出所有的文件夹
static cJSON* get_dirs(cJSON* file_arr, string relativePath = "")
{
    cJSON* result = cJSON_CreateArray();

    for (int i = 0; i < cJSON_GetArraySize(file_arr); i++)
    {
        cJSON* item = cJSON_GetArrayItem(file_arr, i);

        char* name = cJSON_GetObjectItem(item, "name")->valuestring;
        bool is_file = cJSON_IsTrue(cJSON_GetObjectItem(item, "is_file"));
        cJSON* children = cJSON_GetObjectItem(item, "children");

        if (!is_file)
        {
            //cout << "is_dir: " << relativePath << name << endl;

            cJSON* t = cJSON_CreateString((relativePath + name).c_str());
            cJSON_AddItemToArray(result, t);

            cJSON* r = get_dirs(children, relativePath + name + "/");
            for (int j = 0; j < cJSON_GetArraySize(r); j++)
                cJSON_AddItemToArray(result, cJSON_GetArrayItem(r, j));
        }
    }

    return result;
}

// 返回pair<relative_path, key>
static vector<pair<string, string>> compress_dir(cJSON* file_arr, std::string data_dir, string temp_compressed_dir, string relativePath = "")
{
    vector<pair<string, string>> result;

    for (int i = 0; i < cJSON_GetArraySize(file_arr); i++)
    {
        cJSON* item = cJSON_GetArrayItem(file_arr, i);

        char* name = cJSON_GetObjectItem(item, "name")->valuestring;
        bool is_file = cJSON_IsTrue(cJSON_GetObjectItem(item, "is_file"));

        if (is_file)
        {
            string relative_file = relativePath + name;
            // string key = string_replace(relative_file, "/", "___") + ".zlib";
            string key = get_string_md5(string_replace(relative_file, "/", "___") + ".zlib");
            result.push_back(pair(relative_file, key));

            string from = data_dir + "\\" + relative_file;
            string to = temp_compressed_dir + "\\" + key;

            printf("compressing: %s\n", relative_file.c_str());

            deflate_file(from, to);
        } else {
            cJSON* children = cJSON_GetObjectItem(item, "children");
            auto r = compress_dir(children, data_dir, temp_compressed_dir, relativePath + name + "/");
            for (auto it = r.begin(); it != r.end(); ++it)
                result.push_back(*it);
        }
    }

    return result;
}

static string generate_jumpdata(uint64_t offset, uint64_t len)
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "offset", offset);
    cJSON_AddNumberToObject(json, "len", len);

    char* printed = cJSON_PrintUnformatted(json);
    string result = printed;

    free(printed);
    cJSON_Delete(json);

    return result;
}

// 返回元数据的地址,长度
static pair<uint32_t, uint32_t> write_binaries(fstream& fout, string data_dir, string temp_compressed_dir, bool check_hash)
{
    auto metadata_addr = (uint32_t)fout.tellp();

    cJSON* metadata = cJSON_CreateObject();
    cJSON* arr = dir_struct_to_json_in_list(generate_dir_struct(data_dir));
    //cJSON_AddItemToObject(metadata, "struct", arr);
    cJSON_AddNumberToObject(metadata, "version", 1);
    cJSON_AddBoolToObject(metadata, "check_hash", check_hash);
    cJSON_AddItemToObject(metadata, "directories", get_dirs(arr));

    // 生成二进制数据地址表
    auto t = compress_dir(arr, data_dir, temp_compressed_dir);

    cJSON* address_table = cJSON_AddObjectToObject(metadata, "address_table");
    uint32_t addr_offset = 0;
    for (int i = 0; i < t.size(); i++)
    {
        auto item = t[i];
        auto compressed_file = temp_compressed_dir + "\\" + string_replace(item.second, "/", "\\");
        auto offset = addr_offset;
        auto len = get_file_length(compressed_file);
        auto hash = get_file_md5(compressed_file);
        auto raw_file = data_dir + "\\" + item.first;
        auto raw_length = get_file_length(raw_file);
        auto raw_hash = get_file_md5(raw_file);

        cJSON* record = cJSON_AddObjectToObject(address_table, item.second.c_str());
        cJSON_AddStringToObject(record, "raw_path", item.first.c_str());
        cJSON_AddNumberToObject(record, "raw_size", raw_length);
        cJSON_AddStringToObject(record, "raw_hash", raw_hash.c_str());
        cJSON_AddNumberToObject(record, "offset", offset);
        cJSON_AddNumberToObject(record, "len", len);
        cJSON_AddStringToObject(record, "hash", hash.c_str());

        addr_offset += len + split_data_len;
    }

    // 写出数据
    char* m2 = cJSON_Print(metadata);
    char* m3 = cJSON_PrintUnformatted(metadata);
    int metadata_len = strlen(m3);
    //cout << m2 << endl;
    printf("%s\n", m2);
    fout.write(m3, strlen(m3));
    fout.write((char*)split_data, split_data_len);
    free(m2);
    free(m3);
    cJSON_Delete(metadata);

    // 写binaries
    for (int i = 0; i < t.size(); i++)
    {
        auto item = t[i];
        auto compressed_file = temp_compressed_dir + "\\" + string_replace(item.second, "/", "\\");
        std::fstream fin(compressed_file, std::fstream::in | std::fstream::binary);

        error_check(!fin.fail(), "write_binaries: could not open the target-file to extract: " + compressed_file);

        int buf_len = 16 * 1024;
        uint8_t* buf = new uint8_t[buf_len];

        uint32_t readBytes = 0;
        do {
            fin.read((char*)buf, buf_len);
            error_check(!fin.bad(), "write_binaries: could not read the target-file: " + compressed_file);
            readBytes = fin.gcount();

            fout.write((char*)buf, readBytes);
            error_check(!fout.bad(), "write_binaries: could not write the binary");
        } while (readBytes > 0);

        fout.write((char*)split_data, split_data_len);
        error_check(!fout.bad(), "write_binaries: could not write the splite data to the binary");
        fin.close();
    }

    return pair(metadata_addr, metadata_len);
}

void attach_binaries(string fileIn, string fileOut, string data_dir, string temp_compressed_dir, bool check_hash)
{
    std::fstream fin(fileIn, std::fstream::in | std::fstream::binary);
    std::fstream fout(fileOut, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    error_check(!fin.fail(), "attach_binaries: could not open the in-file: " + fileIn);
    error_check(!fout.fail(), "attach_binaries: could not open the out-file: " + fileIn);

    // 获取magic位置
    auto magic_offset = get_magic_offset(fin, (uint8_t*)MAGIC_HEADER, MAGIC_LEN);
    if (magic_offset == 0)
    {
        printf("Can not locate the magic-header in the executable file.\n");
        return;
    }
    printf("magic at: %llx\n", magic_offset);

    // 复制源文件
    int buf_size = 4 * 1024;
    uint8_t* buf = new uint8_t[buf_size];

    int readBytes = 0;
    do {
        fin.read((char*)buf, buf_size);
        error_check(!fin.bad(), "attach_binaries: could not copy the binary: read");
        readBytes = fin.gcount();
        fout.write((char*)buf, readBytes);
        error_check(!fout.bad(), "attach_binaries: could not copy the binary: write");
    } while (readBytes > 0);

    // 准备临时目录用来存放压缩后的数据
    if (!file_exists(temp_compressed_dir))
        error_check(!_mkdir(temp_compressed_dir.c_str()), "attach_binaries: could not create the temp dir: " + temp_compressed_dir);
    printf("tempdir: %s\n", temp_compressed_dir.c_str());

    // 写metadata
    printf("generate directory strcuture for %s\n", data_dir.c_str());
    auto metadata_pair = write_binaries(fout, data_dir, temp_compressed_dir, check_hash);
    uint32_t metadata_addr = metadata_pair.first;
    uint32_t metatada_len = metadata_pair.second;

    // 写jumpdata
    uint64_t jumpdata_addr = magic_offset + MAGIC_LEN;
    string jumpdata = generate_jumpdata(metadata_addr, metatada_len);
    fout.seekp(jumpdata_addr);
    fout.write(jumpdata.c_str(), jumpdata.length());

    printf("wrote metadate at: %lx, magic at: %llx\n", metadata_addr, magic_offset);
    printf("wrote jumpdata data: 0x%llx, or %lld\n", jumpdata_addr, jumpdata_addr);

    fin.close();
    fout.close();
    delete[] buf;
}

// 1: 找不到jumpdata   2: 找不到metadata  3:数据损坏  4:无效jumpdata  5:无效metadata
int extract_binaries(string fileIn, string extract_dir)
{
    fstream fin(fileIn, fstream::in | fstream::binary);
    error_check(!fin.fail(), "extract_binaries: could not open the file to extract: " + fileIn);

    // 读取jumpdata
    uint64_t jumpdata_offset = get_magic_offset(fin, (uint8_t*)MAGIC_HEADER, MAGIC_LEN) + MAGIC_LEN;

    if (jumpdata_offset == 0)
    {
        printf("Can not locate the magic-header in the executable file.\n");
        return 1;
    }

    fin.seekg(jumpdata_offset);
    char* jumpdata = new char[PRESERVE_LEN - MAGIC_LEN];
    fin.read((char*)jumpdata, PRESERVE_LEN - MAGIC_LEN);
    error_check(!fin.bad(), "extract_binaries: could not read the jumpdata: " + fileIn);

    // 解析jumpdata
    cJSON* json = cJSON_Parse(jumpdata);
    if (json == nullptr)
    {
        return 4;
        delete[] jumpdata;
    }
    uint64_t metadata_addr = cJSON_GetObjectItem(json, "offset")->valueint;
    uint64_t metadata_len = cJSON_GetObjectItem(json, "len")->valueint;
    cJSON_Delete(json);
    delete[] jumpdata;

    if (metadata_addr == 0 || metadata_len == 0)
        return 2;

    // 读取元数据
    fin.clear();
    fin.seekg(metadata_addr);

    char* meta_buf = new char[metadata_len + 1];
    //memset(meta_buf, 0, metadata_len2);
    fin.read(meta_buf, metadata_len + 1);
    error_check(!fin.bad(), "extract_binaries: could not read the metadata: " + fileIn);

    printf("metadata offset: 0x%llx, len: %lld\n", metadata_addr, metadata_len);

    // 解析元数据
    cJSON* meta_json = cJSON_Parse(meta_buf);
    if (meta_json == nullptr)
    {
        return 5;
        delete[] meta_buf;
    }
    bool check_hash = cJSON_GetObjectItem(meta_json, "check_hash")->valueint != 0;
    cJSON* directories = cJSON_GetObjectItem(meta_json, "directories");
    cJSON* addr_table = cJSON_GetObjectItem(meta_json, "address_table");

    //char* pretty = cJSON_Print(meta_json);
    //printf("----------\n%s\n----------\n", pretty);
    delete[] meta_buf;
    //delete[] pretty;

    // 建立根目录
    string decompressed = extract_dir;
    if (!file_exists(decompressed))
        error_check(!_mkdir(decompressed.c_str()), "extract_binaries: could not create the extract-dir: " + decompressed);

    // 建立所有的文件夹(directories字段)
    for (int i = 0; i < cJSON_GetArraySize(directories); i++)
    {
        string dir = cJSON_GetArrayItem(directories, i)->valuestring;
        printf("mkdir: %s\n", dir.c_str());

        string cdir = decompressed + "\\" + string_replace(dir, "/", "\\");
        if (!file_exists(cdir))
            error_check(!_mkdir(cdir.c_str()), "extract_binaries: could not create the dir by the bounds: " + decompressed);
    }

    // 解压binaries
    fin.clear();
    fin.seekg(metadata_addr);
    uint64_t base_addr = metadata_addr + metadata_len + split_data_len; // 末尾有8个是分隔符（都是0）

    printf("\nBaseOffset: 0x%llx\n", base_addr);
    printf("CheckHash: %s\n", check_hash?"check":"no_check");

    for (int i = 0; i < cJSON_GetArraySize(addr_table); i++)
    {
        cJSON* item = cJSON_GetArrayItem(addr_table, i);
        string key = item->string;
        string raw_path = cJSON_GetObjectItem(item, "raw_path")->valuestring;
        uint64_t raw_size = cJSON_GetObjectItem(item, "raw_size")->valueint;
        string raw_hash = cJSON_GetObjectItem(item, "raw_hash")->valuestring;
        uint64_t offset = cJSON_GetObjectItem(item, "offset")->valueint;
        uint64_t length = cJSON_GetObjectItem(item, "len")->valueint;
        string hash = cJSON_GetObjectItem(item, "hash")->valuestring;

        string target_file = decompressed + "\\" + string_replace(raw_path, "/", "\\");
        uint64_t addr = base_addr + offset;

        // 校验
        if (check_hash)
        {
            fin.clear();
            fin.seekg(addr);
            string md5 = get_stream_md5(fin, length);
            if (md5 != hash)
            {
                printf("\nhash-check not passed, the file might be damaged!\nfile: %s\n", raw_path.c_str());
                printf("hash-inside: %s\nhash-calculated: %s\n", hash.c_str(), md5.c_str());
                printf("address: 0x%llx\nlength: %lld\n", addr, length);
                return 3;
            }
        }

        // 如果文件大小和校验一样，则跳过解压，重复使用
        if (file_exists(target_file) && 
            get_file_length(target_file) == raw_size && 
            get_file_md5(target_file) == raw_hash
            )
        {
            printf("reuse: %s\n", raw_path.c_str());
            continue;
        }

        printf("decompress: %s, offset: 0x%llx, len: %lld\n", raw_path.c_str(), addr, length);
        // 解压
        inflate_to_file(fin, addr, length, target_file);
    }

    fin.close();
    cJSON_Delete(meta_json);
    return 0;
}

void detail_binaries(string fileIn, string export_file)
{
    fstream fin(fileIn, fstream::in | fstream::binary);
    error_check(!fin.fail(), "detail_binaries: could not open the file to detail: " + fileIn);

    // 读取jumpdata
    uint64_t jumpdata_offset = get_magic_offset(fin, (uint8_t*)MAGIC_HEADER, MAGIC_LEN) + MAGIC_LEN;

    if (jumpdata_offset == 0)
    {
        printf("Can not locate the magic-header in the executable file.\n");
        return;
    }

    fin.seekg(jumpdata_offset);
    char* jumpdata = new char[PRESERVE_LEN - MAGIC_LEN];
    fin.read((char*)jumpdata, PRESERVE_LEN - MAGIC_LEN);
    error_check(!fin.bad(), "detail_binaries: could not read the jumpdata_offset: " + fileIn);

    printf("jumpdata at: 0x%llx\n%s\n", jumpdata_offset, jumpdata);

    // 解析jumpdata
    cJSON* json = cJSON_Parse(jumpdata);
    if (json == nullptr)
    {
        printf("the jumpdata is invaild\n");
        delete[] jumpdata;
        return;
    }
    uint64_t metadata_addr = cJSON_GetObjectItem(json, "offset")->valueint;
    uint64_t metadata_len = cJSON_GetObjectItem(json, "len")->valueint;
    cJSON_Delete(json);
    delete[] jumpdata;

    if (metadata_addr == 0 || metadata_len == 0)
    {
        printf("no detail could be shown.\nraw: off: %lld, len:%lld\n", metadata_addr, metadata_len);
        return;
    }

    // 读取元数据
    fin.clear();
    fin.seekg(metadata_addr);

    char* meta_buf = new char[metadata_len + 1];
    //memset(meta_buf, 0, metadata_len2);
    fin.read(meta_buf, metadata_len + 1);
    error_check(!fin.bad(), "detail_binaries: could not read the metadata: " + fileIn);

    printf("metadata offset: 0x%llx, len: %lld\n", metadata_addr, metadata_len);

    // 解析元数据
    cJSON* meta_json = cJSON_Parse(meta_buf);
    if (meta_json == nullptr)
    {
        printf("the metadata is invaild\n");
        delete[] meta_buf;
        return;
    }
    char* pretty = cJSON_Print(meta_json);

    if (export_file == "")
    {
        // 输出基本信息
        int meta_version = cJSON_GetObjectItem(meta_json, "version")->valueint;
        bool check_hash = cJSON_GetObjectItem(meta_json, "check_hash")->valueint != 0;
        printf("detail(version: %d, check_hash: %s):\n----------\n%s\n----------\n", meta_version, check_hash?"check":"no_check", pretty);

        // 计算基本偏移地址
        uint64_t base_addr = metadata_addr + metadata_len + split_data_len; // 末尾有8个是分隔符（都是0）
        printf("BaseAddr: %llx\n", base_addr);

        // 输出所有目录
        cJSON* directories = cJSON_GetObjectItem(meta_json, "directories");
        for (int i = 0; i < cJSON_GetArraySize(directories); i++)
        {
            string path = cJSON_GetArrayItem(directories, i)->valuestring;

            printf("Directory: %s\n", path.c_str());
        }

        // 输出所有文件
        cJSON* addr_table = cJSON_GetObjectItem(meta_json, "address_table");
        for (int i = 0; i < cJSON_GetArraySize(addr_table); i++)
        {
            cJSON* item = cJSON_GetArrayItem(addr_table, i);
            string key = item->string;
            string path = cJSON_GetObjectItem(item, "raw_path")->valuestring;
            uint64_t offset = cJSON_GetObjectItem(item, "offset")->valueint;
            uint64_t length = cJSON_GetObjectItem(item, "len")->valueint;
            string hash = cJSON_GetObjectItem(item, "hash")->valuestring;
            uint64_t addr = base_addr + offset;

            printf("File: %s, offset: 0x%llx, len: %lld, %s\n", path.c_str(), addr, length, hash.c_str());
        }

        printf("----------\n");
    } else {
        std::fstream fout(export_file, std::fstream::out | std::fstream::trunc);
        error_check(!fout.fail(), "detail_binaries: could not open the export-file: " + export_file);
        fout.write(pretty, strlen(pretty));
        error_check(!fout.fail(), "detail_binaries: could not write to the export-file: " + export_file);
        fout.close();
        printf("detail wrote: %s\n", export_file.c_str());
    }

    fin.close();
    cJSON_Delete(meta_json);
    delete[] pretty;
    delete[] meta_buf;
}