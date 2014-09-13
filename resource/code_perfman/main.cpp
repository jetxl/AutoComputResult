#include "utils/sutils.h"
#include "mygetopt.h"
#include "perfman_comm.h"
#include "perf_counter.hpp"
#include "perfman.hpp"

#pragma comment ( lib , "Pdh.lib" )

void usage(int stauts)
{
    FILE* fout = (stauts != 0)? stderr: stdout;

    fprintf(fout, "����pdh.dll������ͳ�ƹ���\n");
    fprintf(fout, "ʹ��˵��: perfman <-s|-k> [-f cfgfile] [-n sessionname] [-o outdir] [...]  [-h]\n");
    
    fprintf(fout, "\n");
    fprintf(fout, "��ѡ��:\n");    
    fprintf(fout, "  -s         ��ʼ���ܼ���\n");
    fprintf(fout, "  -k         ֹͣ........\n");
    
    fprintf(fout, "\n");
    fprintf(fout, "��ѡ��:\n");
    fprintf(fout, "  -f         �����ļ�\n");
    fprintf(fout, "  -n         �Ự���������ֲ�ͬ��ʵ��\n");
    fprintf(fout, "  -o         ���ܼ���������Ŀ¼�����Ŀ¼�����ڻ��Զ��ؽ�\n");
    fprintf(fout, "  -h         ��ʾ��������Ϣ\n");

    fprintf(fout, "\n");
    fprintf(fout, "����ѡ��...:\n");
    fprintf(fout, "  -T         ����ͳ�Ƽ����ȱʡ1s\n");
    fprintf(fout, "  -R         ��������ˢ��Ƶ��, ȱʡÿͳ��10��ˢ��һ��\n");

    fprintf(fout, "\n");
    fprintf(fout, "Ӧ�þ���:\n");
    //fprintf(fout, "------------------------------------\n");
    fprintf(fout, "  ��ȱʡ���ûỰ����������perfman -s\n");
    fprintf(fout, "  ��ȱʡ���ûỰֹͣ������perfman -k\n");
    fprintf(fout, "  ָ�����ûỰ  ����������perfman -s -f myperf.cfg -n myperf\n");
    fprintf(fout, "  ���贫��Ự  ֹͣ������perfman -k -n myperf\n");

    fprintf(fout, "\n");
    fprintf(fout, "====================================\n");
    fprintf(fout, "Version 1.0.0\n");
    fprintf(fout, "(C) 1999-2011 iFly Info Tek.\n");
    fprintf(fout, "All rights reserved.\n");
    fprintf(fout, "====================================\n");

    return;
}

int main(int argc, char *argv[])
{
    const char* psz_session_name = "default";
    const char* psz_cfg_file = ".\\perfman.cfg";
    const char* psz_out_dir  = "results";    
    bool  start_svc = true;     // 
    int   oper_arg_cnt = 0;

    int opt = 0;
    MY_DISABLE_DEFAULT_UNKNOWN_OPTS_ERR();
    while ( (opt = getopt(argc, argv, "skf:n:o:T:R:h")) != EOF )
    {
        switch (opt)
        {
        case 's':
            ++oper_arg_cnt;
            start_svc   = true;
            break;
        case 'k':
            ++oper_arg_cnt;
            start_svc   = false;
            break;
        case 'f':
            psz_cfg_file = optarg;
            break;
        case 'n':   // �Ự��
            psz_session_name = optarg;
            break;
        case 'o':
            psz_out_dir = optarg;
            break;
        case 'T':
            perfman_conf::instance()->set_interval(1000 * atoi(optarg));
            break;
        case 'R':
            perfman_conf::instance()->set_refresh_freq(1000 * atoi(optarg));
            break;
        case 'h':
            usage(0);
            return 0;
        case '?':
        default:
            usage(2);
            return 2;
        }
    }

    // ��ѡ��������ҽ���һ��
    if (oper_arg_cnt != 1)
    {
        usage(1);
        return 1;
    }

    // ȷ�����Ŀ¼OK.
    if ( !sp::is_dir_exist(psz_out_dir) )
    {
        sp::create_directory(psz_out_dir);
    }

    // ��session.name����ǰ׺ȷ��ϵͳ����������.
    std::string sid  = psz_session_name + std::string(".my.perfman.event");
    psz_session_name = sid.c_str();

    int ret = 0;
    // ��������ģʽ.
    if (start_svc)
    {
        ret = perfman::instance()->start_count(psz_session_name, psz_out_dir, psz_cfg_file);
    }

    // ֹͣ����ģʽ.
    else
    {        
        ret = perfman::instance()->stop_count(psz_session_name);
    }

    return ret;
}
