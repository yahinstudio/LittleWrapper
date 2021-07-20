#include "single_ins.h"
#include "string"
#include "windows.h"
#include "debug.h"

using namespace std;

bool request_single_instance_lock(string name)
{
	HANDLE p =  CreateMutexA(nullptr, false, name.c_str());

	error_check(p != nullptr, "Failed to create Mutex.");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(p);
		return false;
	}

	return true;
}
