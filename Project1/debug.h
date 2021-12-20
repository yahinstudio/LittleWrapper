#pragma once
#include <string>
#include <assert.h>
#include <windows.h>
#include "utils/general_utils.h"
#include "project.h"
#include "traceback.h"
#include "exceptions/exceptions.h"

#define DEBUG_MAX_ERROR_MESSAGE_LEN (8 * 1024)

#define error_check(expression, err_msg) _error_check(expression, err_msg, __FILE__, __LINE__,  __FUNCTION__, __DATE__, __TIME__)

#define exception_thrown(ex) _exception_thrown(ex, __FILE__, __LINE__,  __FUNCTION__, __DATE__, __TIME__);

void _error_check(int expression, std::string err_msg, const char* file, int line, const char* function, const char* data, const char* time);

void _exception_thrown(lw_base_exception ex, const char* file, int line, const char* function, const char* data, const char* time);

std::string get_last_error_message();