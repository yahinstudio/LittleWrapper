#include "project.h"
#include "pe_resource.h"
#include "utils/env_utils.h"
#include "resources/resource.h"
#include <string>
#include "debug.h"

using namespace std;

static char* application_version = nullptr;

char* get_application_version()
{
    if (application_version == nullptr)
    {
        string version = "0.0.0";

        try
        {
            pe_resource_reader reader(get_exe_path());
            pe_resource_reader::res_handle res = reader.open_resource("little_wrapper", IDR_LITTLE_WRAPPER_VERSION);
            version = string((char*) res.data, res.size);
        }
        catch (std::exception& ex)
        {
            error_check(false, "application version failed to be read");
        }

        application_version = new char[version.length() + 1];
        application_version[version.length()] = 0;
        memcpy(application_version, version.c_str(), version.length());
    }
    
    return application_version;
}