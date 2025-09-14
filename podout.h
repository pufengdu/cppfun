/*
    podout.h - a very quick and dirty reflection for pod objects using c++20

    Sep. 2025

    Author: Dr. Pufeng Du, 
    
    Notes: Basically, I reimplement a most basic and inefficient version of the 
    famous Boost:PFR:magic_get, which was developped by Antony Polukhin.
    (https://github.com/apolukhin/magic_get). I do not speak russian. So I 
    spent some time watching his representation on cppcon16 and cppcon18 to
    figure out key techs in that library. I included some technique known as 
    the friend injection or stateful meta programming, which was originally 
    proposed by Alexandr Poltavsky (https://alexpolt.github.io/type-loophole.
    html). I do not believe that I can declare any kind of copyright on this 
    header. So there is no GNU GPL on this header.

    This is a very quick and dirty implementation of the reflection of POD data. 
    The usage of this header is simple. 
    Step 1 - include the header.
    Step 2 - define your own POD structs.
        No array member is allowed in the struct.
    Step 3 - use stream output operator to write all members to the stream

    You can explore this simple header yourself. It has some basic facilities 
    for reflections. For example, you can obtain members using member names at
    runtime. (These names must be assigned by yourself. I mean, any name, not 
    necessarily same as your definition in your POD struct.) The member values 
    will be returned as strings. There is a for_each function if want to use a 
    lambda to visit each member of a POD object. The usage of the for_each is 
    like:

        reflect(pod_object).for_each(your_lambda);
    
    If you want to call static member functions of the class_t<T>, you need to 
    first call reflect<T>() or reflect(t), if t is an object of type T.

    Again, this is just a toy for juniors. Do not expect too much.
*/

#ifndef _PODOUT_H_
#define _PODOUT_H_

#include <iostream>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <array>

template <typename S>
constexpr auto reflect(const S &s = {});

template <typename S>
bool is_reflected = false;

template <typename T>
concept is_stream_readable = requires (std::istream &is, T &a) {  is >> a; };

template <typename T>
concept is_stream_writable = requires (std::ostream &os, const T &a) { os << a; };

template <typename S, typename... Ts>
concept is_aggregatable = requires (){ { S { Ts{}... } } -> std::same_as<S>; };

template<typename T>
std::string typename_to_string(){
    std::string func_name = __PRETTY_FUNCTION__; 
    size_t _s = func_name.find("T = ") + 4;
    size_t _e = func_name.find(";", _s);
    std::string r = func_name.substr(_s, _e - _s);
    return r;
}

// Stateful meta programming using the type loophole. We use friend injections 
// to save types at compile time. Get yourself prepared to see many warnings...
// Or, you should add -Wno-non-template-friend to your compile options. I assume
// you are using GCC.
template <typename S>
struct tag_t{
    friend auto constexpr struct_to_tuple(tag_t<S>);
};

template <typename S, typename Tp>
struct make_tuple_from_struct{
    make_tuple_from_struct(){ is_reflected< S > = true; }
    friend auto constexpr struct_to_tuple(tag_t<S>){ return Tp{}; }
};

template <typename S, size_t I>
struct member_tag_t{
    friend auto constexpr member_type_ptr(member_tag_t<S, I>);
    friend auto constexpr member_type(member_tag_t<S, I>);
};

template <typename S, size_t I, typename M, typename Mp, Mp p >
struct save_member_type{
    friend auto constexpr member_type_ptr(member_tag_t<S, I>){ return p; }
    friend auto constexpr member_type(member_tag_t<S, I>){ return M{}; }
};

template <typename S>
struct void_struct_t{};

template <typename S, size_t n = 0, typename M = void_struct_t<S>, 
    typename... Ts > 
    requires (std::is_trivial_v<M> && std::is_standard_layout_v<S>)
struct auto_t{
    template <typename U> 
        requires (std::is_standard_layout_v<U>)
    constexpr operator U() noexcept {
        if constexpr (std::is_class_v<U>){
            if (!is_reflected< U >)
                reflect<U>();
        }
        // Add members according to S one after one to get member offsets. I 
        // have no idea whether this will be the same as S in all cases, if S is
        // POD. Surely, this relies on optimizations for empty base classes. The 
        // type_S object is trivial. All offsets are obtained by inheritence to 
        // construct a struct according to S. The member pointer to each member 
        // is binary compatible to its offset.
        using type_S = struct: public M { U _t; };

        // This saves the n-th member of S by using pointer-to-member of type_S.
        // Since type_S is not S, we can not save offsets of each member in a 
        // constexpr static member. But, this will delay the specialization of 
        // member_t<S, n>
        save_member_type<S, n, U, decltype(&type_S::_t), &type_S::_t>{};
        
        // Try construction incrementally, to iterate over all members of S 
        // recursively.
        using next_t = auto_t<S, n + 1, type_S, Ts..., U>;
        if constexpr (is_aggregatable<S, Ts..., U, next_t> )
        // By instantiating S with more initilizers, we use the U() function to 
        // iterate over types of all S members.
            S { Ts{}..., U{}, next_t{} };
        else
        // When we are inside the last member's U(), we can save all types of
        // members of S by using friend injections. Below struct MUST be called 
        // in this way or using the sizeof() expression, so that causing friend
        // functions to be injected into the file scope.
            make_tuple_from_struct <S, std::tuple<Ts (S::*)..., U (S::*)> >{};
        return U{};
    }
};

