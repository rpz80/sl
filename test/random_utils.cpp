#include <stdexcept>
#include "random_utils.h"

#include <log/format.h>

RandomData::RandomData(size_t minSize, size_t maxSize) 
  : m_gen(m_device()),
    m_minSize(minSize),
    m_maxSize(maxSize) 
{
  if (m_minSize >= m_maxSize)
    throw std::runtime_error(sl::fmt("RandomData: minSize >= maxSize"));
}

std::string RandomData::operator()(size_t fixedSize) {
  std::string result;
  result.reserve(fixedSize);

  for (size_t i = 0; i < fixedSize; ++i) {
    result.push_back(std::uniform_int_distribution<>(97, 122)(m_gen));
  }

  return result;
}

std::string RandomData::operator()() {
  return operator()(size());
}

size_t RandomData::size() {
  return std::uniform_int_distribution<size_t>(m_minSize, m_maxSize)(m_gen);
}