#pragma once
#include "iostream"

bool deflate_file(std::string fileIn, std::string fileOut);

bool inflate_file(std::string fileIn, std::string fileOut);

bool inflate_to_file(std::fstream& fileIn, size_t offset, size_t length, std::string fileOut);