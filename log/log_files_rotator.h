#pragma once

namespace sl {
namespace detail {

class LogFileRotator : public RotationWatcherHandler {
  static const std::string kLogFileExtension;
public:
  LogFileRotator(const std::string& path, 
                 const std::string& fileNamePattern);

  std::ostream& getCurrentFileStream();

protected:
  std::string getFullPath() const;

private:
  void combineFullPath();
  void openLogFile();

  virtual int64_t clearNeeded(int64_t spaceToClear) override;
  virtual bool nextFile() override;

private:
  RotationLimitWatcher m_limitWatcher;
  std::string m_path;
  std::string m_fileNamePattern;
  std::string m_fullPath;
  OstreamPtr m_currentFile;
  FileInfoVector m_fileInfos;
};

}
}
