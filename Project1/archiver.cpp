#include "archiver.h"
#include "debug.h"
#include "magic.h"
#include <string>
#include <fstream>
#include "exceptions/exceptions.h"
#include "json_obj.h"
#include "utils/general_utils.h"
#include "utils/env_utils.h"
#include "utils/dir_utils.h"
#include "archive.h"
#include "single_ins.h"
#include <direct.h>

using namespace std;

const int split_block_len = 8;
const uint8_t split_block[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/// <summary>
/// 检查出所有的文件夹的路径
/// </summary>
/// <param name="file_array">本地文件结构信息</param>
/// <param name="relativePath">当前相对路径</param>
/// <returns>所有目录的相对路径</returns>
static json_obj get_dir_paths(json_obj file_array, string relativePath = "")
{
    json_obj result(true);

    for (size_t i = 0; i < file_array.get_array_size(); i++)
    {
        json_obj item = file_array[i];

        string name = item.get_object_string("name");
        bool is_file = item.get_object_bool("is_file");
        json_obj children = item["children"];

        if (!is_file)
        {
            result.add_item((relativePath + name));

            json_obj r = get_dir_paths(children, relativePath + name + "/");
            for (int j = 0; j < r.get_array_size(); j++)
                result.add_item(r[i]);
        }
    }

    return result;
}

/// <summary>
/// 执行压缩文件夹
/// </summary>
/// <param name="file_array">本地文件结构信息</param>
/// <param name="source_dir">源目录的根路径</param>
/// <param name="temp__dir">临时目录，用来存放压缩好的数据</param>
/// <param name="relativePath">当前相对路径</param>
/// <returns>返回所有压缩好的临时的.zlib文件的路径</returns>
static vector<string> compress_dir(json_obj file_array, std::string source_dir, string temp_dir, string relativePath = "")
{
    vector<string> result;

    for (int i = 0; i < file_array.get_array_size(); i++)
    {
        json_obj item = file_array[i];

        string name = item.get_object_string("name");
        bool is_file = item.get_object_bool("is_file");

        if (is_file)
        {
            string relative_file = relativePath + name;
            result.push_back(relative_file);

            printf("compressing: %s\n", relative_file.c_str());

            string source = source_dir + "\\" + relative_file;
            string dest = temp_dir + "\\" + get_string_md5(relative_file);
            deflate_file(source, dest);
        } else {
            json_obj children = item["children"];
            vector<string> r = compress_dir(children, source_dir, temp_dir, relativePath + name + "\\");
            for (auto it = r.begin(); it != r.end(); ++it)
                result.push_back(*it);
        }
    }

    return result;
}

/// <summary>
/// 生成jumpdata
/// </summary>
/// <param name="offset">jumpdata的偏移</param>
/// <param name="len">jumpdata的长度</param>
/// <returns>jumpdata</returns>
static string generate_jumpdata(size_t offset, size_t len)
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "offset", (double) offset);
    cJSON_AddNumberToObject(json, "len", (double) len);

    char* printed = cJSON_PrintUnformatted(json);
    string result = printed;

    free(printed);
    cJSON_Delete(json);

    return result;
}

archiver::archiver(std::string file, ios_base::openmode rw_mode)
{
    fstream* fs = new fstream(file, rw_mode);
    stream.reset(fs, [] (fstream* p) { p->close(); });
    error_check(!stream->fail(), "could not open the file: " + file);
}

archiver::~archiver()
{
    
}

archiver::archiver(archiver& other)
{
    file = other.file;
    stream = other.stream;
}

archiver::archiver(archiver&& other)
{
    file = other.file;
    stream = other.stream;
}

archiver& archiver::operator=(archiver& other)
{
    file = other.file;
    stream = other.stream;
    return *this;
}

archiver& archiver::operator=(archiver&& other)
{
    file = other.file;
    stream = other.stream;
    return *this;
}

archiver::lw_options archiver::get_options(json_obj metadata)
{
    bool check_hash = metadata.get_object_bool("check_hash");
    string exec = metadata.get_object_string("exec");
    bool show_console = metadata.get_object_bool("show_console");

    lw_options options;
    options.exec = exec;
    options.check_hash = check_hash;
    options.show_console = show_console;

    return options;
}

