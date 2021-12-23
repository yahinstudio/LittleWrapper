#pragma once
#include <string>

struct app_args
{
    // ͨ�ò���
    std::string input = ""; // unused
    std::string output = "";

    // ѹ��
    bool pack = false;
    std::string pack_src = "";
    bool pack_no_hash = false;
    std::string pack_exec = "";

    // ��ѹ
    bool extract = false;
    std::string extract_dest = "";

    // ����
    bool detail = false;

    // ����
    bool optarg_required = false;
    bool unknown_opt = false;
    std::string invaild_opt_name = "";

    // ���й�������ʾ����
    bool show_console = false;
    // ���й��������ش���
    bool hide_console = false;

    // �����ӽ������
    bool suppress_output = false;

    // ������������
    std::string start_parameters = "";

    // ����
    bool help = false;

    // ԭʼ������Ϣ
    int argc;
    char** argv;
};

app_args parse_app_args(int argc, char** argv);