#pragma once
#include "fstream"

size_t get_preserved_data_address(std::fstream& fin, bool clear_state);

int get_preserved_data_len();

char* get_preserved_data();