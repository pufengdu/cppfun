# CPP fun snippets
This repo is just for fun.

## arrayout.cpp

A common stream output template for all types of array. It is just a toy.

## arrayshape.cpp

A toy to extract shape data of a multi-dimensional array for using in runtime.

## bintypes.cpp

A toy for beginners to see binary encodings of POD arithmetic types, like int and float.

## box.cpp

A box object that can hold all types of pod object. Allow some weired things to be done.

## interface.h

A toy to do runtime polymorphism without using virtual functions. Therefore, standard layouts and runtime polymorphism co-exist. You can use it like the following code:



```cpp
#include <iostream>
#include "interface.h"
using namespace std;

IFACE_BEGIN(IShape)
    double area(){
        DEF_VF(IShape,area);
    }
IFACE_END(IShape, area)

class Circle: public implement <Circle, IShape>{
public:
    Circle(double _r):r(_r){}
    double area(){
        cout << "Circle: ";
        return 3.14 * r * r;
    }
private:
    double r;
};

class Rectangle: public implement <Rectangle, IShape> {
public:
    Rectangle(double _w, double _h):h(_h),w(_w){}
    double area() {
        cout << "Rectangle: ";
        return h * w;
    }
private:
    double h;
    double w;
};

int main(int argc, char *argv[]) {
    Circle s1(10);
    Rectangle s2(10, 20);
    IShape *p = &s1;
    cout << (p->area()) << endl;
    p = &s2;
    cout << (p->area()) << endl;
    return 0;
}
```

## notype.h

A var class that can hold all types of pod object, even programmer defined types. Operators can be automatically applied, if they are defined.

This is NOT a reflection implementation.

It is just a TOY.

Try it with the following codes (Compiled with -std=c++20 -Wa,-mbig-obj):

```cpp
#include <iostream>
#include <cstdint>
#include "notype.h"
using namespace std;

struct Vec2D{
    int x;
    int y;
};

ostream &operator<< (ostream &o, const Vec2D &k){
    return o << k.x << ',' << k.y;
}

istream &operator>> (istream &i, Vec2D &k) {
    return i >> k.x >> k.y;
}

int main(){
    var::init();
    var x = Vec2D(2, 4);
    cout << x << endl;
    x = 10;
    cout << x << endl;
    x = 5.5;
    cout << x << endl;
    x = "Hello";
    cout << x << endl;
    return 0;
}

```

## podobject.cpp

A proof of concept for extending basic data types in C++. A simple quick and dirty demo of concept to wrap and add member functions for types like int, char, float. 

## podout.h

A quick and dirty reflection of pod objects using c++20. With this header, you may stream out pod objects directly. Try it with the following code. Read comments in the header before you want to use it in other places. This needs C++20, add the compile option yourself.

```cpp
#include <iostream>
#include "podout.h"
using namespace std;

struct score_sheet_t{
    double math;
    double english;
    double sport;
};

struct student_t{
    int id;
    const char *name;
    score_sheet_t score;
    bool gender;
};

int main(){
    student_t students[] = {
        {1, "Alice", { 100, 90.5, 87.5 }, 1},
        {2, "Bob", { 95, 77, 59.5 } , 0},
        {3, "Chalie", { 100, 81.2, 89.5 }, 0},
        {4, "David", { 78, 66, 100 }, 0}
    };
/* 
    The output will be like:

    [ 1 Alice [ 100 90.5 87.5 ] 1 ]
    [ 2 Bob [ 95 77 59.5 ] 0 ]
    [ 3 Chalie [ 100 81.2 89.5 ] 0 ]
    [ 4 David [ 78 66 100 ] 0 ]
*/
    for (auto s: students)
        cout << s << endl;
    return 0;
}
```

## strinvoke.h

A candy for newbie to invoke "ALL" functions using strings, which can be obtained at runtime from cin. Never think this TOY as something related to reflection.

```cpp
#include <iostream>
#include <vector>
#include "strinvoke.h"

// We need C++20
// add -std=c++20 to your compiler
using namespace std;

// Some tests for basic types:
int fx(int a, float b, char c){
    cout << a << endl << b << endl << c << endl;
    return 100;
}

int fy(double a){
    cout << a << endl;
    return 100;
}

void fz(){
    cout <<  100 << endl;
}

// Some tests using user-defined classes
struct Vec2D{
    int x, y;
};
// For this to work, we need these stream operators
ostream &operator<< (ostream &o, const Vec2D &v){
    return o << v.x << ' ' << v.y;
}

istream &operator>> (istream &i, Vec2D &v){
    return i >> v.x >> v.y;
}
// For example: a function
Vec2D SumVec(Vec2D a, Vec2D b){
    Vec2D r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

/* 
    Here is the usage:
    step 1, export all functions that need to be invoke-by-strings by macors export_function(function_name)
    you only need names here.
    step 2, call these functions by strings, using the ibsCall function, the first parameter is the stringify 
    function name, others are stringify arguments, in a vector<string> container.
    step 3, the returned value will also be stringify. You will need to parse it yourself.
    Note:
        1 Currently, we do not overload.
        2 All types of returned value and parameters of your "exported function" must be "streamable", as we 
        defined in concepts.
        3 you may pass more strings than need, first ones will be applied. 
        4 you cannot pass less strings than need, runtime error will be thrown.
*/
int main() {
    export_function(fx);
    export_function(fy);
    export_function(fz);
    export_function(SumVec);
    vector<string> x = {"11.01","12.22","foo"};
    vector<string> y = {"11","12"};
    // Most simple case
    cout << ibsCall("fz", x) << endl;
    // Call fx
    cout << ibsCall("fx", x) << endl;
    // Call fx using this form
    cout << ibsCall({"fx", "11.01","12.22","bar"}) << endl;
    // Call fy
    cout << ibsCall ("fy", y) << endl;
    // Try some Vectors
    cout << ibsCall({"SumVec", "1 2", "2 3"}) << endl;
    return 0;
}

```

## typefetch.h

Bind almost any type to an alias name. A proof of concept with the 'loophole' skills. This is NOT for any production use. This is NOT supported officially by standard, although this works in GCC.

## utf8.cpp

A demo for programming in Chinese in Codeblocks. This needs C++20 / UTF-8 compiler, and Windows 10. I do not use Windows 11. I have no idea how to configure that registry options in Windows 11.
