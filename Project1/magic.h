#pragma once
#include "fstream"

uint64_t get_magic_offset(std::fstream& fs, uint8_t* magic, int magic_len);