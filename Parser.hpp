#ifndef PARSER_HPP
#define PARSER_HPP

#include "Common.hpp"
#include "Object.hpp"
#include "StringPool.hpp"
#include "Execption.hpp"

namespace mp
{
struct StreamIt
{
    const char *cur, *end;
    StreamIt() = default;
    explicit StreamIt(const char *json) : cur(json), end(cur + strlen(json)) {}
    char operator*()
    {
        //assert(cur < end);
        return *cur;
    }
    explicit operator bool() { return cur < end; }
    operator const char *&() { return cur; }
    void skip_blanks()
    {
        while (cur != end && (*cur == ' ' || *cur == '\n' || *cur == '\t' || *cur == '\r'))
            ++cur;
    }
};
struct Parser
{
    StreamIt it;
    Exception ex;
    Object::pool_pointer pool;
    Object parse(const char *json)
    {
        it = StreamIt(json);
        ex = Exception(json);
        pool.reset(new StringPool);
        Object ret;
        ret.pool = pool;
        parse_object(ret);
        if (!ex.ok())
        {
            std::cerr << ex.msg << std::endl;
            exit(1);
        }
        return ret;
    }
    // expect and inc
    void expect(char ch)
    {
        if (*it != ch)
            ex.errorf(it, "expect %c, got %c", ch, *it);
        ++it;
    }
    void expect(const char *str)
    {
        const char *where = nullptr;
        for (auto cur = str; *cur; ++cur)
        {
            if (*cur != *it)
                where = cur;
            ++it; // do not put expression with other effect in `assert`
        }
        if (where)
            ex.errorf(where, "expect %s, got %c", str, *where);
    }
    void parse_object(Object &obj)
    {
        it.skip_blanks();
        expect('{');
        String str;
        Value val;
        while (it)
        {
            it.skip_blanks();
            if (*it == '}')
                break;
            parse_string(str);
            it.skip_blanks();
            expect(':');
            it.skip_blanks();
            parse_value(val);
            obj.insert(str, std::move(val));
            it.skip_blanks();
            if (*it != ',') // optional last comma
                break;
            else
                ++it;
        }
        expect('}');
    }
    void parse_value(Value &val)
    {
        if (*it == '\"')
        {
            val.type = Value::str_flag;
            parse_string(val.get_str());
        }
        else if (*it == '{')
        {
            auto obj = std::make_unique<Object>(); // in case an exception is thrown
            obj->pool = pool;
            parse_object(*obj);
            val.type = Value::obj_flag;
            val.get_obj() = obj.release();
        }
        else if (*it == '[')
        {
            val.type = Value::arr_flag;
            parse_array(val.get_arr());
        }
        else if (isdigit(*it) || *it == '-')
        {
            parse_number(val);
        }
        else if (*it == 't' || *it == 'f')
        {
            val.type = Value::bool_flag;
            parse_bool(val.get_bool());
        }
        else if (*it == 'n')
        {
            expect("null");
            val.type = Value::null_flag;
        }
        else
            ex.errorf(it, "unexpected char: %c", *it);
    }
    void parse_number(Value &val)
    {
        char *tmp;
        double res = strtod(it, &tmp);
        if (tmp == it)
            ex.errorf(it, "wrong number format %c", *it);
        static_cast<const char *&>(it) = tmp;
        if (res == (int64_t)res)
        {
            val.type = Value::i64_flag;
            val.get_i64() = (int64_t)res;
        }
        else
        {
            val.type = Value::f64_flag;
            val.get_f64() = res;
        }
    }
    void parse_string(String &str)
    {
        static StringBuilder builder;
        const static auto escape_map = []() {
            std::array<char, 256> ret;
            ret.fill(-1);
            ret[+'0'] = '\0', ret[+'\"'] = '\"', ret[+'\\'] = '\\', ret[+'n'] = '\n', ret[+'t'] = '\t', ret[+'r'] = '\r';
            return ret;
        }();
        builder.clear();
        expect('\"');
        while (*it != '\"')
        {
            if (*it == '\\')
            {
                ++it;
                if (escape_map[*it] == -1)
                    ex.errorf(it, "unexpected excape char \\%c", *it);
                builder.push_back(escape_map[*it++]);
            }
            else
                builder.push_back(*it++);
        }
        expect('\"');
        str = String::from_cstr(builder.to_string(*pool));
    }
    void parse_array(Array &arr)
    {
        arr = Array(16);
        Value val;
        expect('[');
        while (true)
        {
            it.skip_blanks();
            if (*it == ']')
                break;
            parse_value(val);
            arr.push_back(std::move(val));
            it.skip_blanks();
            if (*it != ',') // optional last comma
                break;
            else
                ++it;
        }
        expect(']');
    }
    void parse_bool(bool &bo)
    {
        if (*it == 't')
        {
            expect("true");
            bo = true;
        }
        else
        {
            expect("false");
            bo = false;
        }
    }
};
} // namespace mp

#endif // PARSER_HPP