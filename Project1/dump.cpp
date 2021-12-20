#include "dump.h"
#include <windows.h>
#include <dbghelp.h>
#include <Minidumpapiset.h>

#ifdef _WIN64
#pragma comment(lib, "libs/dbghelp/DbgHelp_x64.Lib")
#else
#pragma comment(lib, "libs/dbghelp/DbgHelp_x86.Lib")
#endif

void create_dump_file(std::string dump_file)
{
	HANDLE file = CreateFileA(dump_file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE process = GetCurrentProcess();
	DWORD process_id = GetCurrentProcessId();

	MiniDumpWriteDump(process, process_id, file, MiniDumpWithFullMemory, NULL, NULL, NULL);

	CloseHandle(file);
}