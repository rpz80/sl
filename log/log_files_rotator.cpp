#include <log/log_files_rotator.h>

namespace sl {
namespace detail {

const std::string LogFileRotator::kLogFileExtension = ".log";
/*
LogFileRotator::LogFileRotator(const std::string& path, 
                               const std::string& fileNamePattern,
                               int64_t totalLimit,
                               int64_t fileLimit)
  : m_limitWatcher(totalLimit, fileLimit, this),
    m_path(path),
    m_fileNamePattern(fileNamePattern) {
  combineFullPath();
  openLogFile();
}

std::ostream& LogFileRotator::getCurrentFileStream() {
}

std::string LogFileRotator::getFullPath() const {
}

void LogFileRotator::combineFullPath() {

}

void LogFileRotator::openLogFile() {

}

int64_t LogFileRotator::clearNeeded(int64_t spaceToClear) {
}

bool LogFileRotator::nextFile() {
}
*/
}
}
