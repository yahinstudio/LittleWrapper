#include "general_utils.h"
#include "windows.h"
#include "stdio.h"
#include "io.h"
#include <iostream>
#include <fstream>
#include "../libs/md5/md5.h"
#include "direct.h"
#include "winuser.h"

using namespace std;

wchar_t* from_char_to_wchar(char* str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), 0, 0);
    WCHAR* m_wchar = new WCHAR[len + 1];
    MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str), m_wchar, len);
    m_wchar[len] = '\0';
    return m_wchar;
}

char* from_wchar_to_char(wchar_t* wchar)
{
    int len = WideCharToMultiByte(CP_ACP, 0, wchar, (int)wcslen(wchar), NULL, 0, NULL, NULL);
    char* m_char = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wchar, (int)wcslen(wchar), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    return m_char;
}

/* wchar_t to char* */
bool to_utf8(char* output_str, const wchar_t* wstr, size_t wstr_max_len)
{
    wstr_max_len = WideCharToMultiByte(CP_UTF8,              /* CodePage */
        0,                    /* dwFlags */
        wstr,                 /* lpWideCharStr */
        -1,                   /* cchWideChar - length in chars */
        output_str,           /* lpMultiByteStr */
        (DWORD)wstr_max_len,  /* cbMultiByte - length in bytes */
        NULL,                 /* lpDefaultChar */
        NULL                  /* lpUsedDefaultChar */
    );

    if (wstr_max_len == 0) 
    {
        printf("Failed to encode wchar_t as UTF-8.\n");
        return false;
    }
    return true;
}


/* char* to wchar_t*/
bool from_utf8(wchar_t* output_wstr, const char* str, size_t wstr_max_len)
{
    wstr_max_len = MultiByteToWideChar(CP_UTF8,              /* CodePage */
        0,                    /* dwFlags */
        str,                  /* lpMultiByteStr */
        -1,                   /* cbMultiByte - length in bytes */
        output_wstr,          /* lpWideCharStr */
        (DWORD)wstr_max_len   /* cchWideChar - length in chars */
    );

    if (wstr_max_len == 0) 
    {
        printf("failed to decode wchar_t from UTF-8.\n");
        return false;
    }
    return true;
}

string string_replace(string str, string oldstr, string newstr)
{
    size_t pos;
    while ((pos = str.find(oldstr)) != string::npos)
    {
        str = str.replace(pos, oldstr.length(), newstr);
    }
    return str;
}

long get_file_length(string file)
{
    file = string_replace(file, "/", "\\");

    _finddata_t find;
    intptr_t handle = _findfirst(file.c_str(), &find);

    //cout << "dinf: " << file << endl;

    if (handle != -1 && !(find.attrib & _A_SUBDIR))
    {
        auto size = find.size;
        _findclose(handle);
        return size;
    }

    return -1;
}

string get_file_md5(string file)
{
    file = string_replace(file, "/", "\\");

    if (!file_exists(file) || is_file_a_dir(file))
        return "";

    MD5 md5;
    std::fstream ff(file, std::fstream::in | std::fstream::binary);
    uint8_t* buf = new uint8_t[1024];
    while (!ff.eof())
    {
        ff.read((char*)buf, 1024);
        streamsize len = ff.gcount();
        if (len > 0)
            md5.update((void*)buf, len);
    }
    ff.close();
    delete[] buf;

    return md5.toString();
}

string get_stream_md5(std::fstream& stream, size_t len)
{
    MD5 md5;
    uint8_t* buf = new uint8_t[1024];
    while (!stream.eof() && len > 0)
    {
        size_t shouldRead = len >= 1024 ? 1024 : len;
        stream.read((char*)buf, shouldRead);
        streamsize readBytes = stream.gcount();
        if (readBytes > 0)
            md5.update((void*)buf, readBytes);
        len -= readBytes;
    }
    delete[] buf;

    return md5.toString();
}

string get_string_md5(string str)
{
    return MD5(str).toString();
}

string get_filename(string path)
{
    path = string_replace(path, "/", "\\");

    auto slash = path.rfind("\\");
    if (slash != std::string::npos)
        path = path.substr(slash + 1);

    auto found = path.rfind(".");

    if (found != std::string::npos)
        path = path.substr(0, found);

    return path;
}

std::string get_dir_name(std::string file)
{
    file = string_replace(file, "/", "\\");

    auto found = file.rfind("\\");

    if (found != std::string::npos)
        file = file.substr(0, found);

    return file;
}

bool check_path(string path)
{
    path = string_replace(path, "/", "\\");

    for (int i = 0; i < 10; i++)
    {
        size_t f1 = path.find("\\");
        string sec = path.substr(0, f1);

        if (sec == "." || sec.find("..") == 0)
            return false;

        if (f1 == string::npos)
            break;
        else
            path = path.substr(f1 + 1);
    }

    return true;
}

bool file_exists(string path)
{
    path = string_replace(path, "/", "\\");

    return _access(path.c_str(), 0) == 0;
}

// 判断文件或文件夹是否存在
bool file_exists2(string path)
{
    path = string_replace(path, "/", "\\");

    DWORD dwAttrib = GetFileAttributesA(path.c_str());
    return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

bool string_starts_with(string str, string starts_with)
{
    return str.find(starts_with) == 0;
}

bool is_file_a_dir(string path)
{
    path = string_replace(path, "/", "\\");

    _finddata_t find;
    intptr_t handle = _findfirst(path.c_str(), &find);

    if (handle != -1)
    {
        bool is_dir = find.attrib & _A_SUBDIR;
        _findclose(handle);
        return is_dir;
    }

    return false;
}

void remove_dir(string path)
{
    path = string_replace(path, "/", "\\");

    if (!file_exists(path))
        return;

    _finddata_t find;
    intptr_t handle = _findfirst((path + "\\*.*").c_str(), &find);

    if (handle != -1)
    {
        do {
            if (strcmp(find.name, ".") != 0 && strcmp(find.name, "..") != 0)
            {
                //cout <<"-"<< find.name << "-   " << (find.attrib & _A_SUBDIR) << endl;
                bool is_file = !(find.attrib & _A_SUBDIR);
                string name = path + "\\" + find.name;

                if (is_file)
                    remove(name.c_str());
                else
                    remove_dir(name);
            }
        } while (_findnext(handle, &find) != -1);
        _findclose(handle);

        if (is_file_a_dir(path))
            _rmdir(path.c_str());
        else
            remove(path.c_str());
    }
}

void show_dialog(string title, string text)
{
    printf("dialog: %s: %s\n", title.c_str(), text.c_str());
    int result = MessageBoxA(nullptr, text.c_str(), title.c_str(), MB_ICONERROR | MB_OK);
}

void set_window_visible(bool visible)
{
#if !defined(ENTRANCE_WINMAIN)
    // 隐藏console窗口
    HWND hwnd = FindWindowA("ConsoleWindowClass", NULL);
    if (hwnd)
        if(!!IsWindowVisible(hwnd) != visible)
            ShowWindowAsync(hwnd, visible ? SW_SHOW : SW_HIDE);
#endif
}

bool is_relative_path(string path)
{
    path = string_replace(path, "/", "\\");
    string disks = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    bool is_abs = path.length() >= 2 && disks.find(path[0]) != string::npos && path[1] == ':';
    bool is_netdisk = path.length() >= 2 && path[0] == '\\' && path[1] == '\\';
    //cout << "isabs: " << is_abs << "  is_netdisk: " << is_netdisk << endl;
    return !(is_abs || is_netdisk);
}