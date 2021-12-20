#pragma once
#include "libs/stack_walker/StackWalker.h"
#include <string>
#include <vector>

class StackTraceback : public StackWalker
{
public:
	struct stack_frame
	{
		std::string address;
		std::string name;
		int line;
		std::string file;
		std::string module;
	};

private:
	std::vector<stack_frame> stack_frames;

public:
	StackTraceback(bool get_callstack = true);

	StackTraceback(std::exception& ex);

	StackTraceback(ExceptType except_type);

	std::string to_string(bool detail = false);

protected:
	//virtual void OnOutput(LPCSTR szText);

	//virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName);

	/*virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size,
		DWORD result, LPCSTR symType, LPCSTR pdbName,
		ULONGLONG fileVersion);*/

	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry);

	//virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
};