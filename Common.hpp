#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cassert>

#include <iostream>
#include <utility>
#include <type_traits>
#include <memory>
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace mp
{
using size_type = uint32_t;

template <typename... Args>
inline std::string format(const char *fmt, Args &&... args)
{
    int len = snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
    std::string ret(len + 1, '\0'); // for '\0'
    snprintf(&ret[0], len + 1, fmt, std::forward<Args>(args)...);
    return ret;
}

#define _MP_ASSERT(cond) assert((cond))
#define _MP_DISABLE_COPY(class_name)         \
    class_name(const class_name &) = delete; \
    class_name &operator=(const class_name &) = delete;

#define _MP_LIKELY(x) __builtin_expect(!!(x), 1)
#define _MP_UNLIKELY(x) __builtin_expect(!!(x), 0)

} // namespace mp

#endif // COMMON_HPP
