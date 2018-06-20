#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "Common.hpp"
#include "StringPool.hpp"

namespace mp
{
struct Object;

struct Value
{
    _MP_DISABLE_COPY(Value)
    struct String // const string view
    {
        const static size_type P = 19260817;
        size_type size;
        size_type hash;
        const char *content;
        String() = default; // won't be properly constructed
        String(const char *src)
        {
            content = src;
            size = 0, hash = 0;
            for (const char *cur = src; *cur; ++cur, ++size)
                hash = hash * P + *cur;
        }
        bool operator==(String rhs) const
        {
            if (size != rhs.size || hash != rhs.hash)
                return false;
            return strcmp(content, rhs.content) == 0;
        }
        char operator[](size_type index) const { return content[index]; }
        template <typename OS>
        friend decltype(auto) operator<<(OS &&os, String str)
        {
            for (int i = 0; i < str.size; ++i)
                os << str.content[i];
            return std::forward<OS>(os);
        }
    };
    struct Array
    {
        size_type size;
        size_type capacity;
        Value *content;

        static size_type size_to_cap(size_type size)
        {
            size_type cap = 1;
            while (cap < size)
                cap <<= 1;
            return cap;
        }
        Array() = default; // won't be properly constructed
        Array(size_type size)
        {
            this->size = size;
            capacity = size_to_cap(size);
            content = (Value *)malloc(sizeof(Value) * capacity);
            for (size_type i = 0; i < size; ++i)
                new (content + i) Value();
        }
        Array &push_back(Value &&val)
        {
            if (size >= capacity)
                content = (Value *)realloc(content, capacity <<= 1);
            new (content + size++) Value(std::move(val));
            return *this;
        }
        Value &operator[](size_type index) { return content[index]; }
        const Value &operator[](size_type index) const { return content[index]; }
    };
    union {
        int32_t i32;
        uint32_t u32;
        float f32;
        int64_t i64;
        uint64_t u64;
        double f64;
        String str;
        Array arr;
        Object *obj;
    } data;
    enum
    {
        null_flag,
        i32_flag,
        u32_flag,
        f32_flag,
        i64_flag,
        u64_flag,
        f64_flag,
        str_flag,
        arr_flag,
        obj_flag
    } type = null_flag;
    decltype(auto) get_i32() { return (data.i32); }
    decltype(auto) get_u32() { return (data.u32); }
    decltype(auto) get_f32() { return (data.f32); }
    decltype(auto) get_i64() { return (data.i64); }
    decltype(auto) get_u64() { return (data.u64); }
    decltype(auto) get_f64() { return (data.f64); }
    decltype(auto) get_str() { return (data.str); }
    decltype(auto) get_arr() { return (data.arr); }
    decltype(auto) get_obj() { return (data.obj); }
    Value() = default;
    Value(decltype(data.i32) i32) : type(i32_flag) { data.i32 = i32; }
    Value(decltype(data.u32) u32) : type(u32_flag) { data.u32 = u32; }
    Value(decltype(data.f32) f32) : type(f32_flag) { data.f32 = f32; }
    Value(decltype(data.i64) i64) : type(i64_flag) { data.i64 = i64; }
    Value(decltype(data.u64) u64) : type(u64_flag) { data.u64 = u64; }
    Value(decltype(data.f64) f64) : type(f64_flag) { data.f64 = f64; }
    Value(decltype(data.str) str) : type(str_flag) { data.str = str; }
    Value(decltype(data.arr) arr) : type(arr_flag) { data.arr = arr; }
    Value(decltype(data.obj) obj) : type(obj_flag) { data.obj = obj; }
    void swap(Value &rhs) noexcept
    {
        std::swap(data, rhs.data);
        std::swap(type, rhs.type);
    }
    Value(Value &&rhs) noexcept
    {
        memcpy(this, &rhs, sizeof(Value));
        memset(&rhs, 0, sizeof(Value));
    }
    Value &operator=(Value &&rhs) noexcept
    {
        Value tmp = std::move(rhs);
        swap(tmp);
        return *this;
    }
    ~Value(); // impl under declaration of Object
};
using String = Value::String;
using Array = Value::Array;
struct Object
{
    struct StringHash
    {
        size_t operator()(String str) const { return ((size_t)str.size << 32ull) | str.hash; }
    };
    using pool_pointer = std::shared_ptr<StringPool>;
    std::unordered_map<String, Value, StringHash> dom;
    pool_pointer pool;
    Value &operator[](String name)
    {
        auto it = dom.find(name);
        _MP_ASSERT(it != dom.end());
        return it->second;
    }
    bool insert(String name, Value &&val)
    {
        auto status = dom.insert({name, std::move(val)});
        return status.second;
    }
};
inline Value::~Value()
{
    if (type == arr_flag)
        for (Value *cur = data.arr.content, *end = cur + data.arr.size; cur != end; ++cur)
            cur->~Value();
    else if (type == obj_flag)
        delete data.obj;
}
} // namespace mp

#endif // OBJECT_HPP