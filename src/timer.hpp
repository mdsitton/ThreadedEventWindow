
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class FpsTimer
{
public:
    FpsTimer();
    double get_time() const;
    double get_smallest_precision();
    double tick();
    double get_timer_time() const;
    double get_current_time() const;
    double get_fps();

private:
    int m_frames;
    double m_currentTime;
    double m_previousTime;
    double m_startTime;
    double m_fpsPeriodStart;

    LARGE_INTEGER m_frequency;

};