template <typename S, size_t I>
struct member_t{
    using type = decltype(member_type(member_tag_t<S, I>{}));
    static type S::* ptr;
    static size_t offset;
    static std::string name;
};

template <typename S, size_t I>
member_t<S, I>::type S::* member_t<S, I>:: ptr = reinterpret_cast< 
    member_t<S, I>::type S::*>( 
        member_type_ptr( member_tag_t<S, I>{} ) 
    );
template <typename S, size_t I>
size_t member_t<S, I>::offset = *reinterpret_cast<size_t*>(
    &member_t<S, I>::ptr
);
template <typename S, size_t I>
std::string member_t<S, I>::name;

template <typename S>
struct class_t{
    class_t(const S &_o):_object(_o){ 
        if (!is_reflected< S >)
            reflect<S>();
    }
    const S &_object;
    
    using tuple_type = decltype(struct_to_tuple(tag_t<S>{}));
    static constexpr size_t member_count = std::tuple_size_v<tuple_type>;
    using offset_list_type = std::array<size_t, member_count>;
    using name_list_type = std::array<std::string, member_count>;
    
    static offset_list_type offsets;
    static tuple_type ptrs;
    static name_list_type names;

    template <size_t... ids>
    static constexpr auto get_offset(std::index_sequence<ids...>){
        return offset_list_type{ member_t<S, ids>::offset... };
    }
    
    template <size_t... ids>
    static auto get_ptrs(std::index_sequence<ids...>){
        return tuple_type{ member_t<S, ids>::ptr... };
    }

    template <size_t n = 0, typename N0 = std::string, typename... Args>
    static auto set_member_names(N0 n0 = "", Args... ns){
        if constexpr (n < member_count){
            member_t<S, n>::name = n0;
            names[n] = n0;
        }
        if constexpr (sizeof...(ns) > 0)
            return set_member_names<n + 1>(ns...);
        else
            return n;
    }

    template <size_t n = 0, typename N = std::string>
    std::string get_member_by_name_as_str(const N &nx){
        if constexpr (n < member_count){
            using member_type_as = decltype(_object.*std::get<n>(ptrs));
            if (member_t<S, n>::name == nx){
                if constexpr (is_stream_writable < member_type_as >)
                    return (std::stringstream() << (_object.*std::get<n>(ptrs))).str();
                else
                    return "";
            }
            return get_member_by_name_as_str<n + 1>(nx);
        }
        else
            throw std::runtime_error("Unable to find member by name");
    }

    template <size_t n = 0, typename N = std::string>
    void set_member_by_name_from_str(const N &nx, const std::string &s){
        if constexpr (n < member_count){
            using member_type_mutable = std::remove_cvref_t<
                decltype(_object.*std::get<n>(ptrs))
            >;
            if (member_t<S, n>::name == nx) {
                if constexpr ( is_stream_readable<member_type_mutable> )
                    std::stringstream(s) >> const_cast<member_type_mutable&>(
                        _object.*std::get<n>(ptrs)
                    );
                return;
            }
            set_member_by_name_from_str< n + 1 >(nx, s);
        }
        else 
            throw std::runtime_error("Unable to find member by name");
    }

    template <size_t n = 0> 
    bool for_each(auto func){
        if constexpr (n < member_count){
            func(_object.*std::get<n>(ptrs));
            return for_each< n + 1>(func);
        }
        else
            return true;
    }
};
template <typename S>
class_t<S>::offset_list_type class_t<S>::offsets = class_t<S>::get_offset(
    std::make_index_sequence<class_t<S>::member_count>{}
);
template <typename S>
class_t<S>::tuple_type class_t<S>::ptrs = class_t<S>::get_ptrs(
    std::make_index_sequence<class_t<S>::member_count>{}
);
template <typename S>
class_t<S>::name_list_type class_t<S>::names;

template <typename S>
    requires (std::is_class_v<S> && std::is_standard_layout_v<S> && std::is_trivial_v<S>)
std::ostream &operator<< (std::ostream &o, const S &a){
    auto stream_out_action = [&](auto &p){ o << p << ' '; };
    o << "[ ";
    reflect(a).for_each(stream_out_action);
    o << ']';
    return o;
}

template <typename S>
    requires (std::is_class_v<S> && std::is_standard_layout_v<S> && std::is_trivial_v<S>)
std::istream &operator>> (std::istream &i, S &a){
    auto stream_in_action = [&](auto &p){ i >> p; };
    reflect(a).for_each(stream_in_action);
    return i;
}

template <typename S>
constexpr auto reflect(const S &s){
    if (!is_reflected< S >)
        S { auto_t<S>{} };
    return class_t<S>(s);
}

#endif //_PODOUT_H_
