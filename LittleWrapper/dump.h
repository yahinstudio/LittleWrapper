#pragma once
#include <string>
#include <windows.h>

#ifdef ENABLED_DUMPFILE
#define CreateDumpFile(file) create_dump_file(file);
#define CreateDumpFileWithExPointers(file, ex_pointers) create_dump_file(file, ex_pointers);
#else
#define CreateDumpFile(file)
#define CreateDumpFileWithExPointers(file, ex_pointers)
#endif // ENABLED_DUMPFILE

void create_dump_file(std::string dump_file, PEXCEPTION_POINTERS ex_pointers = nullptr);