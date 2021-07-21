#pragma once
#include "iostream"
#include "cJSON-1.7.14/cJSON.h"

constexpr int split_block_len = 8;
constexpr uint8_t split_block[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

class lw_base_exception : public std::exception
{
public:
	/*std::string reason;

	lw_base_exception(std::string reason="aaaa") : exception()
	{
		this->reason = reason;
	}

	virtual const char* what() const
	{
		return this->reason.c_str();
	}*/
};

class jumpdata_not_found_exception : public lw_base_exception {};
class jumpdata_invaild_exception : public lw_base_exception {};
class metadata_not_found_exception : public lw_base_exception {};
class metadata_invaild_exception : public lw_base_exception {};
class binaries_damaged_exception : public lw_base_exception {};

struct optiondata
{
	bool check_hash = true;
	std::string exec;
	bool show_console = true;
};

optiondata get_optiondata(cJSON* metadata);

void lw_read_metadata(std::string fileIn, cJSON** out_metadata, size_t* out_addr = nullptr, size_t* out_len = nullptr);

void lw_pack(std::string fileIn, std::string fileOut, std::string source_dir, std::string temp_compressed_dir, optiondata& optdata);

void lw_extract(std::string fileIn, std::string extract_dir, bool single_ins_protection);

void lw_detail(std::string fileIn, std::string export_file = "");