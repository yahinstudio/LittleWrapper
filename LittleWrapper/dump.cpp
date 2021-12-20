#include "dump.h"
#include "project.h"
#include <dbghelp.h>
#include <Minidumpapiset.h>

#ifdef _WIN64
#pragma comment(lib, "libs/dbghelp/DbgHelp_x64.Lib")
#else
#pragma comment(lib, "libs/dbghelp/DbgHelp_x86.Lib")
#endif

void create_dump_file(std::string dump_file, PEXCEPTION_POINTERS ex_pointers)
{
	HANDLE file = CreateFileA(dump_file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	HANDLE process = GetCurrentProcess();
	DWORD process_id = GetCurrentProcessId();

	MINIDUMP_EXCEPTION_INFORMATION ex_info;
	ex_info.ThreadId = GetCurrentThreadId();
	ex_info.ExceptionPointers = ex_pointers;
	ex_info.ClientPointers = false;

	//MiniDumpWriteDump(process, process_id, file, MiniDumpWithFullMemory, NULL, NULL, NULL);

	MiniDumpWriteDump(process, process_id, file, MiniDumpNormal, ex_pointers != nullptr ? &ex_info : nullptr, nullptr, nullptr);

	CloseHandle(file);
}