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

## typefetch.h

Bind almost any type to an alias name. A proof of concept with the 'loophole' skills. This is NOT for any production use. This is NOT supported officially by standard, although this works in GCC.

## utf8.cpp

A demo for programming in Chinese in Codeblocks. This needs C++20 / UTF-8 compiler, and Windows 10. I do not use Windows 11. I have no idea how to configure that registry options in Windows 11.
