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
#include "pe_resource.h"

using namespace std;

const int split_block_len = 8;
const uint8_t split_block[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/// <summary>
/// 检查出所有的文件夹的路径
/// </summary>
/// <param name="file_array">本地文件结构信息</param>
/// <param name="relativePath">当前相对路径</param>
/// <returns>所有目录的相对路径</returns>
static vector<string> get_all_dirs(vector<file_struct> file_array, string relativePath = "")
{
    vector<string> result;

    for (size_t i = 0; i < file_array.size(); i++)
    {
        file_struct file = file_array[i];

        if (!file.is_file)
        {
            result.emplace_back(relativePath + file.name);

            vector<string> r = get_all_dirs(file.children, relativePath + file.name + "/");
            for (int j = 0; j < r.size(); j++)
                result.emplace_back(r[j]);
        }
    }

    return result;
}

/// <summary>
/// vector{string}转json_obj列表
/// </summary>
/// <param name="list"></param>
/// <returns></returns>
static json_obj vector2json(vector<string> list)
{
    json_obj result = json_obj::create_array();

    for (int i = 0; i < list.size(); i++)
        result.add_item(list[i]);

    return result;
}

/// <summary>
/// 生成文件夹结构
/// </summary>
/// <param name="path">要生成的路径</param>
/// <returns>文件夹结构信息</returns>
static json_obj files2json(vector<file_struct> files)
{
    json_obj list = json_obj::create_array();

    for (int i = 0; i < files.size(); i++)
    {
        file_struct file = files[i];
        json_obj entry = json_obj::create_object();
        entry.set_object("is_file", file.is_file);
        entry.set_object("name", file.name);
        entry.set_object("length", (int) file.length);
        entry.set_object("children", files2json(file.children));
        list.add_item(entry);
    }

    return list;
}

/// <summary>
/// 执行文件夹打包操作
/// </summary>
/// <param name="files">文件结构信息</param>
/// <param name="record_list">打包相关的文件数据写出对象</param>
/// <param name="source_dir">源目录的根路径</param>
/// <param name="temp_dir">临时目录，用来存放压缩好的数据</param>
/// <param name="relative_path">当前相对路径</param>
static void pack_with_dir(vector<file_struct> files, json_obj& record_list, std::string source_dir, string temp_dir, string relative_path = "")
{
    for (int i = 0; i < files.size(); i++)
    {
        file_struct file = files[i];
        string relative_file = relative_path + file.name;

        if (file.is_file)
        {
            string source_file = source_dir + "\\" + relative_file;
            string hash = get_file_md5(source_file);
            string compressed = temp_dir + "\\" + hash;

            // 进度指示
            int last_percent = 0;
            int remains_count = 0;
            archive_on_processing cb = [&last_percent, &remains_count](size_t processed, size_t total) {
                size_t percent = processed * 100 / total;
                if (percent != last_percent)
                {
                    //printf(" (%d -> %d) ", last_percent, percent);

                    int delta = percent - last_percent;
                    for (int i = 0; i < delta / 10; i++)
                        printf(".");

                    remains_count += delta % 10;
                    if (remains_count >= 10)
                    {
                        for (int i = 0; i < remains_count / 10; i++)
                            printf(".");
                        remains_count %= 10;
                    }

                    last_percent = percent;
                }
            };

            // 执行压缩
            printf("compressing: %s ", relative_file.c_str());
            deflate_file(source_file, compressed, cb);
            printf("\n");

            // 添加文件数据
            json_obj entry = json_obj::create_object();
            entry.set_object("raw_path", relative_file);
            entry.set_object("raw_len", (int) file.length);
            entry.set_object("raw_hash", hash);
            entry.set_object("len", (int) get_file_length(compressed));
            entry.set_object("hash", get_file_md5(compressed));
            record_list.add_item(entry);
        } else {
            pack_with_dir(file.children, record_list, source_dir, temp_dir, relative_file + "\\");
        }
    }
}

/// <summary>
/// 清理多余文件和目录
/// </summary>
/// <param name="base_path">要清理的目录路径</param>
/// <param name="relative_path">相对目录</param>
/// <param name="local_files">要拿来对比的本地文件</param>
/// <param name="template_files">不要清理的文件</param>
/// <param name="template_directories">不要清理的目录</param>
static void clean_folder(string base_path, string relative_path, vector<file_struct> local_files, vector<string>& template_files, vector<string>& template_directories)
{
    for (int i = 0; i < local_files.size(); i++)
    {
        file_struct entry = local_files[i];
        string relative_entry = relative_path + entry.name;

        if (entry.is_file)
        {
            bool found = false;
            for (int i = 0; i < template_files.size(); i++)
            {
                if (relative_entry == template_files[i])
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                remove_file_or_dir(base_path + "\\" + relative_entry);
        } else {
            clean_folder(
                base_path,
                relative_path + (relative_path.empty() ? "" : "\\") + entry.name,
                entry.children,
                template_files, 
                template_directories
            );

            bool found = false;
            for (int i = 0; i < template_directories.size(); i++)
            {
                if (template_directories[i] == relative_entry)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                remove_file_or_dir(base_path + "\\" + relative_entry);
        }
    }
}

archiver::archiver(std::string file, ios_base::openmode rw_mode)
{
    this->file = file;

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

void archiver::lw_pack(std::string dest_file, std::string source_dir, std::string temp_dir, lw_options& options)
{
    string source_file = get_exe_path();

    // 创建文件夹
    string parent_dir = get_dir_name(dest_file);
    if (!file_exists(parent_dir))
        error_check(!_mkdir(parent_dir.c_str()), "failed to create the parent dir of the output-file: " + parent_dir);

    // 准备临时目录用来存放压缩后的数据
    if (!file_exists(temp_dir))
        error_check(!_mkdir(temp_dir.c_str()), "failed to create the temp dir: " + temp_dir);
    else
        clear_dir(temp_dir);

    // 0.读取目录结构
    vector<file_struct> strcut = cal_dir_struct(source_dir);

    if (strcut.size() == 0)
        throw failed_to_pack_exception();

    // 1.复制EXE文件模板
    std::fstream fs_source(source_file, std::fstream::in | std::fstream::binary);
    error_check(!fs_source.fail(), "failed to open the source-file: " + source_file);
    std::fstream fs_dest(dest_file, std::fstream::out | std::fstream::binary | std::fstream::trunc);
    error_check(!fs_dest.fail(), "failed to open the dest-file: " + dest_file);
    {
        int buf_size = 64 * 1024;
        uint8_t* buf = new uint8_t[buf_size];
        streamsize readBytes = 0;
        do  {
            fs_source.read((char*) buf, buf_size);
            error_check(!fs_source.bad(), "failed to copy the binary: read");
            readBytes = fs_source.gcount();
            fs_dest.write((char*) buf, readBytes);
            error_check(!fs_dest.bad(), "failed to copy the binary: write");
        } while (readBytes > 0);
        delete[] buf;
    }
    fs_source.close();
    fs_dest.close();

    // 2.写出PeResource数据
    // 一.选项数据
    json_obj optiondata = json_obj::create_object();
    optiondata.set_object("check_hash", options.check_hash);
    optiondata.set_object("exec", options.exec);
    optiondata.set_object("show_console", options.show_console);

    // 一.目录数据
    json_obj directories = vector2json(get_all_dirs(strcut));

    // 三.文件数据
    json_obj filetable = json_obj::create_array();
    // 压缩文件
    pack_with_dir(strcut, filetable, source_dir, temp_dir);

    // 更新地址表，添加offset属性，然后
    size_t addr_offset = 0;
    for (int i = 0; i < filetable.get_array_size(); i++)
    {
        filetable[i].set_object("relative_offset", (int) addr_offset);
        addr_offset += filetable[i].get_object_int("len") + split_block_len;
    }

    // 写出元数据
    string text_optiondata = optiondata.to_string(true);
    string text_directories = directories.to_string(true);
    string text_filetable = filetable.to_string(true);

    pe_resource_writer res_writer(dest_file);
    res_writer.update_resouce("little_wrapper", "options", (LPVOID) text_optiondata.c_str(), text_optiondata.length() + 1);
    res_writer.update_resouce("little_wrapper", "directories", (LPVOID) text_directories.c_str(), text_directories.length() + 1);
    res_writer.update_resouce("little_wrapper", "filetable", (LPVOID) text_filetable.c_str(), text_filetable.length() + 1);
    res_writer.close();

    // 3.写出二进制偏移数据
    size_t dest_file_size = get_file_length(dest_file);

    std::fstream fs_dest2(dest_file, std::fstream::in | std::fstream::out | std::fstream::binary);
    error_check(!fs_dest2.fail(), "failed to open the dest-file: " + dest_file);

    // 写二进制偏移数据
    size_t addr = get_preserved_data_address(fs_dest2, true);
    fs_dest2.seekg(addr);

    json_obj offset_data = json_obj::create_object();
    offset_data.set_object("offset", (int) dest_file_size);
    string offset_data_text = offset_data.to_string(false);

    fs_dest2.write((char*) offset_data_text.c_str(), offset_data_text.length() + 1);
    error_check(!fs_dest2.bad(), "failed to write offset-data to pe-file for binaries when packing: " + dest_file);

    fs_dest2.seekg(0, ios_base::end);

    // 4.写出二进制数据
    for (int i = 0; i < filetable.get_array_size(); i++)
    {
        json_obj entry = filetable[i];
        string compressed_file = temp_dir + "\\" + entry.get_object_string("raw_hash");

        std::fstream fin(compressed_file, std::fstream::in | std::fstream::binary);
        error_check(!fin.fail(), "failed to open the binary-file when packing: " + compressed_file);

        int buf_len = 64 * 1024;
        uint8_t* buf = new uint8_t[buf_len];
        streampos bytes_read = 0;
        do {
            fin.read((char*)buf, buf_len);
            error_check(!fin.bad(), "failed to read the binary-file when packing: " + compressed_file);
            bytes_read = fin.gcount();

            fs_dest2.write((char*) buf, bytes_read);
            error_check(!fs_dest2.bad(), "failed to write to pe-file when packing: " + dest_file);
        } while (bytes_read > 0);
        fs_dest2.write((char*) split_block, split_block_len);
        error_check(!fs_dest2.bad(), "failed to write the splite data to pe-file when packing: " + dest_file);

        fin.close();
    }
    fs_dest2.close();
}

void archiver::lw_extract(std::string file_to_extract, std::string extract_dir, bool single_ins_protection, bool no_output)
{
    json_obj file_table = archiver::read_file_table(file_to_extract);
    archiver::lw_options options = archiver::read_options(file_to_extract);
    vector<string> directories = archiver::read_directories(file_to_extract);

    // 单实例保护，当有多个实例存在时，后创建的实例不解压数据，直接运行就好，防止对先创建的运行中的实例造成文件破坏
    string write_protect_key = string("lw-sil-") + get_string_md5(extract_dir);
    printf("write protection key: %s\n", write_protect_key.c_str());
    bool no_second_ins = request_single_instance_lock(write_protect_key);

    if (no_second_ins)
    {
        // 建立解压输出目录
        if (!file_exists(extract_dir))
            error_check(!_mkdir(extract_dir.c_str()), "failed to create the dir to extract: " + extract_dir);

        // 清理多余的文件
        vector<string> _files; // 不要清理的文件的路径
        for (int i = 0; i < file_table.get_array_size(); i++)
            _files.emplace_back(string_replace(file_table[i].get_object_string("raw_path"), "/", "\\"));
        vector<string> _dirs; // 不要清理的目录的路径
        for (int i = 0; i < directories.size(); i++)
            _files.emplace_back(string_replace(directories[i], "/", "\\"));
        vector<file_struct> local_files = cal_dir_struct(extract_dir); // 本地现有文件状况

        clean_folder(extract_dir, "", local_files, _files, _files);

        // 建立所有的文件夹(directories字段)
        for (int i = 0; i < directories.size(); i++)
        {
            string dir = directories[i];

            string cdir = extract_dir + "\\" + string_replace(dir, "/", "\\");
            if (!file_exists(cdir))
            {
                if (!no_output)
                    printf("mkdir: %s\n", dir.c_str());

                error_check(!_mkdir(cdir.c_str()), "failed to create the dir for binaries: " + cdir);
            }
        }

        // 获取二进制数据偏移
        json_obj pdata(string(get_preserved_data(), get_preserved_data_len()));
        size_t base_offset = pdata.get_object_int("offset");

        // 解压所有打包好的文件
        printf("\nbase_offset: 0x%llx      CheckHash: %s\n", base_offset, options.check_hash ? "check" : "no_check");

        std::fstream fs(file_to_extract, std::fstream::in | std::fstream::binary);
        error_check(!fs.fail(), "failed to open the file: " + file_to_extract);

        for (int i = 0; i < file_table.get_array_size(); i++)
        {
            json_obj item = file_table[i];

            string raw_path = item.get_object_string("raw_path");
            size_t raw_len = item.get_object_int("raw_len");
            string raw_hash = item.get_object_string("raw_hash");
            size_t relative_offset = item.get_object_int("relative_offset");
            size_t length = item.get_object_int("len");
            string hash = item.get_object_string("hash");

            string target_file = extract_dir + "\\" + string_replace(raw_path, "/", "\\");
            size_t bin_addr = base_offset + relative_offset;

            // 校验
            if (options.check_hash)
            {
                fs.clear();
                fs.seekg(bin_addr);
                string md5 = get_stream_md5(fs, length);
                if (md5 != hash)
                {
                    printf("\nhash-check not passed, the file might be damaged!\nfile: %s\n", raw_path.c_str());
                    printf("hash-inside: %s\nhash-calculated: %s\n", hash.c_str(), md5.c_str());
                    printf("address: 0x%llx\nlength: %lld\n", bin_addr, length);
                    throw binaries_damaged_exception();
                }
            }

            // 如果文件大小和校验一样，则跳过解压，重复使用
            if (file_exists(target_file) && 
                get_file_length(target_file) == raw_len &&  
                (!options.check_hash || get_file_md5(target_file) == raw_hash)
            ) {
                if (!no_output)
                    printf("reuse: %s\n", raw_path.c_str());
                continue;
            }

            if (!no_output)
                printf("decompress: %s, offset: 0x%llx, len: %lld, raw_len: %lld\n", raw_path.c_str(), bin_addr, length, raw_len);

            // 解压
            inflate_to_file(fs, bin_addr, length, target_file);
        }

        fs.close();
    } else  {
        printf("muilt instance is detected.\n");
    }
}

void archiver::lw_detail(std::string file_to_read)
{
    json_obj file_table = archiver::read_file_table(file_to_read);
    archiver::lw_options options = archiver::read_options(file_to_read);
    vector<string> directories = archiver::read_directories(file_to_read);

    // 输出options
    printf("---------------options----------------\n");
    printf("options:\n  check_hash: %d\n  exec: %s\n  show_console: %d\n",
        options.check_hash,
        options.exec.c_str(),
        options.show_console
    );

    // 输出基本信息
    printf("---------------directories----------------\n");
    for (int i = 0; i < directories.size(); i++)
        printf("Directory: %s\n", directories[i].c_str());

    // 计算基本偏移地址
    printf("---------------file-table----------------\n");
    std::fstream fs(get_exe_path(), std::fstream::in | std::fstream::binary);
    error_check(!fs.fail(), "failed to open the file: " + get_exe_path());
    size_t base_address = get_preserved_data_address(fs, true);
    fs.close();
    printf("base_address: 0x%llx\n", base_address);

    for (int i = 0; i < file_table.get_array_size(); i++)
    {
        json_obj item = file_table[i];
        string path = item.get_object_string("raw_path");
        size_t relative_offset = item.get_object_int("relative_offset");
        size_t length = item.get_object_int("len");
        string hash = item.get_object_string("hash");
        size_t addr = base_address + relative_offset;

        printf("File: %s, file_addr: 0x%llx, len: %lld, %s\n", path.c_str(), addr, length, hash.c_str());
    }

    printf("----------\n");
}

archiver::lw_options archiver::read_options(std::string file_to_read)
{
    pe_resource_reader res_reader(file_to_read);
    auto res = res_reader.open_resource("little_wrapper", "options");
    json_obj options_json(string((char*) res.data, res.size));
    res_reader.close();

    lw_options options;
    options.check_hash = options_json.get_object_bool("check_hash");
    options.exec = options_json.get_object_string("exec");
    options.show_console = options_json.get_object_bool("show_console");

    return options;
}

vector<string> archiver::read_directories(std::string file_to_read)
{
    pe_resource_reader res_reader(file_to_read);
    auto res = res_reader.open_resource("little_wrapper", "directories");
    json_obj directories_json(string((char*) res.data, res.size));
    res_reader.close();
    vector<string> directories;

    for (int i = 0; i < directories_json.get_array_size(); i++)
        directories.emplace_back(directories_json.get_item_string(i));

    return directories;
}

json_obj archiver::read_file_table(std::string file_to_read)
{
    pe_resource_reader res_reader(file_to_read);
    auto res = res_reader.open_resource("little_wrapper", "filetable");
    json_obj json = json_obj(string((char*) res.data, res.size));
    res_reader.close();
    return json;
}