#ifndef PERFCOUNT_WEIXIONG_070415
#define PERFCOUNT_WEIXIONG_070415


#include "perf_inst_impl.hpp"

/** 
 * @class	class counter_conf_parser
 * 
 * @brief	计数器配置文件解析类.
 * 
 * detail...
 * 
 * @author	TTSCodeAutoGener
 * @date	2011年8月8日
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
class counter_conf_parser
{
public:
    typedef counter_conf_parser self;
    static self* instance()
    {
        static self inst;
        return &inst;
    }

    int parse(const char* cfg);

    void get_process_qry_info(std::string& qry_names, std::string& qry_args) const
    {
        qry_names   = process_qry_names_;
        qry_args    = process_qry_args_;
    }

    const sp::str_arr& get_other_qrys() const
    {
        return other_qrys_;
    }
    
protected:
    std::string     process_qry_names_;
    std::string     process_qry_args_;
    sp::str_arr     other_qrys_;
};

/** 
 * @class	class perfman
 * 
 * @brief	性能管理类.
 * 
 * detail...
 * 
 * @author	TTSCodeAutoGener
 * @date	2011年8月8日
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
class perfman
{
public:
    typedef perfman self;
    static self* instance()
    {
        static self inst;
        return &inst;
    }

    virtual int start_count(const char* sid, const char* out_dir, const char* psz_cfg)
    {
        // 开始执行.
        time_t t = time( 0 ); 
        char start_ime[64]; 
        strftime( start_ime, sizeof(start_ime), "%Y/%m/%d %X",localtime(&t) );

        fprintf(stdout, "开始性能计数喽 ^_^ \n\n" );
        fprintf(stdout, "开始时刻\n%s\n", start_ime);
        fprintf(stdout, "现在时刻\n");

        int ret = 0;
        HANDLE sync_evt = CreateEvent(NULL, true, false, sid);
        if (NULL == sync_evt)
        {
            ret = GetLastError();
            fprintf(stderr, "create sync event %s failed with %d\n", sid, ret);
            return ret;
        }

        // 初始化counter_parser.
        ret = counter_conf_parser::instance()->parse(psz_cfg);
        if (ret != 0)
        {
            fprintf(stderr, "Invalid cfg file: %s\n", psz_cfg);
            return ret;
        }
        std::string process_name, process_qry_args;
        counter_conf_parser::instance()->get_process_qry_info(process_name, process_qry_args);
        const sp::str_arr& other_qry = counter_conf_parser::instance()->get_other_qrys();

        // 创建并启动统计器.
        perf_process_inst pp(out_dir, process_name.c_str(), process_qry_args.c_str() );
        perf_other_inst   po(out_dir, other_qry);
        pp.start_count();
        po.start_count();

        // 等待结束事件.
        unsigned int INTERVAL = perfman_conf::instance()->get_interval();
        while (WaitForSingleObject(sync_evt, 0) !=  WAIT_OBJECT_0)
        {
            time_t t = time( 0 ); 
            char CurrentTime[64]; 
            strftime( CurrentTime, sizeof(CurrentTime), "%Y/%m/%d %X",localtime(&t) );

            std::cout << CurrentTime;   //print time to console
            std::cout << "\r";

            pp.do_stat();
            po.do_stat();

            Sleep(INTERVAL);
        }

        // 结束统计.
        pp.stop_count();
        po.stop_count();

        return 0;
    }

    virtual int stop_count(const char* sid)
    {
        int ret = 0;
        HANDLE sync_evt = CreateEvent(NULL, true, false, sid);
        if (NULL == sync_evt)
        {
            ret = GetLastError();
            fprintf(stderr, "create sync event %s failed with %d\n", sid, ret);
            return ret;
        }
        SetEvent(sync_evt);
        return 0;
    }

protected:
    perfman()       {}

protected:

};


/************************************************************************/
/*                                                                      */
/************************************************************************/
inline int counter_conf_parser::parse(const char* cfg)
{
    std::string slash = "\\";
    std::string LeftBracket = "(";
    std::string RightBracket = ")";
    char *pReturnChar= new char [100];
    char tmp[MAX_PATH] = {0};

    //get "counter" objects
    GetPrivateProfileString("counter", "counters", NULL, tmp, sizeof(tmp) - 1, cfg);
    if (*tmp == 0)
    {
        fprintf(stderr, "invalid cfg file %s\n", cfg);
        return -1;
    }

    //split all the counter objects
    sp::str_arr sa;
    sp::split_str(tmp, sa);

    //parse each section
    for (sp::str_arr::iterator p = sa.begin(); p != sa.end(); ++p)
    {
        //get the counter's "name" and "instance"
        std::string Counter= *p;
        GetPrivateProfileString(Counter.c_str(), "name", NULL, tmp, sizeof(tmp) - 1, cfg);
        std::string name = std::string(tmp);
        std::string sCounterName = name;
        GetPrivateProfileString(Counter.c_str(), "Instance", NULL, tmp, sizeof(tmp) - 1, cfg);
        std::string instances = std::string(tmp);
        sCounterName = slash + sCounterName + LeftBracket + std::string(tmp) + RightBracket;

        //parse the "counters",then add all the counters to the counter name list
        GetPrivateProfileString(Counter.c_str(), "counters", NULL, tmp, sizeof(tmp) - 1, cfg);

        // 判断是不是进程模式.
        if (name == "Process")
        {
            process_qry_names_  = instances;
            process_qry_args_   = tmp;
            continue;
        }

        sp::str_arr counter_name_arr;
        sp::split_str(tmp, counter_name_arr);        
        for (sp::str_arr::iterator it = counter_name_arr.begin(); it != counter_name_arr.end(); ++it)
        {
            if (it->length() == 0)
                continue;

            std::string s = sCounterName + slash + *it;
            other_qrys_.push_back(s);            
        }
    }

    return 0;
}

#endif //end of #ifndef PERFCOUNT_WEIXIONG_070415

