/*
 * ArrayShape.cpp - Extract shape of an array for use in runtime.
 *     Author: Dr. Pu-Feng Du (2024)
 *     array_shape(a) will give an rvalue object array_shape, describing array a 's shape.
 *     Shape of an array is a size_t array, containing extent of all dimensions. See below
 *     example for details. This is only a toy example or a proof of concept, not for using
 *     in a production environment.
 */

#include <iostream>
#include <type_traits>
using namespace std;

struct array_shape{
    template <typename T>
    array_shape(T &a)
        requires (is_array<T>::value): n(rank<T>::value), s(new size_t[n]{}) {
        init<T>();
    }
    ~array_shape() {
        if (s) {
            delete [] s;
            s = 0;
        }
    }
    template <typename T, size_t m = rank<T>::value>
    void init() {
        if constexpr (m > 0) {
            s[m - 1] = extent<T, m - 1>::value;
            init<T, m - 1>();
        }
    }
    const size_t n;
    size_t *s;
};

ostream &operator<< (ostream &os, const array_shape &as){
    for (size_t i = 0; i < as.n; i ++)
        os << as.s[i] << ", " + (i == as.n - 1) * 2;
    return os;
}

int main() {
    int a[9][8][7] = {};
    cout << array_shape(a) << endl; // output is 9, 8, 7
    return 0;
}
