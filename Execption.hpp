#ifndef EXECPTION_HPP
#define EXECPTION_HPP

#include "Common.hpp"

namespace mp
{
struct Exception
{
    const char *begin;
    std::string msg;

    Exception() = default;
    explicit Exception(const char *begin) : begin(begin) {}
    bool ok()
    {
        return msg.empty();
    }
    template <typename... Args>
    void errorf(const char *cur, const char *fmt, Args &&... args)
    {
        const char *last{}, *next{};
        int line{}, col{};
        for (const char *it = begin; it != cur; ++it)
        {
            if (*it == '\n')
                ++line, col = 0, last = it + 1, next = last;
            else
                ++col;
        }
        for (const char *it = cur; *it; ++it)
            if (*it == '\n')
            {
                next = it;
                break;
            }
        msg += format("in line %d, column %d:\n", line, col);
        msg += std::string(last, next);
        msg += format(fmt, args...);
    }
};
} // namespace mp

#endif // EXECPTION_HPP