#pragma once
#include <string>
#include <vector>

std::string get_exe_path();

std::string get_exe_filename(bool no_suffix);

std::string get_current_work_dir();

void changed_current_work_dir(std::string newdir);

std::vector<std::string> get_environments();

std::string get_temp_directory();