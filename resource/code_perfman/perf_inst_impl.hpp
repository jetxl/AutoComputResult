#ifndef __PERF_INST_IMPL_HPP__
#define __PERF_INST_IMPL_HPP__


#include "utils/sutils.h"
#include "perf_inst_i.h"
#include "perf_counter.hpp"
#include "win32_os_utils.h"

typedef perf_counter<double>        perf_dbl_counter;
typedef std::list<perf_dbl_counter> perf_dbl_counter_list_t;


enum result_type_t
{
    rt_long,
    rt_mem,
    rt_dobule,
    rt_cnt
};

/** 
 * @class	class perf_def_inst
 * 
 * @brief	缺省的计数实例类.
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
class perf_def_inst : public perf_inst_i
{
public:
    virtual int start_count()
    {
        return 0;
    }

    virtual int stop_count()
    {
        return 0;
    }

    virtual int do_stat()
    {
        return 0;
    }

    perf_def_inst()
        : stated_times_(0)
    {

    }

protected:
    void fmt_2_string(char* psz, double r, unsigned int ix)
    {        
        // 格式输出.
        if (result_type_arr_[ix] == rt_long)
        {
            sprintf(psz, "%-20d", (long)r );
        }
        else if (result_type_arr_[ix] == rt_mem)
        {
            char t[64];
            sprintf(t, "%dM", long(r / 1024 / 1024));
            sprintf(psz, "%-20s", t);
        }
        else
        {
            char t[64];
            sprintf(t, "%-15.2f", r);            
            sprintf(psz, "%-20s", t);
        }
    }

    typedef std::vector<result_type_t>  result_type_arr_t;  
    result_type_arr_t                   result_type_arr_;
    sp::str_arr                         stat_type_arr_;
    size_t                              stated_times_;
};


/** 
 * @class	class perf_process_inst : public perf_def_inst
 * 
 * @brief   进程计数实例类.
 * 
 *  
 * 
 * @author	TTSCodeAutoGener
 * @date	2011年8月8日
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
class perf_process_inst : public perf_def_inst
{
public:
    perf_process_inst(const char* out_dir, const char* query_name, const char* qurey_args)
        : stat_col_cnt_(0)
        , result_dir_(out_dir)        
    {
        if ( *result_dir_.rbegin() != '\\' )
        {
            result_dir_.push_back('\\');
        }

        init(query_name, qurey_args);
    }

    virtual int stop_count()
    {
        write_whole_process_summar();
        write_single_process_summar();

        for (size_t k = 0; k < sub_proc_item_arr_.size(); ++k)
        {
            proc_item_t& pi = sub_proc_item_arr_[k];
            perf_dbl_counter_list_t& counters = pi.counters;
            for (perf_dbl_counter_list_t::iterator p = counters.begin(); p != counters.end(); ++p)
            {
                p->Uninitial();
            }
            counters.clear();
            if (NULL != pi.fdetail) fclose(pi.fdetail);
        }
        sub_proc_item_arr_.clear();

        return 0;
    }

    virtual int do_stat()
    {
        typedef std::vector<double> dbl_result_vec_t;
        dbl_result_vec_t cur_result, total_result;
        cur_result.resize( stat_col_cnt_ );
        total_result.resize(stat_col_cnt_ );
        char tmp[1024] = {0};

        const unsigned int REFRSH_FREQ = perfman_conf::instance()->get_refresh_freq();

        time_t t = time( 0 ); 
        char cur_tm[64]; 
        strftime( cur_tm, sizeof(cur_tm), "%Y/%m/%d %X", localtime(&t) );

        ++stated_times_;
        for (size_t k = 0; k < sub_proc_item_arr_.size(); ++k)
        {
            bool collect_ok = true;
            proc_item_t& pi = sub_proc_item_arr_[k];
            perf_dbl_counter_list_t& counters = pi.counters;
            int ix = 0;
            char* psz = tmp;
            sprintf(psz, "%-25s", cur_tm); psz += strlen(psz);
            for (perf_dbl_counter_list_t::iterator p = counters.begin(); p != counters.end(); ++p)
            {
                collect_ok = p->CollectData();
                if (!collect_ok)
                    break;
                cur_result[ix] = p->GetRealtimeData();

                fmt_2_string(psz, cur_result[ix], ix);
                psz += strlen(psz);

                total_result[ix] += cur_result[ix];
                ++ix;                
            }

            // 简单期间写个文件得了.
            sp::trim_str(tmp);
            if (ix == cur_result.size() )
                fprintf(pi.fdetail, "%s\n", tmp);
            if (stated_times_ % REFRSH_FREQ == 0)
            {
                fflush(pi.fdetail);
            }
        }

        // 写整体的detail.
        char* psz = tmp;
        sprintf(psz, "%-25s", cur_tm); psz += strlen(psz);
        for (size_t k = 0; k < result_type_arr_.size(); ++k)
        {
            fmt_2_string(psz, total_result[k], (unsigned int)k);
            psz += strlen(psz);
        }
        sp::trim_str(tmp);
        fprintf(whole_proc_item_.fdetail, "%s\n", tmp);

        // 每次写一次整体统计结果.
        if (stated_times_ % REFRSH_FREQ == 0)
        {
            fflush(whole_proc_item_.fdetail);
            write_whole_process_summar();
            write_single_process_summar();
        }

        return 0;
    }

protected:
    int init(const char* query_name, const char* qurey_args)
    {
        // #1. 先得到进程列表.
        sp::str_arr pa;        
        sp::split_str(query_name, pa, ",");      

        // #2. 分解查询字符串.
        sp::str_arr argv;        
        stat_col_cnt_ = 0;
        {
            sp::str_arr argv2;
            sp::split_str(qurey_args, argv2, ",");
            for (size_t k = 0; k < argv2.size(); ++k)
                if (argv2[k].length() != 0)
                    argv.push_back(argv2[k]), ++stat_col_cnt_;
        }

        // #3. 创建计数器数组.
        // #3.1 枚举进程，确定监视的个数.
        std::vector<int>    pac;
        int total_process_cnt = 0;
        pac.resize(pa.size() );
        for (size_t k = 0; k < pa.size(); ++k)
        {
            pac[k] = win32::os_utils::query_process_cnt(pa[k].c_str() );
            total_process_cnt += pac[k];
        }
        // #3.2 分配对象.
        int ix = 0;
        sub_proc_item_arr_.resize(total_process_cnt);
        for (size_t k = 0; k < pa.size(); ++k)
        {
            char tmp[MAX_PATH] = {0};
            for (int j = 0; j < pac[k]; ++j)
            {
                proc_item_t& pi = sub_proc_item_arr_[ix]; ++ix;
                pi.pname    = pa[k];
                if (0 == j)
                {
                    pi.pobjname    = pa[k];
                }
                else
                {
                    sprintf(tmp, "%s#%d", pa[k].c_str(), j);
                    pi.pobjname =  tmp;
                }
                pi.detail_file = result_dir_ + pi.pobjname + ".perf.detail.txt";
                pi.summar_file = result_dir_ + pi.pobjname + ".perf.summar.txt";

                pi.fdetail = fopen(pi.detail_file.c_str(), "wt");
                if (NULL == pi.fdetail)
                {
                    fprintf(stderr, "open result file failed with [%d]!! [%s] [%s]\n", 
                        GetLastError(), pi.detail_file.c_str(), pi.summar_file.c_str() );
                    continue;
                }                
            }
        }

        // #3.3 创建整体.
        whole_proc_item_.pname = "##whole##";
        whole_proc_item_.pobjname = whole_proc_item_.pname;
        whole_proc_item_.detail_file = result_dir_ + whole_proc_item_.pobjname + ".perf.detail.txt";
        whole_proc_item_.summar_file = result_dir_ + whole_proc_item_.pobjname + ".perf.summar.txt";
        whole_proc_item_.fdetail = fopen(whole_proc_item_.detail_file.c_str(), "wt");
        if (NULL != whole_proc_item_.fdetail)        
        {
            char col_head[1024];
            char* psz = col_head;
            proc_item_t& pi = whole_proc_item_;
            sprintf(psz, "%-25s", "Time"); psz += strlen(psz);
            for (size_t kk = 0; kk < argv.size(); ++kk)
            {
                fprintf(pi.fdetail, "\\Process(%s)\\%s\n", pi.pobjname.c_str(), argv[kk].c_str() );
                sprintf(psz, "%-20s", argv[kk].c_str() );    psz += strlen(psz);
            }
            sp::trim_str(col_head);
            fprintf(pi.fdetail, "%s\n", col_head);
            fflush(pi.fdetail);
        }

        // #4. 开始计数.
        for (size_t k = 0; k < sub_proc_item_arr_.size(); ++k)
        {
            char tmp[256] = {0};
            proc_item_t& pi = sub_proc_item_arr_[k];

            // 添加查询
            sp::str_arr sa;
            char col_head[1024];
            char* psz = col_head;
            sprintf(psz, "%-25s", "Time"); psz += strlen(psz);
            for (size_t kk = 0; kk < argv.size(); ++kk)
            {
                sprintf(tmp, "\\Process(%s)\\%s", pi.pobjname.c_str(), argv[kk].c_str() );
                pi.counters.push_back( perf_dbl_counter() );
                pi.counters.back().SetName(tmp);
                if ( ! pi.counters.back().Initial() )
                {
                    std::cerr << "counter failed: " << tmp << std::endl;
                }
                sa.push_back(tmp);
                sprintf(psz, "%-20s", argv[kk].c_str() );    psz += strlen(psz);
            }
            for (size_t kk = 0; kk < sa.size(); ++kk)
            {
                fprintf(pi.fdetail, "%s\n", sa[kk].c_str() );
            }
            sp::trim_str(col_head);
            fprintf(pi.fdetail, "%s\n", col_head);
            fflush(pi.fdetail);
        }

        // #5. 数据输出类型分类.
        for (size_t k = 0; k < argv.size(); ++k)
        {
            const std::string& s = argv[k];

            if (s.rfind("Working Set") != std::string::npos )
            {
                result_type_arr_.push_back(rt_mem);
            }
            else if (s.find("% ") != std::string::npos )
            {
                result_type_arr_.push_back(rt_dobule);
            }
            else            
            {
                result_type_arr_.push_back(rt_long);
            }
        }

        stat_type_arr_ = argv;

        return 0;
    }

    int write_single_process_summar()
    {
        char tmp[1024] = {0};
        for (size_t k = 0; k < sub_proc_item_arr_.size(); ++k)
        {
            proc_item_t& pi = sub_proc_item_arr_[k];
            perf_dbl_counter_list_t& counters = pi.counters;

            FILE* fsummar = fopen(pi.summar_file.c_str(), "wt");
            if (NULL == fsummar)
            {
                continue;
            }

            // 写入内容.
            sprintf(tmp, "%-40s%-20s%-20s%-20s", "记录项", "峰值", "均值", "统计次数");
            sp::trim_str(tmp);
            fprintf(fsummar, "%s\n", tmp);
            
            int ix = 0;
            char* psz = tmp;
            for (perf_dbl_counter_list_t::iterator p = counters.begin(); p != counters.end(); ++p)
            {
                double max_data = p->GetMaxData();
                double ave_data = p->GetSumData();
                long   record_cnt = p->GetRecordedCount();

                if (record_cnt != 0)
                    ave_data /= record_cnt;

                psz = tmp;
                sprintf(psz, "%-40s", p->GetName().c_str() ); psz += strlen(psz);
                fmt_2_string(psz, max_data, ix);    psz += strlen(psz);
                fmt_2_string(psz, ave_data, ix);    psz += strlen(psz);
                sprintf(psz, "%-20d", record_cnt);  psz += strlen(psz);
                sp::trim_str(tmp);
                fprintf(fsummar, "%s\n", tmp);

                ++ix;
            }

            fclose(fsummar);
        }

        return 0;
    }

    int write_whole_process_summar()
    {
        char tmp[1024] = {0};
        std::vector<double> total_result;
        total_result.resize(stat_type_arr_.size() );
        // 将各项的和统计出来.
        for (size_t k = 0; k < sub_proc_item_arr_.size(); ++k)
        {
            proc_item_t& pi = sub_proc_item_arr_[k];
            perf_dbl_counter_list_t& counters = pi.counters;
            int ix = 0;            
            for (perf_dbl_counter_list_t::iterator p = counters.begin(); p != counters.end(); ++p)
            {
                total_result[ix] += p->GetSumData();
                ++ix;
            }  
        }

        // 输出结果.
        int total_cnt = 1;
        if (sub_proc_item_arr_.size() != 0)
        {
            total_cnt = sub_proc_item_arr_.front().counters.front().GetRecordedCount();
        }        

        FILE* fsummar = fopen(whole_proc_item_.summar_file.c_str(), "wt");
        if (NULL == fsummar)
        {
            return GetLastError();
        }

        // 写入内容.
        sprintf(tmp, "%-40s%-20s%-20s%-20s", "记录项", "峰值", "均值", "统计次数");
        sp::trim_str(tmp);
        fprintf(fsummar, "%s\n", tmp);
        for (unsigned int k = 0; k < (unsigned int)stat_type_arr_.size(); ++k)
        {            
            double ave_data = total_result[k] / total_cnt;
            char* psz = tmp;
            char tmp2[256];
            sprintf(tmp2, "\\Process(%s)\\%s", whole_proc_item_.pobjname.c_str(), stat_type_arr_[k].c_str() );
            sprintf(psz, "%-40s", tmp2 ); psz += strlen(psz);
            sprintf(psz, "%-20d", -1);          psz += strlen(psz);
            fmt_2_string(psz, ave_data, k);     psz += strlen(psz);
            sprintf(psz, "%-20d", total_cnt);   psz += strlen(psz);                
            sp::trim_str(tmp);
            fprintf(fsummar, "%s\n", tmp);
        }
        fclose(fsummar);

        return 0;
    }

protected:
    struct  proc_item_t
    {        
        std::string                 pname;
        std::string                 pobjname;
        std::string                 detail_file;    // 计数文件.
        std::string                 summar_file;    // 报表文件.
        FILE*                       fdetail;
        perf_dbl_counter_list_t     counters;
    };
    typedef std::vector<proc_item_t>    proc_item_arr_t;

    proc_item_arr_t         sub_proc_item_arr_;
    proc_item_t             whole_proc_item_;
    std::string             result_dir_;
    unsigned int            stat_col_cnt_;
};


/** 
 * @class	class perf_other_inst : public perf_def_inst
 * 
 * @brief	其它计数实例类.
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
class perf_other_inst : public perf_def_inst
{
public:
    perf_other_inst(const char* out_dir, const sp::str_arr& qry)
    {        
        std::string od = out_dir;
        if ( *od.rbegin() != '\\')
        {
            od.push_back('\\');
        }
        detail_file_ = od + "##other##" + ".perf.detail.txt";
        summar_file_ = od + "##other##" + ".perf.summar.txt";

        for (size_t k = 0; k < qry.size(); ++k)
        {
            counters_.push_back( perf_dbl_counter() );
            perf_dbl_counter& pdc = counters_.back();
            pdc.SetName(qry[k]);
            if ( !pdc.Initial() )
            {
                counters_.pop_back();
                continue;
            }

            // 先截断到尾部.
            std::string s = qry[k].substr(qry[k].rfind('\\') + 1 );
            if (s.find("sec") != std::string::npos || s.find_first_of("/%") != std::string::npos)
            {
                result_type_arr_.push_back(rt_dobule);
            }
            else
            {
                result_type_arr_.push_back(rt_long);
            }
            stat_type_arr_.push_back(s);
        }

        // 打开文件.
        fdetail_ = fopen(detail_file_.c_str(), "wt");
        if (NULL != fdetail_)
        {
            // 写个文件头.           
            for (perf_dbl_counter_list_t::iterator p = counters_.begin(); p != counters_.end(); ++p)
            {                 
                fprintf(fdetail_, "%s\n", p->GetName().c_str() );                
            }
            char title[1024];
            char* psz = title;
            sprintf(psz, "%-25s", "Time"); psz += strlen(psz);            
            for (size_t ix = 0; ix < stat_type_arr_.size(); ++ix)
            {
                sprintf(psz, "%-20s", stat_type_arr_[ix].c_str() );
                psz += strlen(psz);
            }
            sp::trim_str(title);
            fprintf(fdetail_, "%s\n", title);
            fflush(fdetail_);
        }
    }

    virtual ~perf_other_inst()
    {
        if (NULL != fdetail_)
        {
            fclose(fdetail_);
            fdetail_ = NULL;
        }
    }

    virtual int stop_count()
    {
        write_summar_i();
        for (perf_dbl_counter_list_t::iterator p = counters_.begin(); p != counters_.end(); ++p)
        {
            p->Uninitial();
        }
        counters_.clear();
        if (NULL != fdetail_)
        {
            fclose(fdetail_);
            fdetail_ = NULL;
        }

        return 0;
    }

    virtual int do_stat()
    {        
        const unsigned int REFRSH_FREQ = perfman_conf::instance()->get_refresh_freq();

        time_t t = time( 0 ); 
        char cur_tm[64]; 
        strftime( cur_tm, sizeof(cur_tm), "%Y/%m/%d %X", localtime(&t) );

        ++stated_times_;

        char tmp[1024];
        char* psz = tmp;
        sprintf(psz, "%-25s", cur_tm);  psz += strlen(psz);
        size_t ix = 0;
        for (perf_dbl_counter_list_t::iterator p = counters_.begin(); p != counters_.end(); ++p)
        {            
            if ( !p->CollectData() )
            {
                break;
            }

            double r = p->GetRealtimeData();
            fmt_2_string(psz, r, (unsigned int)ix++);
            psz += strlen(psz);
        }

        if (ix == stat_type_arr_.size() )
        {
            sp::trim_str(tmp);
            fprintf(fdetail_, "%s\n", tmp);
        }

        if (stated_times_ % REFRSH_FREQ == 0)
        {
            fflush(fdetail_);
            write_summar_i();
        }

        return 0;
    }

protected:
    int write_summar_i()
    {
        char tmp[1024] = {0};
        FILE* fsummar = fopen(summar_file_.c_str(), "wt");
        if (NULL == fsummar)
        {
            return -1;
        }

        // 写入内容.
        sprintf(tmp, "%-30s%-20s%-20s%-20s", "记录项", "峰值", "均值", "统计次数");
        sp::trim_str(tmp);
        fprintf(fsummar, "%s\n", tmp);

        char* psz = tmp;
        unsigned int ix = 0;
        for (perf_dbl_counter_list_t::iterator p = counters_.begin(); p != counters_.end(); ++p)
        {
            double max_data = p->GetMaxData();
            double ave_data = p->GetSumData();
            long   record_cnt = p->GetRecordedCount();

            if (record_cnt != 0)
                ave_data /= record_cnt;

            psz = tmp;
            sprintf(psz, "%-30s", stat_type_arr_[ix].c_str() ); psz += strlen(psz);
            fmt_2_string(psz, max_data, ix);    psz += strlen(psz);
            fmt_2_string(psz, ave_data, ix);    psz += strlen(psz);
            sprintf(psz, "%-20d", record_cnt);  psz += strlen(psz);
            sp::trim_str(tmp);
            fprintf(fsummar, "%s\n", tmp);

            ++ix;
        }

        fclose(fsummar);
        return 0;
    }

protected:
    perf_dbl_counter_list_t     counters_;
    std::string                 detail_file_;
    std::string                 summar_file_;
    FILE*                       fdetail_;
};


#endif // __PERF_INST_HPP__
