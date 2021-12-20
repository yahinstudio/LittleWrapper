#pragma once
#include "libs/cJSON-1.7.14/cJSON.h"
#include <memory>
#include <fstream>
#include <string>
#include "json_obj.h"

extern const int split_block_len;
extern const uint8_t split_block[8];

class archiver
{
public:
	struct lw_options
	{
		bool check_hash = true;
		std::string exec = "";
		bool show_console = true;
	};

	const static std::ios_base::openmode read_only = std::ios_base::in | std::ios_base::binary;
	const static std::ios_base::openmode write_only = std::ios_base::out | std::ios_base::binary;
	const static std::ios_base::openmode read_write = std::ios_base::in | std::ios_base::in | std::ios_base::binary;

public:
	std::string file;
	std::shared_ptr<std::fstream> stream;

	archiver(std::string file, std::ios_base::openmode rw_mode);

	~archiver();

	archiver(archiver& other);

	archiver(archiver&& other);

	archiver& operator=(archiver& other);

	archiver& operator=(archiver&& other);

	/// <summary>
	/// 打包文件夹
	/// </summary>
	/// <param name="source_dir">要打包的目录</param>
	/// <param name="temp_dir">临时目录</param>
	/// <param name="optdata">app参数</param>
	static void lw_pack(std::string dest_file, std::string source_dir, std::string temp_dir, lw_options& options);

	static void lw_extract(std::string file_to_extract, std::string extract_dir, bool single_ins_protection, bool no_output);

	static void lw_detail(std::string file_to_read);

	static lw_options read_options(std::string file_to_read);

	static std::vector<std::string> read_directories(std::string file_to_read);

	static json_obj read_file_table(std::string file_to_read);

};