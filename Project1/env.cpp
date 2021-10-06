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
    _getcwd(cwd, PATH_MAX);
    return string(cwd);
}

void changed_current_work_dir(string newdir)
{
    _chdir(newdir.c_str());
}

vector<string> get_environments()
{
    vector<string> ret;
    LPWCH env = GetEnvironmentStrings();
    int offset = 0;
    while (true)
    {
        LPWCH ch = env + offset;
        int len = wcslen(ch);
        if (len == 0)
            break;
        char* str = from_wchar_to_char(ch);
        ret.push_back(string(str));
        delete str;
        offset += len + 1;
    }
    FreeEnvironmentStrings(env);
    return ret;
}