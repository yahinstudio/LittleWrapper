#include "env_utils.h"
#include "general_utils.h"
#include <windows.h>
#include <direct.h>
#include <stdexcept>

using namespace std;

string get_exe_path()
{
    char temp[MAX_PATH];

    if (!GetModuleFileNameA(nullptr, temp, MAX_PATH))
        throw runtime_error("failed to get executable path");

    //printf("exe: %s\n", temp);

    return temp;
}

string get_current_work_dir()
{
    char cwd[MAX_PATH];
    _getcwd(cwd, MAX_PATH);
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

string get_temp_directory()
{
    char temp_path[MAX_PATH];
    int result = GetTempPathA(MAX_PATH, temp_path);
    if (result > MAX_PATH || result == 0)
        return "";
    return string(temp_path);
}