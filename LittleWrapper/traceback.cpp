#include "traceback.h"
#include "project.h"
#include "utils/general_utils.h"
#include "utils/env_utils.h"

using namespace std;

StackTraceback::StackTraceback(bool get_callstack)
{
    if(get_callstack)
        ShowCallstack();
}

StackTraceback::StackTraceback(std::exception& ex)
{
    ShowCallstack(GetCurrentThread(), GetCurrentExceptionContext());
}

StackTraceback::StackTraceback(ExceptType except_type) : StackWalker(except_type)
{
    ShowCallstack();
}

std::string StackTraceback::to_string(bool detail)
{
    string executable_no_suffix = get_exe_filename(true);

    std::string result;
    char buf[512];
    for (stack_frame sf : stack_frames)
    {
        memset(buf, 0, 512);

        if (!detail)
        {
            // 过滤非项目内的Module
            if (sf.module != executable_no_suffix || sf.file.find(PROJECT_NAME) == -1)
                continue;

            // 缩减项目内的文件名
            string file_name = string_replace(sf.file, "/", "\\");
            if (file_name.find(PROJECT_NAME) != -1)
                file_name = file_name.substr(file_name.find("\\", file_name.find(PROJECT_NAME) + strlen(PROJECT_NAME) + 1) + 1);
            sf.file = file_name;
        }
        
        sprintf(buf, "%s : %s on %ld (%s)\n", sf.name.c_str(), sf.file.c_str(), sf.line, sf.module.c_str());
        result += buf;
    }
    return result;
}

//void StackTraceback::OnOutput(LPCSTR szText)
//{
//    printf("%s\n", szText);
//}

//void StackTraceback::OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName)
//{
//	printf("OnSymInit(): %s | %ld | %s\n" | szSearchPath, symOptions, szUserName);
//}
//
//void StackTraceback::OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
//{
//	printf("OnLoadModule(): %s | %s | %llx | %ld | %ld | %s | %s | %llx\n", img, mod, baseAddr, size, result, symType, pdbName, fileVersion);
//}

void StackTraceback::OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry)
{
	//printf("OnCallstackEntry(): 0x%llx -> ");
    stack_frame frame;
    frame.address = entry.offset;
    frame.name = entry.name;
    frame.line = entry.lineNumber;
    frame.file = entry.lineFileName;
    frame.module = entry.moduleName;

    stack_frames.emplace_back(frame);
}

//void StackTraceback::OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
//{
//	printf("OnDbgHelpErr(): %s | %ld | %llx\n", szFuncName, gle, addr);
//}
