#include "Object.hpp"
#include "Parser.hpp"
#include <iostream>
using namespace std;
using namespace mp;

int main()
{
    // copid from my vscode setting
    const char *json = R"(
{
    "workbench.startupEditor": "newUntitledFile",
    "C_Cpp.default.cppStandard": "c++17",
    "workbench.iconTheme": "material-icon-theme",
    "workbench.colorTheme": "Dark+ Material",
}
    )";
    cout << json;
    Parser p;
    try
    {
        Object obj = p.parse(json);
        cout << endl;
        cout << obj["workbench.startupEditor"].get_str() << endl;
        cout << obj["C_Cpp.default.cppStandard"].get_str() << endl;
        cout << obj["workbench.iconTheme"].get_str() << endl;
        cout << obj["workbench.colorTheme"].get_str() << endl;
    }
    catch (std::exception &ex)
    {
        cout << ex.what() << endl;
    }
}