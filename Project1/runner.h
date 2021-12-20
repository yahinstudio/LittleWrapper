#pragma once
#include <string>

int start_subprocess(std::string file, std::string temp_dir, std::string additional_argument, bool show_console_set, bool show_console, bool no_output);