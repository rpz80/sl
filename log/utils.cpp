#include <vector>
#include <array>
#include <algorithm>
#include <log/utils.h>
#include <log/exception.h>
#include <log/format.h>

namespace sl {
namespace detail {
namespace errh {

void assertThrow(bool expr, const std::string& message) {
  //assert(expr);
  if (!expr) {
    throw LoggerException(message);
  }
}

}

namespace str {
StringRef::StringRef(const std::string& str) :
    m_size(str.size()) {
  if (m_size == 0) {
    m_data = nullptr;
  }
}

StringRef::StringRef(const std::string& str, size_t startPos, size_t size) :
    m_size(size) {
  if (startPos + size > str.size()) {
    throw std::runtime_error(
          sl::fmt("% Invalid startPos and/or size argumets for string %",
                  __FUNCTION__,
                  str));
  }
  m_data = &*str.cbegin() + startPos;
}

StringRef::StringRef(const char* data, size_t size) :
    m_size(size),
    m_data(data) {
}

size_t StringRef::size() const { return m_size; }
bool StringRef::empty() const { return m_size == 0; }

//char& StringRef::operator[](size_t index) {
//  return m_data[index];
//}

const char& StringRef::operator[](size_t index) const {
  return m_data[index];
}

std::string StringRef::toString() const {
  return std::string(m_data, m_size);
}

}

namespace fs {

std::string join(const std::string& subPath1, 
                 const std::string& subPath2) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  static const std::array<char, 1> kPathSeparators = {'/'};
#elif defined (_WIN32)
  static const std::array<char, 2> kPathSeparators = {'/', '\\'};
#endif

  bool path1EndsWithSeparator = 
      std::find_if(kPathSeparators.cbegin(),
                   kPathSeparators.cend(),
                   subPath1[subPath1.size()-1]) != std::end(kPathSeparators);

  if (path1EndsWithSeparator) {
    return detail::str::join(subPath1, subPath2);
  }

  return detail::str::join(subPath1, "/", subPath2); 
}

class OrCombiner {
  using OrGroup = std::vector<str::StringRef>;
  using OrGroups = std::vector<OrGroup>;

public:
  OrCombiner(const std::string& mask): m_mask(mask) {
    if (m_mask.empty())
      throw detail::UtilsException(sl::fmt("% Mask is empty"));

    parseMask();
  }

  template<typename F>
  void forEachMask(F f) {
    if (m_groups.empty()) {
      return;
    }
    getNext(0, f, "");
  }

private:
  void parseMask() {
    while (m_index < m_mask.size()) {
      if (m_mask[m_index] == '{') {
        addOrGroup();
      } else {
        addTrivialGroup();
      }
    }
  }

  void addOrGroup() {
    ++m_index;
    size_t startIndex = m_index;
    OrGroup newGroup;
    for (; m_index < m_mask.size(); ++m_index) {
      if (m_mask[m_index] == '}' && !newGroup.empty()) {
        m_groups.emplace_back(newGroup);
        return;
      } else if (m_mask[m_index] == ',') {
        if (m_index != startIndex) {
          newGroup.emplace_back(&m_mask[startIndex], m_index - startIndex);
          startIndex = m_index + 1;
        }
      }
    }
    throw std::runtime_error(sl::fmt("% Can't parse group for mask %",
                                     __FUNCTION__,
                                     m_mask));
  }

  void addTrivialGroup() {
    size_t startIndex = m_index;
    for (; m_index < m_mask.size(); ++m_index) {
      if (m_mask[m_index] == '{') {
        break;
      }
    }
    if (m_index != startIndex) {
      OrGroup newGroup;
      newGroup.emplace_back(&m_mask[startIndex], m_index - startIndex);
      m_groups.emplace_back(newGroup);
    }
  }

  template<typename F>
  void getNext(size_t groupIndex, F f, const std::string& prevMaskPart) {
    if (groupIndex >= m_groups.size()) {
      f(prevMaskPart);
      return;
    }
    for (size_t i = 0; i < m_groups[groupIndex]; ++i) {
      getNext(groupIndex + 1, f, prevMaskPart + m_groups[groupIndex][i].toString());
    }
  }

private:
  OrGroups m_groups;
  bool m_parseFailed = false;
  size_t m_index = 0;
  std::string& m_mask;
};

class MaskFit {
  enum class ParseState {
    asteriks,
    any,
    group,
    notGroup,
    symbol,
    error
  };

public:
  MaskFit(const std::string& fileName, const std::string& mask) : 
      m_fileName(fileName) {
    if (mask.empty()) {
      return;
    }

    if (fileName.empty()) {
      m_yes = true;
      return;
    }

    forEachOrString(mask, [this] (const std::string& orMask) {
      m_mask = orMask;
      applyMask();
    });
  }

  explicit operator bool() {
    return m_yes;
  }

private:
  void applyMask() {
    setState();

    switch (m_state) {
      case ParseState::error: return;
      case ParseState::asteriks: processAsteriks(); break;
      case ParseState::any: processAny(); break;
      case ParseState::group: processGroup(); break;
      case ParseState::notGroup: processNotGroup(); break;
      case ParseState::symbol: processSymbol(); break;
    }
  }

  template<typename F>
  void forEachOrString(const std::string& originalMask, F f) {
    OrCombiner orCombiner(originalMask);
    orCombiner.forEachMask(f);
  }

  void parseEscape() {
    ++m_maskPos;
    if (m_maskPos == m_mask.size()) {
      m_state = ParseState::error;
    } else {
      m_state = ParseState::symbol;
    }
  }

  void parseGroup() {
    ++m_maskPos;
    m_group.clear();

    if (m_maskPos == m_mask.size()) {
      m_state = ParseState::error;
      return;
    }

    bool notGroup = false;
    if (m_mask[m_maskPos] == '!') {
      ++m_maskPos;
      notGroup = true;
    }

    for (; m_maskPos < m_mask.size(); ++m_maskPos) {
      if (m_mask[m_maskPos] == ']') {
        m_state = notGroup ? ParseState::notGroup : ParseState::group;
        ++m_maskPos;
        return;
      } else {
        m_group.push_back(m_mask[m_maskPos]);
      }
    }

    m_state = ParseState::error;
  }

  void setState() {
    switch (mask[m_maskPos]) {
      case '*': m_state = ParseState::asteriks; break;
      case '?': m_state = ParseState::any; break;
      case '[': parseGroup(); break;
      case '\\': parseEscape(); break;
      default: m_state = ParseState::symbol; break;
    }
    ++m_maskPos;
  }

private:
  size_t m_namePos = 0;
  size_t m_maskPos = 0;
  bool m_yes = false;
  ParseState m_state; 
  std::string m_group;
  const std::string& m_fileName;
  std::string m_mask;
};

bool maskFits(const std::string& fileName, const std::string& mask) {
  return static_cast<bool>(MaskFit(fileName, mask));
}

}

}
}

