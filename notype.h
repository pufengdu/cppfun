/****************************************************************************
 * notype.h - Copyright 2021 Pufeng Du, Ph.D.                               *
 *                                                                          *
 * This file is a toy in playing with "dynamic reflection" in cpp           *
 * notype.h is free software: you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * notype.h is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with notype.h  If not, see <http://www.gnu.org/licenses/>.         *
 ****************************************************************************/

#ifndef NOTYPE_H_INCLUDED
#define NOTYPE_H_INCLUDED
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <functional>

#define BUILTIN_TYPES             /* C++ standard fundamental value types   */\
    bool,                                              /* 1 byte type       */\
    char, unsigned char,                               /* 1 byte chars      */\
    short, unsigned short,                             /* 2 bytes types     */\
    int, unsigned int, long long, unsigned long long,  /* 4+ bytes types    */\
    float, double, long double                         /* Floating points   */

#define BUILTIN_OPBINS            /* Binary operators in functional classes */\
    plus<>, minus<>, multiplies<>, divides<>, modulus<>,/* + - * / %        */\
    equal_to<>, not_equal_to<>, greater<>, less<>,      /* == != > <        */\
    greater_equal<>, less_equal<>,                      /* >= <=            */\
    logical_and<>, logical_or<>,                        /* && ||            */\
    bit_and<>, bit_or<>, bit_xor<>                      /* & | ^            */

#define BUILTIN_OPBIN_NAMES       /* Binary operators in enumerator names   */\
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,             /* + - * / %        */\
    OP_EQ, OP_NE, OP_GT, OP_LT, OP_GE, OP_LE,           /* == != > < >= <=  */\
    OP_AND, OP_OR,                                      /* && ||            */\
    OP_BAND, OP_BOR, OP_BXOR,                           /* & | ^            */\
    OP_END                                         /* Sentinel for counting */

using namespace std;

template<typename T>
string CompileTimeTypeName() {
    string func_name = __PRETTY_FUNCTION__;
    size_t _s = func_name.find("T = ") + 4;
    size_t _e = func_name.find(";", _s);
    string r = func_name.substr(_s, _e - _s);
    return r;
}

struct var;

struct _void {
    virtual ostream &output(ostream &o) = 0;
    virtual istream &input(istream &i) = 0;
    virtual size_t getTypeLength() = 0;
    virtual string getTypeName() = 0;
    virtual type_index getRunTimeTypeIndex() = 0;
    virtual const char *getRunTimeTypeName() = 0;
    virtual void *getAddress() = 0;
    virtual operator bool() = 0;
    virtual _void *clone() = 0;
    virtual ~_void() = default;
};

template <typename T>
struct _Value: public _void{
    typedef typename remove_reference<T>::type ValueType;
    typedef typename conditional<is_array<ValueType>::value, typename remove_all_extents<ValueType>::type, void>::type ItemType;
    _Value(const T &a) requires (is_reference<T>::value && !is_fundamental<ValueType>::value): _data( *new ValueType{a}) {
        throw runtime_error("Error: complex underlying type reference wrapping detected.");
    }
    _Value(const T &a) requires (is_same<ValueType, var>::value):_data( *new ValueType{0}) {
        throw runtime_error("Recursive abstraction detected. Abort.");
    }
    _Value(const T &a) requires (is_reference<T>::value && is_fundamental<ValueType>::value): _data(*new ValueType{a}) { }
    _Value(const T &a)
        requires (
            !is_reference<T>::value &&
            !is_same<T, var>::value &&
            is_copy_assignable<T>::value &&
            is_copy_constructible<T>::value
        ): _data(*new ValueType{a}) { }
    _Value(const T &a)
        requires (
            is_array<ValueType>::value &&
            is_copy_assignable<ItemType>::value &&
            is_copy_constructible<ItemType>::value
        ): _data( *(ValueType*)(new ValueType) ) {
        constexpr size_t n = sizeof(ValueType) / sizeof(ItemType);
        ItemType *_t = reinterpret_cast<ItemType*>(_data);
        const ItemType *_s = reinterpret_cast<const ItemType*>(a);
        for (size_t i = 0; i < n ; i ++)
            _t[i] = _s[i];
    }
    _Value<T> *clone(){ return new _Value<T>(_data); }
    constexpr size_t getTypeLength() noexcept override { return sizeof(ValueType); }
    constexpr string getTypeName() noexcept override  { return CompileTimeTypeName<ValueType>(); }
    type_index getRunTimeTypeIndex() override { return type_index(typeid(ValueType)); }
    const char *getRunTimeTypeName() override { return type_index(typeid(ValueType)).name(); }
    void *getAddress() noexcept override { return &_data; }
    operator bool() {
        if constexpr (is_convertible<ValueType, bool>::value)
            return static_cast<bool>(_data);
        else{
            bool r = false;
            auto p = reinterpret_cast<const uint8_t(*)[sizeof(_data)]>(&_data);
            for (auto &i: *p) r |= !!i;
            return r;
        }
    }
    ostream &output(ostream &o) override { return o << _data; }
    istream &input(istream &i) override { return i >> _data; }
    ~_Value() {
        if constexpr (is_array<ValueType>::value)
            delete [] reinterpret_cast<ItemType*>(&_data);
        else
            delete &_data;
    }
    ValueType &_data;
};

