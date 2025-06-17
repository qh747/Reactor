#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <sys/stat.h>
#include "Utils/Utils.h"

namespace Utils {

std::string TimeHelper::GetCurrentTime() {
    auto now = std::chrono::system_clock::now();

    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm now_tm = *std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    return oss.str();
}

std::string TimeHelper::GetCurrentData() {
    time_t now = time(nullptr);
    struct tm t = {};

    char buf[80];
    t = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y%m%d", &t);
    return buf;
}

const char* StringHelper::GetFileName(const char* path) {
    const char* file = path;
    while (*path) {
        if (*path == '/' || *path == '\\') file = path + 1;
        path++;
    }
    return file;
}

std::string StringHelper::GetUniqueId(const std::string& localIp, uint16_t localPort, const std::string& peerIp, uint16_t peerPort) {
    std::stringstream ss;
    ss << localIp << ":" << localPort << "-" << peerIp << ":" << peerPort;
    return ss.str();
}

std::string StringHelper::EventTypeToString(Event_t event) {
    static const std::unordered_map<Event_t, std::string> EventTypeStrings = {
        {Event_t::EvTypeNone, "None"},
        {Event_t::EvTypeRead, "Read"},
        {Event_t::EvTypeWrite, "Write"},
        {Event_t::EvTypeClose, "Close"},
        {Event_t::EvTypeError, "Error"},
        {Event_t::EvTypeReadWrite, "Read | Write"},
        {Event_t::EvTypeReadClose, "Read | Close"},
        {Event_t::EvTypeReadError, "Read | Error"},
        {Event_t::EvTypeWriteClose, "Write | Close"},
        {Event_t::EvTypeWriteError, "Write | Error"},
        {Event_t::EvTypeCloseError, "Close | Error"},
        {Event_t::EvTypeReadWriteClose, "Read | Write | Close"},
        {Event_t::EvTypeReadWriteError, "Read | Write | Error"},
        {Event_t::EvTypeReadCloseError, "Close | Error"},
        {Event_t::EvTypeWriteCloseError, "Write | Close | Error"},
        {Event_t::EvTypeAll, "Read | Write | Close | Error"}};

    auto it = EventTypeStrings.find(event);
    if (it != EventTypeStrings.end()) {
        return it->second;
    }

    // 对于复合值或未知值，返回其数值表示
    return "EventType(" + std::to_string(static_cast<int>(event)) + ")";
}

std::string StringHelper::StateTypeToString(State_t state) {
    static const std::unordered_map<State_t, std::string> StateTypeStrings = {
        {State_t::StatePending, "Pending "},
        {State_t::StateInLoop, "InLoop"},
        {State_t::StateNotInLoop, "NotInLoop"}};

    auto it = StateTypeStrings.find(state);
    if (it != StateTypeStrings.end()) {
        return it->second;
    }

    // 如果传入的值不在映射中，返回其数值表示
    return "StateType(" + std::to_string(static_cast<int>(state)) + ")";
}

std::string StringHelper::PollerCtrlTypeToString(PollerCtrl_t type) {
    static const std::unordered_map<PollerCtrl_t, std::string> EpollCtrlTypeStrings = {
        {PollerCtrl_t::PollerAdd, "Add"},
        {PollerCtrl_t::PollerModify, "Modify"},
        {PollerCtrl_t::PollerRemove, "Remove"}};

    auto it = EpollCtrlTypeStrings.find(type);
    if (it != EpollCtrlTypeStrings.end()) {
        return it->second;
    }

    return "EpollCtrlType(" + std::to_string(static_cast<int>(type)) + ")";
}

std::string DirHelper::GetDirectory(const std::string& path) {
    std::size_t found = path.find_last_of("/\\");
    if (found != std::string::npos) {
        return path.substr(0, found);
    }

    return "";
}

bool DirHelper::CheckDirectoryExists(const std::string& path) {
    struct stat info = {};
    if (stat(path.c_str(), &info)) {
        return false;
    }

    return (info.st_mode & S_IFDIR);
}

bool DirHelper::CreateDirectory(const std::string& path) {
    return mkdir(path.c_str(), 0755) == -1;
}

Event_t EventHelper::ConvertToEventType(uint32_t type) {
    int result = 0;
    if (POLLIN == type || POLLPRI == type || POLLRDHUP == type) {
        result |= static_cast<int>(Event_t::EvTypeRead);
    }

    if (POLLOUT == type) {
        result |= static_cast<int>(Event_t::EvTypeWrite);
    }

    if (POLLHUP == type || POLLNVAL == type) {
        result |= static_cast<int>(Event_t::EvTypeClose);
    }

    if (POLLERR == type) {
        result |= static_cast<int>(Event_t::EvTypeError);
    }

    return static_cast<Event_t>(result);
}

} // namespace Utils