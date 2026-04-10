#include "performancetimer.h"
#pragma comment(lib,"Winmm.lib")

#ifdef __MINGW32__
#define TIME_KILL_SYNCHRONOUS 0x0100
#endif

void CALLBACK PeriodCycle(uint timerId,uint,DWORD_PTR user,DWORD_PTR,DWORD_PTR)
{
    PerformanceTimer *t=reinterpret_cast<PerformanceTimer *>(user);
    emit t->timeout();
}

PerformanceTimer::PerformanceTimer(QObject *parent) : QObject(parent)
{
    m_id=0;
}

PerformanceTimer::~PerformanceTimer()
{
    stop();
}
void PerformanceTimer::start(int timeInterval)      
{                 
    m_id=timeSetEvent(timeInterval,1,PeriodCycle,(DWORD_PTR)this,TIME_CALLBACK_FUNCTION|TIME_PERIODIC|TIME_KILL_SYNCHRONOUS);
}

void PerformanceTimer::stop(void)
{
    if(m_id)
    {
        timeKillEvent(m_id);
        m_id=0;
     }
}
