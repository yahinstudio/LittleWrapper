#pragma once
#include "string"
#include "utils.h"
#include "assert.h"
#include "project.h" // ENABLED_ERROR_CHECK
#include "windows.h"

#if defined(ENABLED_ERROR_CHECK)

#define error_check(expression, err_msg) do { \
	if(expression == 0) { \
		char* buf = new char[300]; \
		std::string cause = err_msg; \
		std::string last_error_msg = get_last_error_message(); \
		sprintf_s(buf, 300, "Error occured: %d\nfile: %s on line %d\nfunc: %s\ncompilation-time: %s %s\n\ncause: %s\n\nprobable reason: %s(%d)", \
							expression, __FILE__, __LINE__,  __FUNCTION__, __DATE__, __TIME__, cause.c_str(), last_error_msg.c_str(), GetLastError()); \
		show_dialog(PROJECT_NAME" "VERSION_TEXT, buf); \
		delete[] buf; \
		exit(1); \
	} \
} while (0)

#else

void error_check(bool expression, std::string err_msg);

#endif

std::string get_last_error_message();