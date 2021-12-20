#pragma once
#include "fstream"

//size_t get_magic_offset(std::fstream& fin);

size_t get_jumpdata_address(std::fstream& fin);

std::string get_preserved_data();