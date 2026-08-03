#ifndef SNOWFLAKE_H
#define SNOWFLAKE_H

#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <mutex>

class SnowflakeNonlock
{
public:
    void lock() {}
    void unlock() {}
};

template<int64_t Twepoch = 0, typename Lock = SnowflakeNonlock>
class Snowflake
{
    using lock_type = Lock;
    static constexpr int64_t TWEPOCH = Twepoch;
    static constexpr int64_t WORKER_ID_BITS = 5L;
    static constexpr int64_t DATACENTER_ID_BITS = 5L;
    static constexpr int64_t MAX_WORKER_ID = (1 << WORKER_ID_BITS) - 1;
    static constexpr int64_t MAX_DATACENTER_ID = (1 << DATACENTER_ID_BITS) - 1;
    static constexpr int64_t SEQUENCE_BITS = 12L;
    static constexpr int64_t WORKER_ID_SHIFT = SEQUENCE_BITS;
    static constexpr int64_t DATACENTER_ID_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS;
    static constexpr int64_t TIMESTAMP_LEFT_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS + DATACENTER_ID_BITS;
    static constexpr int64_t SEQUENCE_MASK = (1 << SEQUENCE_BITS) - 1;

    using time_point = std::chrono::time_point<std::chrono::steady_clock>;

    time_point m_start_time_point = std::chrono::steady_clock::now();
    int64_t m_start_millsecond = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    int64_t m_last_timestamp = -1;
    int64_t m_workerid = 0;
    int64_t m_datacenterid = 0;
    int64_t m_sequence = 0;
    lock_type m_lock;
public:
    Snowflake() = default;
    Snowflake(const Snowflake&) = delete;
    Snowflake& operator=(const Snowflake&) = delete;

    void init(int64_t workerid, int64_t datacenterid)
    {
        if (workerid > MAX_WORKER_ID || workerid < 0) {
            throw std::runtime_error("worker Id can't be greater than 31 or less than 0");
        }

        if (datacenterid > MAX_DATACENTER_ID || datacenterid < 0) {
            throw std::runtime_error("datacenter Id can't be greater than 31 or less than 0");
        }

        m_workerid = workerid;
        m_datacenterid = datacenterid;
    }

    int64_t nextId()
    {
        std::lock_guard<lock_type> lock(m_lock);
        //std::chrono::steady_clock  cannot decrease as physical time moves forward
        auto timestamp = millsecond();
        if (m_last_timestamp == timestamp) {
            m_sequence = (m_sequence + 1)&SEQUENCE_MASK;
            if (m_sequence == 0) {
                timestamp = waitNextMillis(m_last_timestamp);
            }
        }
        else {
            m_sequence = 0;
        }

        m_last_timestamp = timestamp;

        return ((timestamp - TWEPOCH) << TIMESTAMP_LEFT_SHIFT)
            | (m_datacenterid << DATACENTER_ID_SHIFT)
            | (m_workerid << WORKER_ID_SHIFT)
            | m_sequence;
    }

private:
    int64_t millsecond() const noexcept
    {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time_point);
        return m_start_millsecond + diff.count();
    }

    int64_t waitNextMillis(int64_t last) const noexcept
    {
        auto timestamp = millsecond();
        while (timestamp <= last)
        {
            timestamp = millsecond();
        }
        return timestamp;
    }
};

#endif // SNOWFLAKE_H
