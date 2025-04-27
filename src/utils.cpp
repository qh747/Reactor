#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include "common/utils.h"

namespace COMMON {

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
    struct tm tstruct;

    char buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y%m%d", &tstruct);
    return std::string(buf);
}

const char* StringHelper::GetFileName(const char* path) {
    const char* file = path;
    while (*path) {
        if (*path == '/' || *path == '\\') file = path + 1;
        path++;
    }
    return file;
}

std::string DirHelper::GetDirectory(const std::string& path) {
    std::size_t found = path.find_last_of("/\\");
    if (found != std::string::npos) {
        return path.substr(0, found);
    }

    return "";
}

bool DirHelper::CheckDirectoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info)) {
        return false;
    }

    return (info.st_mode & S_IFDIR);
}

bool DirHelper::CreateDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0755) == -1) {
        return false;
    }

    return true;
}

} // namespace COMMON