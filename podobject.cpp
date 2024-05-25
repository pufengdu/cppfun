/* ================================================================================
 * Author: Dr. Pu-Feng Du (2024)
 * A proof of concept: Extending built-in basic data types in C++.
 * Basic types can be wrapped by a struct type, with the same layout. This will
 * work if the wrapped type follows standard layout. This example shows how to
 * create wrapping object for all basic type objects, like int, char, or float.
 * The pod_object_t wraps all pod object, in place. This type CANNOT be
 * instantiated normally. It MUST be constructed by the factory function:
 * as_object(). The as_object() function makes objects in-place for all basic
 * type objects. For example. int z = 1; declares object z of basic int type. The
 * as_object(z) creates a object pod_object_t<int> in the same address as z. This
 * allows adding member functions to basic types. For example, the as_string()
 * function is added. With type traits, it is possible to define member function
 * for a specific basic type. The only additional requirement is use as_object()
 * to temporarily use the basic type object as a wrapped object. See main() function
 * for more details.
 * =================================================================================
 */

#include <iostream>
#include <format>
using namespace std;

template<typename T>
struct alignas(alignof(T)) pod_object_t {
    T data;
    string as_string(){
        return format("{}", data);
    }
private:
    pod_object_t() = delete;
    ~pod_object_t() = delete;
};

template<typename T>
ostream &operator<< (ostream &os, const pod_object_t<T> &a){
    return os << a.data;
}

template<typename T>
pod_object_t<T> &as_object(T &a) {
    return *reinterpret_cast<pod_object_t<T>*>(&a);
}

int main() {
    int z = 1;
    cout << as_object(z).as_string() << endl;
    return 0;
}
