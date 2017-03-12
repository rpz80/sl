# Simple cpp logger (sl)
Simple in use c++ logger. Features:
* Multiple sinks 
* Log rotation
* No dependencies (except small [shared mutex library](https://github.com/rpz80/sm))
* Thread safety
* Very simple format string
* Five standard log levels (easily customizable though)

## Usage

```c++
#include <sl/log/log.h>

enum LogSinks {
  DB_LOG,
  NET_LOG
};

int main() {
  auto& logger = sl::Logger::getLogger();
  logger.setDefaultSink(
    "/var/log/myApp",   // log directory
    "log_file",         // log file base name
    sl::Level::info,    // lowest log level (debug < info < warning < error < critical)
    50 * 1024 * 1024ll, // total space limit in bytes
    1 * 1024 * 1024ll,  // space limit per file
    false);             // duplicate log messages to stdout

  // optional sinks
  logger.addSink(
    DB_LOG,                // Sink id
    "/var/log/myApp/db",   // log directory
    "log_file",            // log file base name
    sl::Level::error,      // lowest log level (debug < info < warning < error < critical)
    10 * 1024 * 1024ll,    // total space limit in bytes
    1 * 1024 * 1024ll,     // space limit per file
    false);                // duplicate log messages to stdout
    
    // log to default sink
    LOG(sl::Level::info, "% %st %", "my", 1, "log message"); 
    // output: "2017-03-11 22:10:59.129     INFO 0x7fffa2ba73c0 my 1st log message"
    
    // log to optional sink
    LOG_S(DB_LOG, sl::Level::error, "% %st %", "my", 1, "DB log message");
    // output: "2017-03-11 22:10:59.129     ERROR 0x7fffa2ba73c0 my 1st DB log message"
}
```
