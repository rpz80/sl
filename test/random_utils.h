#pragma once

#include <random>
#include <string>

class RandomData {
public:
  RandomData(size_t minSize, size_t maxSize);
  std::string operator()(size_t fixedSize);
  std::string operator()();

private:
  size_t size();

private:
  size_t m_minSize;
  size_t m_maxSize;
  std::random_device m_device;
  std::mt19937 m_gen;
};