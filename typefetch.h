/**
 * typefetch.h - Aliasing nearly all types using state-full metaprogramming
 *     Curated by: Pufeng Du.
 *
 *     Warning: This is not officially supported in standard.
 *
 *     Notes:
 *     This is a demo to use the well-known 'loophole bug' to bind almost any type 
 *     to a name that can be used in compile time. Private member type can be extracted 
 *     in this way.
 *     This is NOT authored by me. I only curated pieces of codes from Github and Zhihu 
 *     with some modifications. 
 *
 *     References:
 *     https://stackoverflow.com/questions/65190015/c-type-loophole-explanation
 *     https://alexpolt.github.io/type-loophole.html
 *     https://www.zhihu.com/question/663583917/answer/3590252149
 *
 *     Usage: 
 *       BIND_TYPE(type_name, expression_to_extract_type, unique_id_for_the_type)
 *       BIND_TYPE must be used in file scope, out of all fucntions.
 *         type_name: The type alias name.
 *         expression_to_extract_type: like any expression you put in decltype(). However, 
 *       this expression will NOT be evaluated. The private scope restriction will NOT be
 *       checked.
 *         unique_id_for_the_type: a positive integer, uniquely binded to the type, to 
 *       tell which type is which. You may use your own macro skills to optimize this part.
 *       Like creating some tricks to make it increasing automatically.
 */

#include <iostream>
using namespace std;

template<typename T>
string CompileTimeTypeName() {
    string func_name = __PRETTY_FUNCTION__;
    size_t _s = func_name.find("T = ") + 4;
    size_t _e = func_name.find(";", _s);
    string r = func_name.substr(_s, _e - _s);
    return r;
}

template <size_t N>
struct tag{};

template <typename T, size_t N>
struct type_fetch_t{
    friend auto internal_type(tag<N>){return T{};}
};

#define BIND_TYPE(TYPENAME, EXPR, TYPE_ID) \
auto internal_type(tag<TYPE_ID>); \
template struct type_fetch_t<decltype(EXPR), TYPE_ID>; \
using TYPENAME = decltype(internal_type(tag<TYPE_ID>{}));

/*
// This is a use case, to extract the type of a private
// member function of a given class A. The type of the
// private member function is binded with an alias
// method_t.

class A{
private:
    int a_private_method();
};

BIND_TYPE(method_t, A::a_private_method, 0);

int main() {
    // The output should be
    // int (A::*)()
    cout << CompileTimeTypeName<method_t>() << endl;
}
*/

