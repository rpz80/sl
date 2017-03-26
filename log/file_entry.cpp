#include <stdio.h>
#include <stdexcept>
#include <memory>

#include <errno.h>

#include <log/format.h>
#include <log/file_entry.h>
#include <log/utils.h>

#if defined (_WIN32)
  #include <windows.h>
#endif
namespace sl {
namespace detail {

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

int64_t getFileSizeUnix(const std::string& fullPath) {
  struct stat st;
  if (stat(fullPath.data(), &st) != 0)
    throw std::runtime_error(
        sl::fmt("Get file size failed for the file %", fullPath));
  return st.st_size;
}

#elif defined (_WIN32)

int64_t getFileSizeWin(const std::string& fullPath) {
  HANDLE hFile = CreateFile(fullPath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == nullptr)
    throw std::runtime_error(sl::fmt("Get file size failed for the file %", 
                                     fullPath));
  LARGE_INTEGER result;
  if (!GetFileSizeEx(hFile, &result))
    throw std::runtime_error(sl::fmt("Get file size failed for the file %",
                                     fullPath));

  return result.QuadPart;
}
#endif

FileEntryPtr FileEntryFactory::create(const std::string& path, 
                                      const std::string& baseName,
                                      size_t index) {
  std::string fullFileName = getFullFileName(path, baseName, index);
  return FileEntryPtr(new FileEntry(fullFileName));
}

std::string FileEntryFactory::getFullFileName(const std::string& path,
                                              const std::string& baseName,
                                              size_t index) {
  if (index == 0) {
    return str::join(fs::join(path, baseName), kLogFileExtension);
  }

  return str::join(fs::join(path, baseName), 
                   std::to_string(index),
                   kLogFileExtension);
}

FileEntryList FileEntryFactory::getExistent(const std::string& path,
                                            const std::string& baseName) {
  FileEntryList result;
  fs::Dir dir(path);

  dir.forEachEntry([&baseName, &path, &result, this](const fs::Dir::Entry& entry) {
    if (entry.type == fs::Dir::Type::file &&
        fs::globMatch(entry.name.c_str(), (baseName + '*').c_str())) {
      result.push_back(FileEntryPtr(new FileEntry(str::join(fs::join(path, baseName),
                                                            kLogFileExtension))));
    }
  });

  return result;
}

FileEntry::FileEntry(const std::string& fullPath) : 
    m_fullPath(fullPath) {
}

void FileEntry::remove() {
  if (::remove(m_fullPath.data()) != 0) {
    throw std::runtime_error(sl::fmt("Delete file % failed", m_fullPath));
  }
}

void FileEntry::rename(const std::string& newName) {
  if (::rename(m_fullPath.data(), newName.data()) != 0) {
    throw std::runtime_error(sl::fmt("Rename file % to % failed", 
                                     m_fullPath, 
                                     newName));
  }
  m_fullPath = newName;
}

std::string FileEntry::name() const  {
  return m_fullPath;
}

int64_t FileEntry::size() const {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  return getFileSizeUnix(m_fullPath);
#elif defined (_WIN32)
  return getFileSizeWin(m_fullPath);
#endif
}

bool FileEntry::exists() const {
  struct stat st; 
  if (stat(m_fullPath.c_str(), &st) == 0)
    return true;
  return false;
}

FileStreamPtr FileEntry::open() {
  return FileStreamPtr(new FileStream(m_fullPath));
}

} // detail
} // sl
