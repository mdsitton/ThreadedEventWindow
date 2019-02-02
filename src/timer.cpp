#include "timer.hpp"

FpsTimer::FpsTimer()
{
    QueryPerformanceFrequency(&m_frequency);
    m_frames = 0;
    m_currentTime = get_time();
    m_previousTime = m_currentTime;
    m_startTime = m_currentTime;
    m_fpsPeriodStart = m_currentTime;
}

double FpsTimer::get_time() const
{
    LARGE_INTEGER startingTime;
    QueryPerformanceCounter(&startingTime);
    return startingTime.QuadPart / static_cast<double>(m_frequency.QuadPart);
}


double FpsTimer::get_smallest_precision()
{
    return 1000.0 / m_frequency.QuadPart;
}

double FpsTimer::tick()
{
    m_frames++;
    m_previousTime = m_currentTime;
    m_currentTime = get_time();
    
    double delta = m_currentTime - m_previousTime;

    return delta;
}

double FpsTimer::get_timer_time() const
{
    return m_currentTime - m_startTime;
}

double FpsTimer::get_current_time() const
{
    return m_currentTime;
}


double FpsTimer::get_fps()
{
    if (m_fpsPeriodStart == m_currentTime) {
        return 0.0;
    }

    double fps = m_frames / (m_currentTime - m_fpsPeriodStart);
    m_fpsPeriodStart = m_currentTime;
    m_frames = 0;
    return fps;
}