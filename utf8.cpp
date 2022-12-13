#include <iostream>
using namespace std;

// Programming in Chinese with UTF-8 in C::B
// Compiler Options:
// -Wall -fexceptions -g -pedantic-errors -std=c++20 -fextended-identifiers -finput-charset=utf-8 -fexec-charset=utf-8
// C::B Settings on the menu
// Use the following registry file:
// Settings -> Editor -> Encoding Settings -> Use encoding when opening files = UTF-8
// Settings -> Editor -> Encoding Settings -> Use this encoding = As default encoding (bypass...)
// Settings -> Editor -> Encoding Settings -> If conversion fails using this settings above, try... = [tick]
// Registry:
// HKEY_CURRENT_USER\Console\D:_Work_bin_codeblocks_cb_console_runner.exe
// \CodePage = 65001
/* The following is a template of Registry file, replace thost $XXX$
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Console\$cb_code_runner full path$]
"CodePage"=dword:0000fde9
*/
int main() {
    const char *姓名 = "阿巴阿巴阿巴";
    cout << 姓名 << endl;
    return 0;
}
