/***********************************************************
  This is a portable way in C++20 to get name of a type.
  require:
  __cpp_lib_source_location >= 201907L
  The macro TypeName return type of any expression as a
  string_view object. Test this using the following:

    #include <iostream>
    #include "typename.h"
    int main(){
        std::cout << TypeName(1.5) << std::endl;
        return 0;
    }

***********************************************************/
#ifndef TYPENAME_H_INCLUDED
#define TYPENAME_H_INCLUDED
#include <source_location>
#include <string_view>
template <typename T>
constexpr std::string_view getCompileTimeTypeName() noexcept{
    using std::string_view;
    using std::source_location;
    typedef string_view::size_type SizeType;
    string_view s = source_location::current().function_name();
    SizeType name_begin = s.find("T =");
    if (name_begin == string_view::npos) return string_view();
    name_begin += 4;
    SizeType name_end = s.find(";");
    if (name_end == string_view::npos) return string_view();
    string_view r = s.substr(name_begin, name_end - name_begin);
    return r;
}
#ifndef TypeName
    #define TypeName(x) getCompileTimeTypeName<decltype((x))>()
#endif // TypeName
#endif // TYPENAME_H_INCLUDED
