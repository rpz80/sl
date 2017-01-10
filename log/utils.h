#pragma once

namespace sl {
namespace detail {

namespace errh {
void assertThrow(bool expr, const std::string& message); 
}

namespace fs {
std::string join(const std::string& subPath1, 
                 const std::string& subPath2);
}

namespace str {
template<typename... Args>
std::string join(Args&&... args) {
}
}

}
}
