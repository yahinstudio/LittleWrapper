#include <string>
#include <windows.h>
#include "utils/env_utils.h"
#include "utils/general_utils.h"
#include "little_wrapper_app.h"

using namespace std;

// Window Subsystem
int main(int argc, char** argv);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    // 转换参数
    int argc = 0;
    wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

    char** argv = new char* [argc];
    for (int i = 0; i < argc; i++)
        argv[i] = from_wchar_to_char(wargv[i]);

    return main(argc, argv);
}

// Console Subsystem
int main(int argc, char** argv)
{
    little_wrapper_app().main(argc, argv);
}
