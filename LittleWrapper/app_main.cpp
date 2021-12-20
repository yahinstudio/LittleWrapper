#include <string>
#include <windows.h>
#include "utils/env_utils.h"
#include "utils/general_utils.h"
#include "little_wrapper_app.h"
#include <errhandlingapi.h>
#include "traceback.h"

using namespace std;

LONG unhandled_exception_filter(EXCEPTION_POINTERS* ex)
{
    StackTraceback st(false);
    st.ShowCallstack(GetCurrentThread(), ex->ContextRecord);
    string stack = st.to_string(true);
    FatalAppExitA(0, stack.c_str());
    return EXCEPTION_CONTINUE_SEARCH;
}

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
    SetUnhandledExceptionFilter(unhandled_exception_filter);

    little_wrapper_app().main(argc, argv);
}
