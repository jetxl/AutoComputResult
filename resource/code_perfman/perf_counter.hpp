#ifndef __PREF_COUNTER_HPP__
#define __PREF_COUNTER_HPP__

#include <Windows.h>
#include <Pdh.h>
#include <iostream>
#include <cassert>

/** 
* @class	PdhCounter
* 
* @brief	class for the windows PDH counters
* 
* detail...
* 
* @author	weixiong
* @date	2007年4月13日
* 
* @see		
* 
* @par 备注：
* 
*/
template <typename T>
class perf_counter
{
public:
    perf_counter();
    virtual ~perf_counter();
    bool				Initial(const std::string sCounterName = "");//initial the counter by the given name
    void				SetName(const std::string sName);//set the counter's name
    std::string			GetName();//get the name of counter
    bool				CollectData();//collect the real-time data
    T					GetRealtimeData();//get real-time data
    long				GetRecordedCount();//get the count of all the recorded data until now
    T					GetSumData();//get the summation of all the recorded data
    T					GetMaxData();//get the maximum data of all the recorded data
    bool				Uninitial();//destroy the counter

private:
    PDH_FMT_COUNTERVALUE		m_CurValue;//value collected by the counter
    std::string					m_Name;
    T							m_RecordValue;//value we want to record
    T							m_MaxValue;
    long						m_RecordedNum;
    T							m_SumData;
    HQUERY						m_hQuery;
    HCOUNTER					*m_pCounter;
    DWORD						m_DataType;
    DWORD						m_CounterType;
    PDH_STATUS					m_Status;
};

template <class T>
inline perf_counter<T>::perf_counter()
{
    memset(&m_CurValue, 0, sizeof(PDH_FMT_COUNTERVALUE));//给m_CurValue赋初值
    m_Name = "";
    m_MaxValue = T(0);
    m_RecordedNum = 0;
    m_SumData = T(0);
    m_hQuery = NULL;
    m_pCounter = NULL;
    if(strcmp(typeid(T).name(), "long") == 0)
    {
        m_DataType = PDH_FMT_LONG | PDH_FMT_NOCAP100;
    }
    else if (strcmp(typeid(T).name(), "double") == 0)
    {
        m_DataType = PDH_FMT_DOUBLE | PDH_FMT_NOCAP100;
    }
    else if (strcmp(typeid(T).name(), "__int64") == 0)
    {
        m_DataType = PDH_FMT_LARGE | PDH_FMT_NOCAP100;
    }
    else
    {
        std::cout << typeid(T).name() << std::endl;
        std::cerr << "Bad template type!" << std::endl;
        assert(false && "Bad template type!");
        exit(1);
    }
    m_RecordValue = T(0);
    m_CounterType = 0;
    m_Status = 0;

}

template <class T>
inline perf_counter<T>::~perf_counter()
{
    Uninitial();
}


template <class T>
inline bool perf_counter<T>::Initial(const std::string sCounterName)
{
    //打开计数器
    std::string CounterName = "";
    if (sCounterName != "")
    {
        CounterName = sCounterName;
        m_Name = sCounterName;
    }
    else
    {
        if (m_Name == "")
        {
            std::cerr << "Initial with wrong name." << std::endl << std::flush;
            return false;
        }
        CounterName = m_Name;
    }
    m_Status = PdhOpenQuery ( 0 , 0 , &m_hQuery );
    assert(m_Status == ERROR_SUCCESS);
    if ( m_Status != ERROR_SUCCESS )
    {
        return false;
    }
    m_pCounter = (HCOUNTER *)GlobalAlloc(GPTR, sizeof(HCOUNTER));
    assert(m_Status == ERROR_SUCCESS);
    if ( m_pCounter == NULL )
    {
        return false;
    }

    //创建计数器
    m_Status = PdhAddCounter(m_hQuery, 
        CounterName.c_str(),
        0,
        m_pCounter);
    assert(m_Status == ERROR_SUCCESS);
    if ( m_Status != ERROR_SUCCESS )
    {
        return false;
    }
    return true;
}


template <class T>
inline bool perf_counter<T>::CollectData()
{
    m_Status = PdhCollectQueryData(m_hQuery);
    assert(m_Status == ERROR_SUCCESS);
    if ( m_Status != ERROR_SUCCESS )
    {
        return false;
    }

    // 得到当前计数器值
    m_Status = PdhGetFormattedCounterValue(*m_pCounter,
        m_DataType,
        &m_CounterType,
        &m_CurValue );

    if ( m_Status != ERROR_SUCCESS )
    {
        //flog << "error occurred while " << m_Name << " get counter data. " << std::endl;
        //flog << "PDH_STATUS : " << m_Status << std::endl;
        //flog << "GetLastError() : " << GetLastError() << std::endl;
        //flog << std::flush;
        return false;
    }
    else if(strcmp(typeid(T).name(), "long") == 0)
    {
        m_RecordValue = m_CurValue.longValue;
    }
    else if(strcmp(typeid(T).name(), "double") == 0)
    {
        m_RecordValue = m_CurValue.doubleValue;
    }
    else if(strcmp(typeid(T).name(), "__int64") == 0)
    {
        m_RecordValue = (T)(m_CurValue.largeValue);
    }
    else
    {
        return false;
    }

    //record the maximum value all the recorded data
    if (m_MaxValue < m_RecordValue)
    {
        m_MaxValue = m_RecordValue;
    }

    //record the collected data's count and summation of all the recorded data
    m_SumData = m_SumData + m_RecordValue;
    m_RecordedNum++;
    return true;

}


template <class T>
inline std::string	perf_counter<T>::GetName()
{
    return m_Name;
}


template <class T>
inline void perf_counter<T>::SetName(const std::string sName)
{
    m_Name = sName;
}


template <class T>
inline T	perf_counter<T>::GetMaxData()
{
    return m_MaxValue;
}


template <class T>
inline T	perf_counter<T>::GetRealtimeData()
{
    return m_RecordValue;
}


template <class T>
inline T	perf_counter<T>::GetSumData()
{
    return m_SumData;
}


template <class T>
inline long perf_counter<T>::GetRecordedCount()
{
    return m_RecordedNum;
}


template <class T>
inline bool perf_counter<T>::Uninitial()
{
    if (m_pCounter != NULL)
    {
        GlobalFree(m_pCounter);
        m_pCounter = NULL;

    }
    if (m_hQuery != NULL)
    {
        m_Status = PdhCloseQuery (m_hQuery);
        assert(m_Status == ERROR_SUCCESS);
        if (m_Status != ERROR_SUCCESS)
        {
            return false;
        }
        m_hQuery = NULL;
    }
    return true;
}


#endif // __PREF_COUNTER_HPP__
