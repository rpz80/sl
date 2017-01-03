#include <stdexcept>
#include <cassert>
#include "log.h"

namespace sl {

namespace aux {
template<typename Exception>
void assertThrow(bool expr, const std::string& message) {
  assert(expr);
  if (!expr) {
    throw Exception(message);
  }
}

void assertThrowRuntime(bool expr, const std::string& message) {
  assertThrow<std::runtime_error>(expr, message);
}

void assertThrowDomain(bool expr, const std::string& message) {
  assertThrow<std::domain_error>(expr, message);
}

}

Logger::Logger() {}

Logger::SinkMapIterator Logger::getSinkById(int sinkId) {
  auto sinkIt = m_sinks.find(sinkId);
  aux::assertThrowRuntime(sinkIt != m_sinks.cend(), 
                          fmt("sinkId % not found", sinkId));
  return sinkIt;
}

void Logger::setDefaultSink(Level level, 
                            const std::string& fileName, 
                            OstreamPtr sinkStream, 
                            bool duplicateToStdout) {
  aux::assertThrowRuntime(
      static_cast<bool>(sinkStream), 
      fmt("% sink stream is null. fileName = %", 
          __FUNCTION__,
          fileName));
  std::lock_guard<sm::shared_mutex> lock(m_defaultSinkMutex);
  m_defaultSink = Sink(level, 
                       fileName,
                       std::move(sinkStream), 
                       duplicateToStdout);
}

void Logger::addSink(int sinkId, 
                     Level level, 
                     const std::string& fileName, 
                     OstreamPtr sinkStream, 
                     bool duplicateToStdout) {
  aux::assertThrowRuntime(
    static_cast<bool>(sinkStream), 
    fmt("% sink stream is null. fileName = %. sinkId = %",
        __FUNCTION__,
        fileName, 
        sinkId));
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  if (m_sinks.find(sinkId) != m_sinks.cend()) {
    throw std::runtime_error(
        fmt("% sink with this id (%) already exists", 
            __FUNCTION__, 
            sinkId));
  }
  bool emplaceResult = 
      m_sinks.emplace(sinkId, 
                      Sink(level, 
                           fileName, 
                           std::move(sinkStream), 
                           duplicateToStdout)).second;
  aux::assertThrowRuntime(
    emplaceResult, 
    fmt("% Emplace failed. fileName = %. sinkId = %",
        __FUNCTION__,
        fileName, 
        sinkId));
}

void Logger::setLevel(int sinkId, Level level) {
  std::lock_guard<sm::shared_mutex> lock(m_sinksMutex);
  auto sinkIt = getSinkById(sinkId);
  sinkIt->second.level = level;
}

void Logger::setDefaultLevel(Level level) {
  std::lock_guard<sm::shared_mutex> lock(m_defaultSinkMutex);
  m_defaultSink.level = level;
}

Level Logger::getLevel(int sinkId) const {
}

Level Logger::getDefaultLevel() const {
}

std::string Logger::getFileName(int sinkId) const {
}

std::string Logger::getDefaultFileName() const {
}

void Logger::addSink(int sinkId, 
                     const std::string& fileName,
                     Level level, 
                     bool duplicateToStdout) {
}

void Logger::removeSink(int sinkId) {
}

bool Logger::hasSink(int sinkId) const {
}

void Logger::setDefaultSink(const std::string& fileName, 
                            Level level,
                            bool duplicateToStdout) {
}

void Logger::removeDefaultSink() {
}

bool Logger::hasDefaultSink() const {
}

namespace detail {

void printTillSpecial(std::stringstream& out, 
                      const char** formatString) {
  for (; **formatString; ++*formatString) {
    if (**formatString == '\\') {
      if (*(*formatString + 1) && *(*formatString + 1) == '%') {
        ++*formatString;
        out << '%';
      } else {
        out << **formatString;
      }
    } else if (**formatString == '%') {
      break;
    } else {
      out << **formatString;
    }
  }
}

std::string fmt(std::stringstream& out,
                const char* formatString) {
  for (; *formatString; ++formatString) {
    out << *formatString;
  }
  return out.str();
}

}
}
