#ifndef PARSER_HPP
#define PARSER_HPP

#include "Common.hpp"
#include "Object.hpp"
#include "StringPool.hpp"
#include "Stream.hpp"

namespace mp
{
struct ParserExecption : std::runtime_error
{
    template <typename Str>
    ParserExecption(Str &&str) : std::runtime_error(std::forward<Str>(str)) {}
};
struct Parser
{
    Stream s;
    Object::pool_pointer pool;
    Object parse(const char *json)
    {
        s = json;
        pool.reset(new StringPool);
        Object ret;
        ret.pool = pool;
        parse_object(ret);
        return ret;
    }
    void parse_object(Object &obj)
    {
        s.skip_blanks();
        s.match('{');
        String str;
        Value val;
        while (true)
        {
            s.skip_blanks();
            if (s.peek() == '}')
                break;
            parse_string(str);
            s.skip_blanks();
            s.match(':');
            s.skip_blanks();
            parse_value(val);
            obj.insert(str, std::move(val));
            s.skip_blanks();
            if (s.peek() != ',') // optional last comma
                break;
            else
                s.next();
        }
        s.match('}');
    }
    void parse_value(Value &val)
    {
        if (s.peek() == '\"')
        {
            parse_string(val.get_str());
            val.type = Value::str_flag;
        }
        else if (s.peek() == '{')
        {
            auto obj = std::make_unique<Object>(); // in case an exception is thrown
            obj->pool = pool;
            parse_object(*obj);
            val.get_obj() = obj.release();
            val.type = Value::obj_flag;
        }
        else if (s.peek() == '[')
        {
            parse_array(val.get_arr());
            val.type = Value::arr_flag;
        }
        else if (isdigit(s.peek()) || s.peek() == '-')
        {
            parse_number(val);
        }
        else if (s.peek() == 't' || s.peek() == 'f')
        {
            parse_bool(val.get_bool());
            val.type = Value::bool_flag;
        }
        else
            throw ParserExecption(format("unexpected char: %c", s.peek()));
    }
    void parse_number(Value &val)
    {
        double res = s.get_number();
        if (res == (int64_t)res)
        {
            val.get_i64() = (int64_t)res;
            val.type = Value::i64_flag;
        }
        else
        {
            val.get_f64() = res;
            val.type = Value::f64_flag;
        }
    }
    void parse_string(String &str)
    {
        static StringBuilder builder;
        const static auto escape_map = []() {
            std::array<char, 256> ret;
            ret[+'\"'] = '\"', ret[+'\\'] = '\\', ret[+'n'] = '\n', ret[+'t'] = '\t', ret[+'r'] = '\r';
            return ret;
        }();
        builder.clear();
        s.match('\"');
        while (s.peek() != '\"')
        {
            if (_MP_UNLIKELY(s.peek() == '\\'))
            {
                s.next();
                if (!escape_map[s.peek()])
                    throw ParserExecption(format("unexpected escape character: \\%c", s.peek()));
                builder.push_back(escape_map[s.next()]);
            }
            else
                builder.push_back(s.next());
        }
        s.match('\"');
        str = builder.to_string(*pool);
    }
    void parse_array(Array &arr)
    {
        arr = Array(16);
        Value val;
        s.match('[');
        while (true)
        {
            s.skip_blanks();
            if (s.peek() == ']')
                break;
            parse_value(val);
            arr.push_back(std::move(val));
            s.skip_blanks();
            if (s.peek() != ',') // optional last comma
                break;
            else
                s.next();
        }
        s.match(']');
    }
    void parse_bool(bool &bo)
    {
        if (s.peek() == 't')
        {
            s.match('t'), s.match('r'), s.match('u'), s.match('e');
            bo = true;
        }
        else
        {
            s.match('f'), s.match('a'), s.match('l'), s.match('s'), s.match('e');
            bo = false;
        }
    }
};
} // namespace mp

#endif // PARSER_HPP