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
            s.match(',');
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
        else if (isdigit(s.peek()))
        {
            parse_number(val);
        }
        else
            throw ParserExecption(format("unexpected char: %c", s.peek()));
    }
    void parse_number(Value &val)
    {
    }
    void parse_string(String &str)
    {
        static StringBuilder builder;
        builder.clear();
        s.match('\"');
        while (s.peek() != '\"')
            builder.push_back(s.next());
        s.match('\"');
        str = builder.to_string(*pool);
    }
    // void parse_bool()
    // {

    // }
};
} // namespace mp

#endif // PARSER_HPP