#pragma once
#include "iostream"

bool deflate_file(std::string fileIn, std::string fileOut);

bool inflate_file(std::string fileIn, std::string fileOut);

bool inflate_to_file(std::fstream& fileIn, uint64_t offset, uint64_t length, std::string fileOut);