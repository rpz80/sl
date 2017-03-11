#include <stdio.h>
#include <log/file_stream.h>
#include <log/format.h>

namespace sl {
namespace detail {

FileStream::FileStream(const std::string& fileName) 
  : m_fileName(fileName),
    m_stream(nullptr) 
{
  open();
}

FileStream::~FileStream() {
  close();
}

void FileStream::open() {
  if (m_stream != nullptr) {
    throw std::runtime_error(sl::fmt("FileStream: file % already opened", 
                                     m_fileName));
  }

  m_stream = fopen(m_fileName.c_str(), "ab");
  if (m_stream == nullptr) {
    throw std::runtime_error(sl::fmt("FileStream: file % open failed", 
                                     m_fileName));
  }

  if (setvbuf(m_stream, NULL, _IONBF, 0) != 0) {
    fclose(m_stream);
    throw std::runtime_error(sl::fmt("FileStream: file % setvbuf failed", 
                                     m_fileName));
  }
}

void FileStream::close() {
  if (m_stream) {
    fflush(m_stream);
    fclose(m_stream);
    m_stream = nullptr;
  }
}

void FileStream::write(const void* data, size_t size) {
  auto bytesWritten = fwrite(data, size, 1, m_stream) * size;
  if (bytesWritten != size) {
    throw std::runtime_error(sl::fmt("FileStream: file % write failed, bytesWritten = %", 
                                     m_fileName, bytesWritten));
  }
}

bool FileStream::isOpened() const {
  return m_stream != nullptr;
}

}
}