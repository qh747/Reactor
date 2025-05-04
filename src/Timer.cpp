#include <chrono>
#include "Utils/Logger.h"
#include "Utils/Timer.h"

namespace Utils {

/** -------------------------------- TimerTask ------------------------------------- */

static std::atomic<uint64_t> TimerIdCounter {0};

TimerTask::TimerTask(Task cb, Timestamp expires, double intervalSec) 
    : m_id(TimerIdCounter++),
      m_cb(cb), 
      m_expires(expires), 
      m_intervalSec(intervalSec), 
      m_repeat(intervalSec > 0.0) {
}

bool TimerTask::executeTask() {
    if (nullptr == m_cb) {
        LOG_ERROR << "Execute timer task error. timer task callback invalid. id: " << m_id;
        return false;
    }

    m_cb();
    return true;
}

bool TimerTask::reset() {
    if (!m_repeat) {
        LOG_ERROR << "Reset timer task error. timer task not repeat. id: " << m_id;
    }

    auto intervalMs = std::chrono::duration<double, std::milli>(m_intervalSec);
    auto intervalDuration = std::chrono::duration_cast<std::chrono::system_clock::duration>(intervalMs);
    m_expires += intervalDuration;
    return true;
}

} // namespace Utils