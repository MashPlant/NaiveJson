#ifndef STREAM_HPP
#define STREAM_HPP

#include "Common.hpp"

namespace mp
{
struct StreamException : std::runtime_error
{
    template <typename Str>
    StreamException(Str &&str) : std::runtime_error(std::forward<Str>(str)) {}
};
struct Stream
{
    const char *cur = nullptr, *end = nullptr;
    Stream() = default;
    Stream(const char *src)
    {
        cur = src;
        end = cur + strlen(src);
    }
    void skip_blanks()
    {
        while (cur != end && (*cur == ' ' || *cur == '\n' || *cur == '\t' || *cur == '\r'))
            ++cur;
    }
    char peek() const
    {
#if defined(_MP_DEBUG)
        if (cur >= end)
            throw StreamException("Stream::peek: out of range");
#endif // _MP_DEBUG
        return *cur;
    }
    explicit operator bool() const
    {
        return cur != end;
    }
    char next()
    {
#if defined(_MP_DEBUG)
        if (cur >= end)
            throw StreamException("Stream::next: out of range");
#endif // _MP_DEBUG
        return *cur++;
    }
    void match(char ch)
    {
#if defined(_MP_DEBUG)
        if (cur >= end)
            throw StreamException("Stream::match: out of range");
#endif // _MP_DEBUG
        if (*cur != ch)
            throw StreamException(format("cur is %c, expect %c", *cur, ch));
        ++cur;
    }
    bool try_match(char ch)
    {
#if defined(_MP_DEBUG)
        if (cur >= end)
            throw StreamException("Stream::try_match: out of range");
#endif // _MP_DEBUG
        return *cur++ == ch;
    }
};
} // namespace mp

#endif // STREAM_HPP