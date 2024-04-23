/*
 * ArrayOut.cpp - Stream output for ALL types of array ^_^
 *     This is just a toy. DO NOT use it in any production environment.
 *     It stream output all types of array, if the stream output for its
 *     element is defined.
 *     This requires C++20 to compile.
 */
#include <iostream>
#include <type_traits>
using namespace std;

template <typename T, size_t n> 
ostream &operator<< (ostream &o, const T (&a)[n]) requires (
    ! is_same <char, typename remove_cvref<T>::type>::value 
) {
    for (size_t i = 0; i < n; i ++)
        o << a[i] << ", " + ((i == n - 1) << 1);
    return o;
}

/*
 * The output of the following program will be:
 * Data: [ 1, 2, 3, 4, 5 ]
 * Name: [ Alice, Bob, Charlie, David, Eve ]
 */
int main(){
    int data[] = { 1, 2, 3, 4, 5 };
    const char *strs[] = { "Alice", "Bob", "Charlie", "David", "Eve" };
    cout << "Data: [ " << data << " ]" << endl;
    cout << "Name: [ " << strs << " ]" << endl;
    return 0;
}
