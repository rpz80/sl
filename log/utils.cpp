#include <log/utils.h>

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

namespace fs {

std::string join(const std::string& subPath1, 
                 const std::string& subPath2) {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  static const std::array<1, char> kPathSeparators = {"/"};
#elif defined (_WIN32)
  static const std::array<2, char> kPathSeparators = {"/", "\\"};
#endif

  bool path1EndsWithSeparator = 
      std::find(std::begin(kPathSeparators), 
                std::end(kPathSeparators),
                subPath1[subPath1.size()-1]) != std::end(kPathSeparators);

  if (path1EndsWithSeparator) {
    return detail::str::jon(subPath1, subPath2);
  }

  return detail::str::join(subPath1, "/", subPath2); 
}

class MaskFit {
  enum class ParseState {
    asteriks,
    any,
    group,
    notGroup,
    or_,
    symbol,
    error
  };

public:
  MaskFit(const std::string& fileName, const std::string& mask) : 
      m_fileName(fileName),
      m_mask(mask) {
    if (mask.empty()) {
      return;
    }

    if (fileName.empty()) {
      m_yes = true;
      return;
    }
      
    applyMask();
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
      case ParseState::or_: processOr(); break;
      case ParseState::symbol: processSymbol() break;
    }
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

  void parseOr() {
    ++m_maskPos;
    m_orStrings.clear();

    if (m_maskPos == m_mask.size()) {
      m_state = ParseState::error;
      return;
    }

    m_orStrings.emplace_back();

    for (; m_maskPos < m_mask.size(); ++ m_maskPos) {
      if (m_mask[m_maskPos] == '}') {
        m_state = ParseState::or_;
        ++m_maskPos;
        return;
      } 
      if (m_mask[m_maskPos] == ',') {
        m_orStrings.emplace_back();
      } else {
        m_orStrings[m_orStrings.size() - 1].emplace_back(m_mask[m_maskPos]);
      }
    }

    m_state = ParseState::error;
  }

  void setState() {
    switch (mask[m_maskPos]) {
      case '*': m_state = ParseState::asteriks; break;
      case '?': m_state = ParseState::any; break;
      case '[': parseGroup(); break;
      case '{': parseOr(); break;
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
  std::vector<std::string> m_orStrings;
  const std::string& m_fileName;
  const std::string& m_mask;
};

bool maskFits(const std::string& fileName, const std::string& mask) {
  return static_cast<bool>(MaskFit(fileName, mask));
}

}

}
}

