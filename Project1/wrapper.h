#pragma once
#include "wrapper.h"
#include "iostream"
#include "cJSON-1.7.14/cJSON.h"

void attach_binaries(std::string fileIn, std::string fileOut, std::string data_dir, std::string temp_compressed_dir, bool check_hash=true);

int extract_binaries(std::string fileIn, std::string extract_dir);

void detail_binaries(std::string fileIn, std::string export_file="");