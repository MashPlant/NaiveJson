#include "../Object.hpp"
#include "../StringPool.hpp"
#include <iostream>

using namespace std;
using namespace mp;

int main()
{
    StringPool pool;
    StringBuilder builder;
    for (int i = 0; i < 5000; ++i)
    {
        builder.push_back('1');
        assert(strlen(builder.to_string(pool)) == i + 1);
    }
}