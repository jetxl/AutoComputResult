#ifndef __PERFMAN_COMM_H__
#define __PERFMAN_COMM_H__


#pragma warning(disable:4996)


/** 
 * @class	class perfman_conf
 * 
 * @brief	���ܼ�������������
 * 
 * detail...
 * 
 * @author	TTSCodeAutoGener
 * @date	2011��8��11��
 * 
 * @see		
 * 
 * @par ��ע��
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
    unsigned int        interval_;      // ͳ�Ƽ�����ڲ��������.
    unsigned int        refresh_freq_;  // ˢ��Ƶ��.
};








#endif // __PERFMAN_COMM_H__
