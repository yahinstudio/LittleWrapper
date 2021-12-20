#include "debug.h"
#include "string"
#include "utils/general_utils.h"
#include "assert.h" // assert()
#include "windows.h"
#include "project.h"
#include "exceptions/exceptions.h"

using namespace std;

void _error_check(int expression, std::string err_msg, const char* file, int line, const char* function, const char* data, const char* time)
{
	if (expression == 0)
	{
		char* buf = new char[DEBUG_MAX_ERROR_MESSAGE_LEN];

		sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN,
			"expression-value: %d\n"
			"file: %s on %d\n"
			"function: %s\n"
			"compile-time: %s %s\n"
			"cause: %s\n"
			"probable reason: %s(%d)\n"
	#ifdef _DEBUG
			"_________________________________traceback________________________________"
			"\n%s",
	#else
			,
	#endif
			expression,
			file,
			line,
			function,
			data, time,
			err_msg.c_str(),
			get_last_error_message().c_str(), GetLastError()
	#ifdef _DEBUG
			,
			traceback::stack_trace_text(true).c_str()
	#endif
		);

	#ifdef _DEBUG
		printf("---------------------------\n%s---------------------------\n", traceback::stack_trace_text(false).c_str());
	#endif

		MessageBoxA(nullptr, buf, PROJECT_NAME " " VERSION_TEXT " Error occured", MB_ICONERROR | MB_OK);

		delete[] buf;
		exit(1);
	}
}

void _exception_thrown(lw_base_exception ex, const char* file, int line, const char* function, const char* data, const char* time)
{
	char* buf = new char[DEBUG_MAX_ERROR_MESSAGE_LEN];

	sprintf_s(buf, DEBUG_MAX_ERROR_MESSAGE_LEN,
		"exception thrown: %s\n"
		"compile-time: %s %s\n"
		"cause: %s\n"
		"_________________________________traceback________________________________"
		"\n%s",
		typeid(ex).name(),
		__DATE__, __TIME__,
		ex.what(),
		traceback::stack_trace_text(true).c_str()
	);
	printf("---------------------------\n%s---------------------------\n", traceback::stack_trace_text(false).c_str());

#ifdef _DEBUG
	MessageBoxA(nullptr, buf, PROJECT_NAME " " VERSION_TEXT " Error occured", MB_ICONERROR | MB_OK);
#endif // _DEBUG
	
	delete[] buf;
	exit(1);
}

string get_last_error_message()
{
	string result;

	char* msg_buf;
	DWORD err_code = GetLastError();

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		err_code,
		0,
		(LPSTR)&msg_buf,
		0,
		0
	);

	result = msg_buf;
	free(msg_buf);

	return string_replace(result, "\r\n", "");
}

