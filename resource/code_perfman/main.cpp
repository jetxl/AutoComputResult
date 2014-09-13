#include "utils/sutils.h"
#include "mygetopt.h"
#include "perfman_comm.h"
#include "perf_counter.hpp"
#include "perfman.hpp"

#pragma comment ( lib , "Pdh.lib" )

void usage(int stauts)
{
    FILE* fout = (stauts != 0)? stderr: stdout;

    fprintf(fout, "基于pdh.dll的性能统计工具\n");
    fprintf(fout, "使用说明: perfman <-s|-k> [-f cfgfile] [-n sessionname] [-o outdir] [...]  [-h]\n");
    
    fprintf(fout, "\n");
    fprintf(fout, "必选项:\n");    
    fprintf(fout, "  -s         开始性能计数\n");
    fprintf(fout, "  -k         停止........\n");
    
    fprintf(fout, "\n");
    fprintf(fout, "可选项:\n");
    fprintf(fout, "  -f         配置文件\n");
    fprintf(fout, "  -n         会话名用于区分不同的实例\n");
    fprintf(fout, "  -o         性能计数结果输出目录，如果目录不存在会自动重建\n");
    fprintf(fout, "  -h         显示本帮助信息\n");

    fprintf(fout, "\n");
    fprintf(fout, "基它选项...:\n");
    fprintf(fout, "  -T         计数统计间隔，缺省1s\n");
    fprintf(fout, "  -R         输出结果的刷新频率, 缺省每统计10次刷新一次\n");

    fprintf(fout, "\n");
    fprintf(fout, "应用举例:\n");
    //fprintf(fout, "------------------------------------\n");
    fprintf(fout, "  按缺省配置会话启动计数：perfman -s\n");
    fprintf(fout, "  按缺省配置会话停止计数：perfman -k\n");
    fprintf(fout, "  指定配置会话  启动计数：perfman -s -f myperf.cfg -n myperf\n");
    fprintf(fout, "  仅需传入会话  停止计数：perfman -k -n myperf\n");

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
        case 'n':   // 会话名
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

    // 必选项参数有且仅有一个
    if (oper_arg_cnt != 1)
    {
        usage(1);
        return 1;
    }

    // 确保输出目录OK.
    if ( !sp::is_dir_exist(psz_out_dir) )
    {
        sp::create_directory(psz_out_dir);
    }

    // 将session.name带上前缀确保系统对象无重名.
    std::string sid  = psz_session_name + std::string(".my.perfman.event");
    psz_session_name = sid.c_str();

    int ret = 0;
    // 启动服务模式.
    if (start_svc)
    {
        ret = perfman::instance()->start_count(psz_session_name, psz_out_dir, psz_cfg_file);
    }

    // 停止服务模式.
    else
    {        
        ret = perfman::instance()->stop_count(psz_session_name);
    }

    return ret;
}
