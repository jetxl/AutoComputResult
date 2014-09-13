#ifndef __PERFMAN_COMM_H__
#define __PERFMAN_COMM_H__


#pragma warning(disable:4996)


/** 
 * @class	class perfman_conf
 * 
 * @brief	性能计数公共配置类
 * 
 * detail...
 * 
 * @author	TTSCodeAutoGener
 * @date	2011年8月11日
 * 
 * @see		
 * 
 * @par 备注：
 * 
 */
class perfman_conf
{
public:
    perfman_conf()
        : interval_(1000)
        , refresh_freq_(10)
    {

    }

    typedef perfman_conf self;
    static self*        instance()
    {
        static self inst;
        return &inst;
    }

    unsigned int        get_interval()  const       { return interval_; }
    unsigned int        get_refresh_freq() const    { return refresh_freq_; }

    void                set_interval(unsigned int val)      { interval_ = val; }
    void                set_refresh_freq(unsigned int val)  { refresh_freq_ = val; }

protected:
    unsigned int        interval_;      // 统计间隔，内部按毫秒计.
    unsigned int        refresh_freq_;  // 刷新频率.
};








#endif // __PERFMAN_COMM_H__
