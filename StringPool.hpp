#ifndef STRINGPOOL_HPP
#define STRINGPOOL_HPP

#include <vector>
#include "Common.hpp"

namespace mp
{
struct StringPool
{
    const static size_type max_len = 4096;
    struct Block
    {
        union {
            char chs[sizeof(void *)];
            void *next /*= nullptr*/;
        } slots[1 + max_len / sizeof(void *)];
        // 1 for next, not for '\0'
    };
    Block *pool = (Block *)calloc(1, sizeof(Block));
    size_type cur = sizeof(void *);

    const char *allocate(const char *src)
    {
        size_type size = strlen(src);
        if (cur + size >= sizeof(Block))
        {
            Block *tmp = pool;
            if (_MP_UNLIKELY(size >= max_len)) // then size + sizeof(void *) + 1 > sizeof(Block)
                pool = (Block *)calloc(1, size + sizeof(void *) + 1);
            else
                pool = (Block *)calloc(1, sizeof(Block));
            pool->slots[0].next = tmp;
            cur = sizeof(void *);
        }
        // now cur + size + 1 <= the empty space
        char *ret = (char *)pool + cur;
        strcpy(ret, src);
        cur += size + 1;
        return ret;
    }
    ~StringPool()
    {
        while (pool)
        {
            Block *tmp = (Block *)pool->slots[0].next;
            free(pool);
            pool = tmp;
        }
    }
};
struct StringBuilder : std::vector<char>
{
    using std::vector<char>::vector;
    const char *to_string(StringPool &pool)
    {
        push_back('\0');
        const char *ret = pool.allocate(data());
        pop_back();
        return ret;
    }
};
} // namespace mp

#endif // STRINGPOOL_HPP