void archiver::locate_metadata(size_t* addr, size_t* len)
{
    // 解析jumpdata，以此快速定位metadata
    json_obj jumpdata(get_preserved_data());
    size_t metadata_addr = jumpdata.get_object_int("offset");
    size_t metadata_len = jumpdata.get_object_int("len");

    if (metadata_addr == 0 || metadata_len == 0)
        throw metadata_not_found_exception();

    if (addr != nullptr)
        *addr = metadata_addr;

    if (len != nullptr)
        *len = metadata_len;
}

json_obj archiver::read_metadata()
{
    // 解析jumpdata
    json_obj jumpdata(get_preserved_data());
    size_t metadata_addr = jumpdata.get_object_int("offset");
    size_t metadata_len = jumpdata.get_object_int("len");

    if (metadata_addr == 0 || metadata_len == 0)
        throw metadata_not_found_exception();

    // 读取metadata
    stream->clear();
    stream->seekg(metadata_addr);

    char* meta_buf = new char[metadata_len + 1];
    //memset(meta_buf, 0, metadata_len2);
    stream->read(meta_buf, metadata_len + 1);
    error_check(!stream->bad(), "could not read the metadata: " + file);

    printf("metadata offset: 0x%llx, len: %lld\n", metadata_addr, metadata_len);

    // 解析metadata
    json_obj metadata(meta_buf);
    delete[] meta_buf;
    return metadata;
}

