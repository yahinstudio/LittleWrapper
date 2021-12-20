#pragma once
#include <iostream>
#include <functional>

typedef std::function<void(size_t, size_t)> archive_on_processing;

bool deflate_file(std::string file_in, std::string file_out, archive_on_processing callback = nullptr, long chunk_size = 128 * 1024);

bool inflate_file(std::string file_in, std::string file_out, archive_on_processing callback = nullptr, long chunk_size = 128 * 1024);

bool inflate_to_file(std::fstream& file_in, size_t offset, size_t length, std::string file_out, archive_on_processing callback = nullptr, long chunk_size = 128 * 1024);