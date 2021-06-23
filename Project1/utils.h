#pragma once
#include <iostream>
#include "cJSON-1.7.14/cJSON.h"

wchar_t* from_char_to_wchar(char* str);

char* from_wchar_to_char(wchar_t* wchar);

bool to_utf8(char* output_str, const wchar_t* wstr, size_t wstr_max_len);

bool from_utf8(wchar_t* output_wstr, const char* str, size_t wstr_max_len);

std::string string_replace(std::string str, std::string oldstr, std::string newstr);

long get_file_length(std::string file);

std::string get_file_md5(std::string file);

std::string get_stream_md5(std::fstream& stream, uint64_t len);

std::string get_string_md5(std::string str);

std::string get_filename(std::string path);

std::string get_dir_name(std::string file);

bool check_path(std::string path);

bool file_exists(std::string path);

bool file_exists2(std::string path);

bool string_starts_with(std::string str, std::string starts_with);

bool is_file_a_dir(std::string path);

void remove_dir(std::string path);

void show_dialog(std::string title, std::string text);

void set_window_visible(bool visible);