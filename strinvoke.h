/****************************************************************************
 * strinvoke.h - Copyright 2025 Pufeng Du, Ph.D.                            *
 *                                                                          *
 * The strinvoke.h is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * strinvoke.h is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with interface.h  If not, see <http://www.gnu.org/licenses/>.      *
 ****************************************************************************/

/* 
    strinvoke.h - Invoke a function using only strings
    
    Author: Dr. Pu-Feng Du (2025)
    
    For all C++ newbies, there is alway a dream. Get a function name by cin and execute 
    this function at runtime. This seems to be a funny or maybe stupid idea in eyes of 
    people understand more C++. However, this worth a header file. Because it is a 
    question that can we invoke functions and pass arguments to functions all by strings
    at runtime. Someone may directly refer this to the great OOP term "reflection", which
    will become reality in C++26 (Please check with those C++ gods). Not long, right? On 
    the other way, if your babies/students cry for such things, you can give them this 
    header as a candy. It will work in most simple cases that newbies can imaging of.
    
    This header can be used in this way:
    1 include it
    2 define your functions, whatever, say 
        int foo(int a, float b);
    3 make sure all parameter types and the return-value type are "Streamable", that is to 
    have << and >> for stream out and in.
    4 use macro export_function(name) to export your function, like export_function(foo);
    4 use the function ibsCall to invoke your function, like
        ibsCall ({"foo", "15", "15.5"});
    ibsCall will also return the return value of your function as a string.

    Oh, we need C++20. Please update your compiler, if you had not.

*/

#ifndef __STR_INVOKE_H__
#define __STR_INVOKE_H__
#include <concepts>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

// Define concept streamable
template <typename T>
concept Streamable_out = requires (const T &a) { declval<ostream &>() << a; };

template <typename T>
concept Streamable_in = requires (T &a){ declval<istream &>() >> a; };

template <typename T>
concept Streamable = Streamable_out<T> && Streamable_in<T>;

template <typename T>
concept FunctionRet = Streamable_out<T> || is_void_v<T>;

// Converting between other types and strings, rely on stringstream.
// Not a good solution, but it works.
template <Streamable_in T>
T FromString(const string &s){
    stringstream ss(s);
    T r;
    ss >> r;
    return r;
}

template <Streamable_out T>
const string ToString(const T &o) {
    stringstream ss;
    ss << o;
    return ss.str();
}

// Core functions for Invoke-By-Strings to work
template <size_t n, Streamable_in T, typename... Args>
auto StringToObject(const string &s){
    if constexpr (n)
        return StringToObject<n - 1, Args...>(s);
    else
        return FromString<T>(s);
}

template <FunctionRet R, typename... FArgs, typename... Args>
auto InvokeByStrings( R(&f)(FArgs...), const vector<string> &p, Args... a){
    // Iterating args right-to-left
    const size_t args_i = sizeof...(FArgs) - sizeof...(Args) - 1;
    if (p.size() < sizeof...(FArgs))
        throw runtime_error("Not enough arguments");
    if constexpr (sizeof...(Args) < sizeof...(FArgs))
        return InvokeByStrings( f, p, StringToObject< args_i, FArgs... >(p[args_i]), a... );
    else
        return f(a...); //Here, we emit the real call
}

// ibs - Invoke-By-Strings
// Dynamic facilities
struct ibsBase{
    virtual const string invoke(const vector<string> &p) = 0;
    template <typename R, typename... Args>
    static bool addFunction(R(&f)(Args...), const string &name);
    static unordered_map<string, ibsBase*> ibsFunctions; // Naked poiner type?? This is a toy. If it breaks, buy a new one.
};
unordered_map<string, ibsBase*> ibsBase::ibsFunctions;

template <typename R, typename... FArgs>
struct ibsCaller: public ibsBase {
    using fType = R(&)(FArgs...);
    ibsCaller(fType f):fp(f){}
    const string invoke(const vector<string> &p) {
        if constexpr (!is_void<R>::value)
            return ToString(InvokeByStrings(fp, p));
        else {
            InvokeByStrings(fp, p);
            return string();
        }
    }
    fType fp;
};

template <typename R, typename... Args>
bool ibsBase::addFunction(R(&f)(Args...), const string &name){
    if (ibsBase::ibsFunctions.contains(name))
        return false;
    else
        ibsBase::ibsFunctions[name] = new ibsCaller(f); // Memory leaks!! Wooops....
    return true;
}

// Exposing interface function to clients
const string ibsCall(const string &name, const vector<string> &p){
    if (ibsBase::ibsFunctions.contains(name))
        return ibsBase::ibsFunctions[name]->invoke(p);
    else
        throw runtime_error("Required function was not registered");
}

const string ibsCall(const vector<string> &p){
    const vector<string> a (p.begin() + 1,p.end());
    return ibsCall(p[0], a);
}

#define export_function(f) if (!ibsBase::addFunction(f, #f)) throw runtime_error("Duplicate function names");

// End of the Invoke-By-Strings

#endif //__STR_INVOKE_H__
