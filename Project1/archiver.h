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

private:
	/// <summary>
	/// 打包数据
	/// </summary>
	/// <param name="source_dir">要打包的目录</param>
	/// <param name="temp_dir">临时目录</param>
	/// <param name="options">app参数</param>
	/// <returns>返回pair(元数据的地址, 元数据的长度)</returns>
	std::pair<size_t, size_t> pack_binaries(std::string source_dir, std::string temp_dir, lw_options& options);

	void locate_metadata(size_t* out_addr, size_t* out_len);

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
	void lw_pack(std::string source_dir, std::string temp_dir, lw_options& optdata);

	// 解压数据
	void lw_extract(std::string extract_dir, bool single_ins_protection, bool no_output);

	void lw_detail(std::string export_file = "");

	/// <summary>
	/// 读取metadata
	/// </summary>
	json_obj read_metadata();

	/// <summary>
	/// 从metadata里获取optiondata
	/// </summary>
	lw_options get_options(json_obj metadata);
};