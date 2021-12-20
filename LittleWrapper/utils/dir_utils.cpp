#include "dir_utils.h"
#include "../libs/cJSON-1.7.14/cJSON.h"
#include <io.h>
#include <windows.h>

using namespace std;

vector<file_struct> cal_dir_struct(string path)
{
    vector<file_struct> result;

    _finddata_t find;
    intptr_t handle = _findfirst((path + "\\*.*").c_str(), &find);

    if (handle != -1)
    {
        do {
            if (strcmp(find.name, ".") != 0 && strcmp(find.name, "..") != 0)
            {
                //cout <<"-"<< find.name << "-   " << (find.attrib & _A_SUBDIR) << endl;

                file_struct finfo;
                finfo.is_file = !(find.attrib & _A_SUBDIR);
                finfo.read_only = find.attrib & _A_RDONLY;
                finfo.hidden = find.attrib & _A_HIDDEN;
                finfo.create = find.time_create;
                finfo.access = find.time_access;
                finfo.write = find.time_write;
                finfo.name = find.name;
                finfo.length = find.size;
                if (!finfo.is_file)
                    finfo.children = cal_dir_struct(path + "\\" + finfo.name);

                result.push_back(finfo);
            }
        } while (_findnext(handle, &find) != -1);
        _findclose(handle);
    }

    return result;
}