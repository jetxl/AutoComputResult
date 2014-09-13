#ifndef __PERF_INST_H__
#define __PERF_INST_H__


struct perf_inst_i
{
    virtual int start_count()   = 0;
    virtual int stop_count()    = 0;
    virtual int do_stat()       = 0;
    virtual ~perf_inst_i()       {}
};


#endif // __PERF_INST_H__