enum opbin_names: size_t { BUILTIN_OPBIN_NAMES };

template <size_t N, typename... DataTypes>
struct type_array;

template <size_t N, typename T, typename... DataTypes>
struct type_array <N, T, DataTypes...> {
    typedef typename conditional<(N > 0), typename type_array<N - 1, DataTypes...>::type, T>::type type;
};

template <typename T, typename... DataTypes>
struct type_array <0, T, DataTypes...> { typedef T type; };

template <size_t op>
struct opbin_func_lib { typedef typename type_array<op, BUILTIN_OPBINS >::type type; };

template <typename _A, typename _B, typename _OP>
_void *opbin_func(const _void &_a, const _void &_b){
    const _Value<_A> &a = dynamic_cast<const _Value<_A>&>(_a);
    const _Value<_B> &b = dynamic_cast<const _Value<_B>&>(_b);
    auto r = _OP()(a._data, b._data);
    return new _Value<decltype(r)>(r);
}

struct {
    typedef _void *(*opbin_func_t)(const _void &, const _void &);
    typedef unordered_map<type_index, opbin_func_t> opbin_list;
    typedef unordered_map<type_index, opbin_list> opbin_matrix;
    bool has_type(const type_index &a) {
        return !(r.find(a) == r.end());
    }
    bool has_type(const type_index &a, const type_index &b) {
       return has_type(a)? !(r[a].find(b) == r[a].end()): false;
    }
    void set_op(const type_index &a, const type_index &b, opbin_func_t f) {
        (has_type(a)? r[a]: ((*(r.emplace(a, opbin_list()).first)).second))[b] = f;
    }
    opbin_func_t get_op(const type_index &a, const type_index &b) {
        return has_type(a, b)? r[a][b]: nullptr;
    }
    opbin_matrix r;
} opbin_registry[OP_END];

template <typename _A, typename _B, size_t _opbin>
void opbin_enable(){
    typedef typename opbin_func_lib<_opbin>::type _op;
    typedef typename remove_reference<typename remove_cv<_A>::type>::type A;
    typedef typename remove_reference<typename remove_cv<_B>::type>::type B;
    if constexpr (is_invocable<_op, A, B>::value)
        opbin_registry[_opbin].set_op(type_index(typeid(A)), type_index(typeid(B)), opbin_func<A, B, _op>);
}

template <typename _A, typename _B, size_t _opbin>
void opbin_enable_all()  noexcept {
    opbin_enable<_A, _B, _opbin>();
    opbin_enable<_B, _A, _opbin>();
    if constexpr (_opbin)
        opbin_enable_all<_A, _B, _opbin - 1>();
}

template<size_t M, size_t N, typename... DataTypes>
struct type_pair{
    typedef typename type_array<M, DataTypes...>::type TypeA;
    typedef typename type_array<N, DataTypes...>::type TypeB;
};

