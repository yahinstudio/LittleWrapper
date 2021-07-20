#pragma once
#include "wrapper.h"
#include "iostream"
#include "cJSON-1.7.14/cJSON.h"

struct optiondata
{
	bool check_hash;
	std::string exec;
};

void lw_pack(std::string fileIn, std::string fileOut, std::string source_dir, std::string temp_compressed_dir, optiondata& optdata);

int lw_extract(std::string fileIn, std::string extract_dir, bool single_ins_protection, optiondata* out_optdata=nullptr);

void lw_detail(std::string fileIn, std::string export_file="");