#pragma once
#include <string>
#include "exceptions/exceptions.h"

#define DEBUG_MAX_ERROR_MESSAGE_LEN (8 * 1024)

#define error_check(expression, err_msg) _error_check(expression, err_msg, __FILE__, __LINE__,  __FUNCTION__, __DATE__, __TIME__)

#define exception_thrown(ex) _exception_thrown(ex, __FILE__, __LINE__,  __FUNCTION__, __DATE__, __TIME__);

void _error_check(int expression, std::string err_msg, const char* file, int line, const char* function, const char* data, const char* time);

void _exception_thrown(std::exception ex, const char* file, int line, const char* function, const char* data, const char* time);

void unknown_exception_thrown();

std::string get_last_error_message();