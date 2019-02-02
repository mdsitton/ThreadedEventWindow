#include <atomic>
#include <cstdint>

template<typename T, int16_t queueSize>
class QueueSPSC
{
public:
    QueueSPSC()
    :m_head(0), m_tail(0)
    {}

    bool push(const T& item)
    {   
        const int16_t currentHead = m_head.load();
        const int16_t nextHead = (currentHead + 1) % queueSize;

        if (nextHead != m_tail.load())
        {
            m_queueBuffer[currentHead] = item;
            m_head.store(nextHead);
            return true;
        }

        return false;
    }

    bool pop(T& item)
    {
        const int16_t currentTail = m_tail.load();

        if (currentTail != m_head.load()) {

            item = m_queueBuffer[currentTail];
            m_tail.store((currentTail + 1) % queueSize);
            return true;
        }
        return false;
    }
    int16_t queue_length() const
    {
        return (m_head.load() - m_tail.load() + queueSize) % queueSize;
    }

    bool is_lock_free() const
    {
        return m_head.is_lock_free() && m_tail.is_lock_free();
    }
private:
    std::atomic<int16_t> m_head;
    T m_queueBuffer[queueSize];
    std::atomic<int16_t> m_tail;
};