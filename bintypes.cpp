/*
 *  bintypes.cpp - Fancy basic types for beginners to see binaries
 *
 *    Author: Dr. Pu-Feng Du (2024)
 *
 *    I defined a serial of "improved" basic types, Int32, Char, Float, Double,
 *    and Bool, for beginners of C++ to see their binary encodings.
 *
 *    To investigate the binary encodings of these types, one can simply write
 *    the 'bytes' member of an object on the screen.
 *
 *    For floating point types, like Float or Double, one can write the Mantissa,
 *    Exponent and Sign part separately on the screen, by calling member method
 *    ieee754().get_m() or something like it. Currently, only float and double
 *    types are supported. That is to say we can parse FP32 and FP64 in IEEE754
 *    standard. However, if someone defined the FP16 for C++, current codes
 *    should also work with it. If FP128 is defined, I think the only modification
 *    is the fp_desc<T> template. It will need an FP128 format definition.
 *
 *    Display of these members are in binary format, with LSB to the RIGHT, which
 *    is good for carbon-based life forms to read ^_^. There is still room to
 *    improve the efficiency / flexible of the binary format display.
 *
 *    For any POD arithmetic types, you may wrap it by the seqable<T> template.
 *    The 'bytes' member can be displayed to show its binary encodings.
 *
 *    Common arithmetic operators, like +, -, *, and /, are automatically defined
 *    for the wrapped type.
 *
 *    We need C++20 to compile.
 *
 *    THIS IS A TOY. DO NOT EXPECT TOO MUCH.
 */
#include <iostream>
#include <cstdint>
using namespace std;

#define OP_DEF(_OP_)                              \
template <typename T>                             \
auto &operator _OP_ (seqable<T> &a, const T &b) { \
    a.data _OP_ b;                                \
    return a;                                     \
}

template <typename T, uint32_t W>
struct bit_field{
    T _data;
    static const uint32_t w;
    bit_field(const T &a) { _data = a; };
};
template <typename T, uint32_t W>
const uint32_t bit_field<T, W>::w = W;

template <typename T, uint32_t W>
ostream &operator<< (ostream &o, const bit_field<T, W> &a){
    uint32_t w = W;
    T d = a._data, t = 0;
    for (uint32_t i = 0; i < w; i++, d >>= 1) (t <<= 1) |= (d & 1);
    for (uint32_t i = 0; i < w; i++, t >>= 1) o << (t & 1);
    return o;
}

using byte_t = bit_field<uint8_t, 8>;
template <uint32_t N>
struct bytes_t{
    byte_t data[N];
};

template <uint32_t N>
ostream &operator<< (ostream &o, const bytes_t<N> &a){
    for (uint32_t i = 0; i < N; i ++)
        o << a.data[N - i - 1];
    return o;
}

template <typename T, uint32_t M, uint32_t E, uint32_t S>
struct fp_desc_t{
    using int_val_t = T;
    static const uint32_t m_s, e_s, s_s;
};
template <typename T, uint32_t M, uint32_t E, uint32_t S>
const uint32_t fp_desc_t<T, M, E, S>::m_s = M;
template <typename T, uint32_t M, uint32_t E, uint32_t S>
const uint32_t fp_desc_t<T, M, E, S>::e_s = E;
template <typename T, uint32_t M, uint32_t E, uint32_t S>
const uint32_t fp_desc_t<T, M, E, S>::s_s = S;

template <typename T>
using fp_desc =
    typename conditional< sizeof(T) == 8, fp_desc_t<uint64_t, 52, 11, 1>,         /*IEEE754 binary 64 Format*/
        typename conditional< sizeof(T) == 4, fp_desc_t<uint32_t, 23, 8 , 1>,     /*IEEE754 binary 32 Format*/
            typename conditional< sizeof(T) == 2, fp_desc_t<uint16_t, 10, 5, 1>,  /*IEEE754 binary 16 Format*/
                void
            >::type
        >::type
    >::type;

template <typename T>
struct ieee754_t{
    using desc_type = fp_desc<T>;
    using val_type = desc_type::int_val_t;
    bit_field<val_type, desc_type::m_s> get_m() const { return m; }
    bit_field<val_type, desc_type::e_s> get_e() const { return e; }
    bit_field<val_type, desc_type::s_s> get_s() const { return s; }
    val_type m:desc_type::m_s;
    val_type e:desc_type::e_s;
    val_type s:desc_type::s_s;
};

template <typename T>
union seqable{
    T data;
    bytes_t<sizeof(T)> bytes;
    const ieee754_t<T> &ieee754() requires (is_floating_point<T>::value){
        return *reinterpret_cast<ieee754_t<T>*>(&data);
    }
    seqable(const T &a){ data = a; }
    operator T(){ return data; }
    const seqable<T> &operator= (const T &a){
        data = a;
        return *this;
    }
};

OP_DEF(+=)
OP_DEF(-=)
OP_DEF(*=)
OP_DEF(/=)
OP_DEF(%=)

using Float = seqable<float>;
using Double = seqable<double>;
using Int32 = seqable<int32_t>;
using UInt32 = seqable<uint32_t>;
using Char = seqable<char>;
using Bool = seqable<bool>;

/*
 *    A use case for demo
 *      I did not test above codes entirely.
 *      Bugs may be there.
 *      Below is a simple test program.
 */
int main(){
    /*
     *  Output the Mantissa part of an FP32 value.
     */
    Float k = 1.2;
    // You have 00110011001100110011010 on your screen.
    cout << k.ieee754().get_m() << endl;

    Bool b = true;
    cout << b.bytes << endl;

    Int32 x = -127;
    cout << x.bytes << endl;

    Double d = 3.14;
    cout << d.ieee754().get_e() << endl;
    return 0;
}
