#pragma once
#include "wrapper.h"
#include "iostream"
#include "cJSON-1.7.14/cJSON.h"

void pack_binaries(std::string fileIn, std::string fileOut, std::string source_dir, std::string temp_compressed_dir, bool check_hash, std::string exec);

int extract_binaries(std::string fileIn, std::string extract_dir, std::string* exec=nullptr);

void detail_binaries(std::string fileIn, std::string export_file="");