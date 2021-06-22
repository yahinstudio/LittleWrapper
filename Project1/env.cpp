#include "env.h"
#include "utils.h"
#include <iostream>
#include <windows.h> // GetModuleFileNameW()
#include <direct.h> // getcwd(), chdir()

using namespace std;

string get_exe_path()
{
    char temp[PATH_MAX];

    if (!GetModuleFileNameA(nullptr, temp, PATH_MAX))
        throw runtime_error("failed to get executable path");

    //printf("exe: %s\n", temp);

    return temp;
}

string get_current_work_dir()
{
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    return string(cwd);
}

void changed_current_work_dir(string newdir)
{
    chdir(newdir.c_str());
}