void archiver::lw_pack(std::string source_dir, std::string temp_dir, lw_options& optdata)
{
    // 创建文件夹
    string parent_dir = get_dir_name(file);
    if (!file_exists(parent_dir))
        error_check(!_mkdir(parent_dir.c_str()), "pack_binaries: could not create the parent dir of the output-file: " + parent_dir);

    std::fstream fin(get_exe_path(), std::fstream::in | std::fstream::binary);
    std::fstream fout(file, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    error_check(!fin.fail(), "pack_binaries: could not open the in-file: " + get_exe_path());
    error_check(!fout.fail(), "pack_binaries: could not open the out-file: " + file);

    // 复制源文件
    int buf_size = 4 * 1024;
    uint8_t* buf = new uint8_t[buf_size];
    streamsize readBytes = 0;
    do {
        fin.read((char*) buf, buf_size);
        error_check(!fin.bad(), "pack_binaries: could not copy the binary: read");
        readBytes = fin.gcount();
        fout.write((char*) buf, readBytes);
        error_check(!fout.bad(), "pack_binaries: could not copy the binary: write");
    } while (readBytes > 0);
    delete[] buf;

    // 准备临时目录用来存放压缩后的数据
    if (!file_exists(temp_dir))
        error_check(!_mkdir(temp_dir.c_str()), "pack_binaries: could not create the temp dir: " + temp_dir);
    printf("tempdir: %s\n", temp_dir.c_str());

    // 打包文件
    printf("generate directory strcuture for %s\n", source_dir.c_str());
    auto metadata_pair = pack_binaries(source_dir, temp_dir, optdata);
    size_t metadata_addr = metadata_pair.first;
    size_t metatada_len = metadata_pair.second;

    // 定位jumpdata
    size_t jumpdata_addr = get_jumpdata_address(fin);
    if (jumpdata_addr == 0)
        throw jumpdata_not_found_exception();
    //printf("magic at: %llx\n", magic_offset);

    // 写jumpdata
    string jumpdata = generate_jumpdata(metadata_addr, metatada_len);
    fout.seekp(jumpdata_addr);
    fout.write(jumpdata.c_str(), jumpdata.length());

    printf("wrote metadate at: %llx, jumpdata at: %llx\n", metadata_addr, jumpdata_addr);

    fin.close();
    fout.close();
}

void archiver::lw_extract(std::string extract_dir, bool single_ins_protection, bool no_output)
{
    // 定位metadata
    size_t metadata_addr;
    size_t metadata_len;
    locate_metadata(&metadata_addr, &metadata_len);

    // 读取metadata
    json_obj metadata = read_metadata();
    lw_options opt = get_options(metadata);

    json_obj directories = metadata["directories"];
    json_obj addr_table = metadata["address_table"];

    // 单实例保护，当有多个实例存在时，后创建的实例不解压数据，直接运行就好，防止对先创建的运行中的实例造成文件破坏
    string write_protect_key = string("lw-sil-") + get_string_md5(extract_dir);
    printf("write protection key: %s\n", write_protect_key.c_str());
    bool no_second_ins = request_single_instance_lock(write_protect_key);

    if (no_second_ins)
    {
        // 建立解压输出目录
        string decompressed = extract_dir;
        if (!file_exists(decompressed))
            error_check(!_mkdir(decompressed.c_str()), "extract_binaries: could not create the extract-dir: " + decompressed);

        // 建立所有的文件夹(directories字段)
        for (int i = 0; i < directories.get_array_size(); i++)
        {
            string dir = directories.get_item_string(i);

            if (!no_output)
                printf("mkdir: %s\n", dir.c_str());

            string cdir = decompressed + "\\" + string_replace(dir, "/", "\\");
            if (!file_exists(cdir))
                error_check(!_mkdir(cdir.c_str()), "extract_binaries: could not create the dir by the bounds: " + decompressed);
        }

        // 解压所有打包好的文件
        stream->clear();
        stream->seekg(metadata_addr);
        size_t binaries_addr = metadata_addr + metadata_len + split_block_len; // 末尾有8个是分隔符（都是0）

        printf("\nbinaries_offset: 0x%llx\n", binaries_addr);
        printf("CheckHash: %s\n", opt.check_hash ? "check" : "no_check");

        for (int i = 0; i < addr_table.get_array_size(); i++)
        {
            json_obj item = addr_table[i];

            string key = item->string;
            string raw_path = item.get_object_string("raw_path");
            size_t raw_size = item.get_object_int("raw_size");
            string raw_hash = item.get_object_string("raw_hash");
            size_t offset = item.get_object_int("offset");
            size_t length = item.get_object_int("len");
            string hash = item.get_object_string("hash");

            string target_file = decompressed + "\\" + string_replace(raw_path, "/", "\\");
            size_t addr = binaries_addr + offset;

            // 校验
            if (opt.check_hash)
            {
                stream->clear();
                stream->seekg(addr);
                string md5 = get_stream_md5(*stream, length);
                if (md5 != hash)
                {
                    printf("\nhash-check not passed, the file might be damaged!\nfile: %s\n", raw_path.c_str());
                    printf("hash-inside: %s\nhash-calculated: %s\n", hash.c_str(), md5.c_str());
                    printf("address: 0x%llx\nlength: %lld\n", addr, length);
                    throw binaries_damaged_exception();
                }
            }

            // 如果文件大小和校验一样，则跳过解压，重复使用
            if (file_exists(target_file) && 
                get_file_length(target_file) == raw_size && 
                (!opt.check_hash || get_file_md5(target_file) == raw_hash)
            ) {
                if (!no_output)
                    printf("reuse: %s\n", raw_path.c_str());
                continue;
            }

            if (!no_output)
                printf("decompress: %s, offset: 0x%llx, len: %lld\n", raw_path.c_str(), addr, length);

            // 解压
            inflate_to_file(*stream, addr, length, target_file);
        }
    } else  {
        printf("muilt instance is detected.\n");
    }
}

void archiver::lw_detail(std::string export_file)
{
    // 读取metadata
    json_obj metadata = read_metadata();

    if (export_file == "")
    {
        // 获取选项数据
        lw_options opt = get_options(metadata);

        // 定位metadata
        size_t metadata_addr;
        size_t metadata_len;
        locate_metadata(&metadata_addr, &metadata_len);

        // 输出基本信息
        printf("detail:\n  check_hash: %d\n  exec: %s\n  show_console: %d\n----------\n%s\n----------\n", opt.check_hash, opt.exec.c_str(), opt.show_console, metadata.to_string());

        // 计算基本偏移地址
        size_t binaries_addr = metadata_addr + metadata_len + split_block_len; // 末尾有8个是分隔符（都是0）
        printf("binaries_address: 0x%llx\n", binaries_addr);

        // 输出所有目录
        json_obj directories = metadata["directories"];
        for (int i = 0; i < directories.get_array_size(); i++)
        {
            string path = directories.get_item_string(i);
            printf("Directory: %s\n", path.c_str());
        }

        // 输出所有文件
        json_obj addr_table = metadata["address_table"];
        for (int i = 0; i < addr_table.get_array_size(); i++)
        {
            json_obj item = addr_table[i];
            string key = item->string;
            string path = item.get_object_string("raw_path");
            size_t offset = item.get_object_int("offset");
            size_t length = item.get_object_int("len");
            string hash = item.get_object_string("hash");
            size_t addr = binaries_addr + offset;

            printf("File: %s, offset: 0x%llx, len: %lld, %s\n", path.c_str(), addr, length, hash.c_str());
        }

        printf("----------\n");
    } else {
        string pretty = metadata.to_string();
        std::fstream fout(export_file, std::fstream::out | std::fstream::trunc);
        error_check(!fout.fail(), "detail_binaries: could not open the export-file: " + export_file);
        fout.write(pretty.c_str(), pretty.length());
        error_check(!fout.fail(), "detail_binaries: could not write to the export-file: " + export_file);
        fout.close();
        printf("detail wrote to: %s\n", export_file.c_str());
    }
}

pair<size_t, size_t> archiver::pack_binaries(string source_dir, string temp_dir, lw_options& options)
{
    size_t metadata_addr = stream->tellp();

    json_obj file_array = json_obj(dirstruct_to_jsonlist(generate_dir_struct(source_dir))); // 文件目录结构数据

    // 准备元数据
    json_obj metadata;
    metadata.set_object("check_hash", options.check_hash);
    metadata.set_object("exec", options.exec);
    metadata.set_object("show_console", options.show_console);
    metadata.set_object("directories", get_dir_paths(file_array));

    // 压缩文件
    vector<string> t = compress_dir(file_array, source_dir, temp_dir);

    // 生成地址表
    json_obj address_table(false);
    size_t addr_offset = 0;
    for (int i = 0; i < t.size(); i++)
    {
        auto filename = t[i];
        auto hashed_filename = get_string_md5(filename);
        auto compressed_file = temp_dir + "\\" + hashed_filename;
        auto offset = addr_offset;
        auto len = get_file_length(compressed_file);
        auto hash = get_file_md5(compressed_file);
        auto raw_file = source_dir + "\\" + filename;
        auto raw_length = get_file_length(raw_file);
        auto raw_hash = get_file_md5(raw_file);

        json_obj record;
        record.set_object("raw_path", filename);
        record.set_object("raw_size", (int) raw_length);
        record.set_object("raw_hash", raw_hash);
        record.set_object("offset", (double) offset);
        record.set_object("len", (int) len);
        record.set_object("hash", hash);
        address_table.set_object(hashed_filename.c_str(), record);

        addr_offset += len + split_block_len;
    }
    metadata.set_object("address_table", address_table);

    // 1.写出整个metadata到文件
    string metadata_text = metadata.to_string(false);
    stream->write(metadata_text.c_str(), metadata_text.length());
    stream->write((char*) split_block, split_block_len);

    // 2.开始打包数据
    for (int i = 0; i < t.size(); i++)
    {
        auto filename = t[i];
        auto hashed_filename = get_string_md5(filename);
        auto compressed_file = temp_dir + "\\" + hashed_filename;
        std::fstream fin(compressed_file, std::fstream::in | std::fstream::binary);
        error_check(!fin.fail(), "write_binaries: could not open the target-file to extract: " + compressed_file);

        int buf_len = 16 * 1024;
        uint8_t* buf = new uint8_t[buf_len];

        streampos readBytes = 0;
        do {
            fin.read((char*) buf, buf_len);
            error_check(!fin.bad(), "write_binaries: could not read the target-file: " + compressed_file);
            readBytes = fin.gcount();

            stream->write((char*) buf, readBytes);
            error_check(!stream->bad(), "write_binaries: could not write the binary");
        } while (readBytes > 0);

        stream->write((char*) split_block, split_block_len);
        error_check(!stream->bad(), "write_binaries: could not write the splite data to the binary");
        fin.close();
    }

    return pair(metadata_addr, metadata_text.length());
}