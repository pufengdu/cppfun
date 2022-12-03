#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

void *mbcopy(void *dst, const void *src, size_t dst_size, size_t src_size){
    memcpy(dst, src,  ( src_size <= dst_size ) ? src_size : dst_size);
    memset((byte*)dst + src_size, 0, ( src_size <= dst_size ) ? dst_size - src_size : 0);
    return dst;
}

//Do you like a box?
template <typename T>
struct Box{
    T &data;
    template <typename H>
    T &operator= (const H &a){ return *reinterpret_cast<T*>(mbcopy(&data, &a, sizeof(T), sizeof(H))); }
    operator T&(){return data;}
    T &operator()(){return data;}
};

int main() {
    int a[] = {1, 2, 3, 4};
    int b[] = {5, 6, 7, 8, 9};
    Box{a} = b; // We can not do a = b. But, we can do Box{a} = b;
    for (auto &p: a)
        cout << p << endl;
    return 0;
}
