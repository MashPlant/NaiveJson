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
        const char *content;
        size_type size;
        size_type hash;
        static size_type hash_code(const char *src, size_type size)
        {
            size_type hash{};
            for (size_type i = 0; i < size; ++i)
                hash = hash * P + *src++;
            return hash;
        }
        static String from_cstr(const char *src)
        {
            String ret = {.content = src, .size = (size_type)strlen(src)};
            ret.hash = hash_code(src, ret.size);
            return ret;
        }
        static String from_cstr(const char *src, size_type size)
        {
            return {.content = src, .size = size, .hash = hash_code(src, size)};
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
            for (size_type i = 0; i < str.size; ++i)
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
        explicit Array(size_type capacity)
            : size(0), capacity(capacity), content((Value *)malloc(sizeof(Value) * capacity)) {}
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
        bool bo;
        int64_t i64;
        double f64;
        String str;
        Array arr;
        Object *obj;
    } data;
    enum
    {
        null_flag,
        bool_flag,
        i64_flag,
        f64_flag,
        str_flag,
        arr_flag,
        obj_flag
    } type = null_flag;
    decltype(auto) get_bool()
    {
        assert(type == bool_flag);
        return (data.bo);
    }
    decltype(auto) get_i64()
    {
        assert(type == i64_flag);
        return (data.i64);
    }
    decltype(auto) get_f64()
    {
        assert(type == f64_flag);
        return (data.f64);
    }
    decltype(auto) get_str()
    {
        assert(type == str_flag);
        return (data.str);
    }
    decltype(auto) get_arr()
    {
        assert(type == arr_flag);
        return (data.arr);
    }
    decltype(auto) get_obj()
    {
        assert(type == obj_flag);
        return (data.obj);
    }
    Value() = default;
    Value(decltype(data.bo) bo) : type(bool_flag) { data.bo = bo; }
    Value(decltype(data.i64) i64) : type(i64_flag) { data.i64 = i64; }
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
    Value &operator[](const char *name)
    {
        auto it = dom.find(String::from_cstr(name));
        assert(it != dom.end());
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