#pragma once
#include "string"
#include "project.h"

std::string get_exe_path();

std::string get_current_work_dir();

void changed_current_work_dir(std::string newdir);