template<size_t M, size_t N, typename... DataTypes>
void type_iter_b();

template<size_t M, size_t N, typename... DataTypes>
void type_iter_a(){
    type_iter_b<M, N, DataTypes...>();
    if constexpr (M) type_iter_a<M - 1, N, DataTypes...>();
}

template<size_t M, size_t N, typename... DataTypes>
void type_iter_b(){
    typedef typename type_pair<M, N, DataTypes...>::TypeA A;
    typedef typename type_pair<M, N, DataTypes...>::TypeB B;
    opbin_enable_all< A, B, OP_END - 1 >();
    if constexpr (N) type_iter_b<M, N - 1, DataTypes...>();
}

template<typename... DataTypes>
void type_iter(){
    type_iter_a<sizeof...(DataTypes) - 1, sizeof...(DataTypes) - 1, DataTypes...>();
}

struct var{
    template <typename T>
    var(const T &a):_vdata(new _Value<T>(a)){ }
    template <typename T>
    var(T &&a):_vdata(new _Value<T>(a)){ }
    var (const var &a): _vdata(a._vdata->clone()){}
    ~var(){ delete _vdata;  _vdata = nullptr; }
    const var &operator=(const var &a) {
        delete _vdata;
        _vdata = a._vdata->clone();
        return *this;
    }
    operator bool() { return !!(*_vdata); }
    size_t _sizeof() { return _vdata->getTypeLength(); }
    string _typename() { return _vdata->getTypeName(); }
    type_index _rtTypeIndex() { return _vdata->getRunTimeTypeIndex(); }
    const char *_rtTypeName() { return _vdata->getRunTimeTypeName(); }
    void *getDataAddress() { return _vdata;}
    _void *_vdata;
    static void init(){ type_iter<BUILTIN_TYPES>(); }
    friend var opbin_dispatch(const var &a, const var &b, opbin_names op);
    private: var():_vdata(nullptr){ }
};

ostream &operator<< (ostream &o, const var &k){
    return (k._vdata)->output(o);
}

istream &operator>> (istream &i, const var &k) {
    return (const_cast<var &>(k)._vdata)->input(i);
}

var opbin_dispatch(const var &a, const var &b, opbin_names op){
    var r;
    type_index ta = a._vdata->getRunTimeTypeIndex();
    type_index tb = b._vdata->getRunTimeTypeIndex();
    auto opbin_action = opbin_registry[op].get_op(ta, tb);
    if (!opbin_action) throw bad_function_call();
    r._vdata = opbin_action(*(a._vdata),*(b._vdata));
    if (!r._vdata) throw runtime_error("Undefined operation between underlying types.");
    return r;
}

var operator+  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_ADD); }
var operator-  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_SUB); }
var operator*  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_MUL); }
var operator/  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_DIV); }
var operator%  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_MOD); }
var operator== (const var &a, const var &b){ return opbin_dispatch(a, b, OP_EQ);  }
var operator!= (const var &a, const var &b){ return opbin_dispatch(a, b, OP_NE);  }
var operator>  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_GT);  }
var operator<  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_LT);  }
var operator>= (const var &a, const var &b){ return opbin_dispatch(a, b, OP_GE);  }
var operator<= (const var &a, const var &b){ return opbin_dispatch(a, b, OP_LE);  }
var operator&& (const var &a, const var &b){ return opbin_dispatch(a, b, OP_AND); }
var operator|| (const var &a, const var &b){ return opbin_dispatch(a, b, OP_OR);  }
var operator|  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_BOR); }
var operator&  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_BAND);}
var operator^  (const var &a, const var &b){ return opbin_dispatch(a, b, OP_BXOR);}
var &operator+= (var &a, const var &b) {a = a + b; return a;}
var &operator-= (var &a, const var &b) {a = a - b; return a;}
var &operator*= (var &a, const var &b) {a = a * b; return a;}
var &operator/= (var &a, const var &b) {a = a / b; return a;}
var &operator%= (var &a, const var &b) {a = a % b; return a;}


#endif // NOTYPE_H_INCLUDED
