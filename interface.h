/****************************************************************************
 * interface.h - Copyright 2024 Pufeng Du, Ph.D.                            *
 *                                                                          *
 * The interface.h is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * interface.h is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with notype.h  If not, see <http://www.gnu.org/licenses/>.         *
 ****************************************************************************/

/*
 * interface.h - A runtime polymorphism implementation without virutal function
 * 
 *     Author: Dr. Pu-Feng Du (2024)
 * 
 *     Generally, runtime polymorphism is achieved by using virtual functions, 
 * or more essentially, vtables. However, virtual functions destroy standard 
 * layouts of classes. A class containing virutal functions is not POD any more. 
 * In some cases, runtime polymorphism and POD layouts are required at the same 
 * time. Therefore many alternative ways of runtime polymorphism were developped. 
 * For example, M$'s proxy library is a good one. But to me, just for the fun 
 * purpose, I think it may be good to let C++ have some semantics like java's 
 * interfaces. Plus, I do need a solution for runtime polymorphism on objects 
 * with standard layouts. I also feel that M$'s proxy library is not quite easy 
 * to be adopted with simple client codes. 
 * 
 *     For someone who has already get used to the virutal function / inheritance 
 * way to express runtime polymorphism, Java's interface semantics is easy to be 
 * understood. That's is the purpose of this header. 
 * 
 *     In this header, you will find a template "interface" and a template "implement". 
 * They genenrally allow you to define your "interface" and let your classes to 
 * "implement" those interfaces. Since the usage of the "interface" template is a bit 
 * bizzare. I made several macros to help (Do not hate me because of this. Macros 
 * are good here.). Still, remember this is just a toy level example. DO NOT expect 
 * any efficiency or safty from this. You never use it in your production environment!
 * 
 *     For example, you may write your interface and implementation as follows: 
 * 
 * // A student book level example.
 * 
 * IFACE_BEGIN(IShape)
 *      double area(){  // you may have parameters for virtual functions, if you have int a, int b, float c
 *          DEF_VF(IShape, area);   // add them here like: DEF_VF(IShape, area, a, b, c);
 *      }
 * IFACE_END(IShape, area) // if you have several virutal functions, like area, length, volume, list them here like:
 *                         // IFACE_END (IShape, area, length, volume). Currently, up to 8 virtual functions can be used in
 *                         // every interface. You can modify the FOR_EACH macros below to increase this.
 * 
 * // I believe nothing below this line require explaination
 * // Just remember that you can implement more than one interfaces in one class, 
 * // like: class Circle: public implement<Circle, IShape, I2DShape, I3DShape, IFssK> {
 * // The first type must be your class itself, others are interfaces.
 * class Circle: public implement <Circle, IShape>{
 * public:
 *      Circle(double _r):r(_r){}
 *      double area(){
 *          cout << "Circle: ";
 *          return 3.14 * r * r;
 *      }
 * private:
 *      double r;
 * };
 * class Rectangle: public implement <Rectangle, IShape> {
 * public:
 *      Rectangle(double _w, double _h):h(_h),w(_w){}
 *      double area() {
 *          cout << "Rectangle: ";
 *          return h * w;
 *      }
 * private:
 *      double h;
 *      double w;
 * };
 *  
 */

#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <typeinfo>
using namespace std;

/**
 *  Make member function ptr hashable, using GCC ABI specs.
 *  This does not consider built-in vf ptrs and thunk adjustments. They do not happen in "interfaces".
 *  For general mf ptrs in single inheritance contexts, higher half is not used.
 *  Lower half is the entrace of mf. Hashes can be generated on lower half of mf ptrs.
 */

template <typename C>
struct void_mf_ptr_t{
    template <typename F>
    void_mf_ptr_t(const F& f):p( reinterpret_cast<void(C::*)(void)>(f)) { }
    void_mf_ptr_t():p(0){ }
    bool operator==(const void_mf_ptr_t<C> &a) const { return p == a.p; }
    void(C::*p)(void);
};

template <typename C>
union void_mf_ptr_hash_helper {
    void *ptrs[ sizeof(void_mf_ptr_t<C>) / sizeof(void*) ];
    void_mf_ptr_t<C> p;
    void_mf_ptr_hash_helper(const void_mf_ptr_t<C> &_p):p(_p){ }
    size_t rehash() const noexcept { return hash<void*>()(*ptrs); }
};

template <typename C>
struct hash< void_mf_ptr_t<C> > {
    size_t operator() (const void_mf_ptr_t<C> &p) const noexcept {
        return void_mf_ptr_hash_helper(p).rehash();
    }
};

// -- Member function ptrs are now hashable.

// Common "interface" facilities
// interface is a CRTP template
// an interface should inherit interface and put itself as the type argument
// All types implementing the interface<IFaceType> will be recorded, with their concret vmfs
// This is like vtable in standard cxx. Only without vptr.
template <typename IFaceType>
struct interface{
    using mf_ptr_t = void_mf_ptr_t<IFaceType>;
    static size_t num_vmfs; // vmfs = Virtual Member Functions
    static size_t num_dts; // dts = Derived Types
    static unordered_map<type_index, size_t> dts; // map rtti -> type_id
    static unordered_map<const void*, size_t> objs; // map object address -> type_id
    static unordered_map<mf_ptr_t, size_t> vmf_idx; // map interface virtual function address -> function_id
    static vector<mf_ptr_t*> vmf_map; // virtual function entrance matrix [type_id][function_id]
    static bool add_dt(const type_index &t){
        if (auto f = dts.find(t); f == dts.end()){
            dts[t] = num_dts ++;
            vmf_map.push_back(new mf_ptr_t[num_vmfs]{});
            return true;
        }
        return false;
    }

    static bool add_obj(const type_index &t, const void *p){
        bool is_new_dt = add_dt(t);
        if (auto f = objs.find(p); f == objs.end())
            objs[p] = dts[t];
        return is_new_dt;
    }

    static bool set_vmf_handler(const type_index &t, mf_ptr_t vmf, size_t idx){
        auto &_vmf = vmf_map[dts[t]][idx];
        if (_vmf.p) return false; // not an empty slot, abort
        _vmf = vmf;
        return true;
    }

    template <typename IFaceFunc, typename... IFaceFuncs>
    static constexpr size_t add_vmfs(IFaceFunc f0, IFaceFuncs... fs){
        vmf_idx[ f0 ] = num_vmfs ++;
        if constexpr (sizeof... (fs) != 0)
            return add_vmfs(fs...);
        return num_vmfs;
    }

    template <size_t vmf_id = 0, typename IFaceFunc, typename... IFaceFuncs>
    static constexpr size_t set_all_handlers(const type_index &t, IFaceFunc f0, IFaceFuncs... fs){
        set_vmf_handler(t, f0, vmf_id);
        if constexpr (sizeof... (fs) != 0)
            return set_all_handlers<vmf_id + 1>(t, fs...);
        if (vmf_id >= num_vmfs)
            return -1;
        return vmf_id;
    }

    template <typename IFaceFunc>
    static IFaceFunc get_handler(const IFaceType *self, IFaceFunc vmf){
        auto &dt_idx = objs[self];
        auto &vf_idx = vmf_idx[vmf];
        auto &handler = vmf_map[dt_idx][vf_idx];
        return reinterpret_cast<IFaceFunc>(handler.p);
    }
};
template <typename IFaceType>
size_t interface<IFaceType>::num_vmfs = IFaceType::num_vmfs;
template <typename IFaceType>
size_t interface<IFaceType>::num_dts = 0;
template <typename IFaceType>
unordered_map<type_index, size_t> interface<IFaceType>::dts;
template <typename IFaceType>
unordered_map<const void*, size_t> interface<IFaceType>::objs;
template <typename IFaceType>
unordered_map< typename interface<IFaceType>::mf_ptr_t, size_t > interface<IFaceType>::vmf_idx;
template <typename IFaceType>
vector< typename interface<IFaceType>::mf_ptr_t* > interface<IFaceType>::vmf_map;

// Common facilities for runtime inits
// For any concret type implementing a serial of interfaces, a special implementation base is generated
// so that all implemented interfaces are connected to the concret type. Also, all implemented interfaces
// know the existance of the concret type.
template <typename ImplType, typename... IFaceTypes>
struct implement: public IFaceTypes... {
    implement(){
        add_obj<IFaceTypes...>(reinterpret_cast<ImplType*>(this));
        // Although I want these impl<...>() to run in the global inits phase. I can not find a way to do that.
        // If someone knows how, please tell me.
        if (!is_vmf_added){
            impl<IFaceTypes...>();
            is_vmf_added = true;
        }
    }
    template <typename ImplFaceType, typename... ImplFaceTypes>
    static constexpr size_t impl(){
        size_t r = ImplFaceType::template add_impl<ImplType>();
        if constexpr (sizeof... (ImplFaceTypes) == 0)
            return r;
        else
            return r + impl<ImplFaceTypes...>();
    }

    static bool is_vmf_added;

    template <typename ImplFaceType, typename... ImplFaceTypes>
    static void add_obj(ImplType *self){
        interface<ImplFaceType>::add_obj(typeid(ImplType), self);
        if constexpr (sizeof... (ImplFaceTypes) == 0)
            return;
        else
            add_obj<ImplFaceTypes...>(self);
    }

};
template <typename ImplType, typename... IFaceTypes>
bool implement<ImplType, IFaceTypes...>::is_vmf_added = false;

// I obtained the FOREACH macro snippets from
// https://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros

#define FE_0(WHAT, PREFIX)
#define FE_1(WHAT, PREFIX, X) WHAT(PREFIX, X)
#define FE_2(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_1(WHAT, PREFIX, __VA_ARGS__)
#define FE_3(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_2(WHAT, PREFIX, __VA_ARGS__)
#define FE_4(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_3(WHAT, PREFIX, __VA_ARGS__)
#define FE_5(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_4(WHAT, PREFIX, __VA_ARGS__)
#define FE_6(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_5(WHAT, PREFIX, __VA_ARGS__)
#define FE_7(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_6(WHAT, PREFIX, __VA_ARGS__)
#define FE_8(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_7(WHAT, PREFIX, __VA_ARGS__)
#define FE_9(WHAT, PREFIX, X, ...) WHAT(PREFIX, X),FE_8(WHAT, PREFIX, __VA_ARGS__)
#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,NAME,...) NAME
#define FOR_EACH(action, P, ...) \
  GET_MACRO(_0,__VA_ARGS__,FE_9,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1,FE_0)(action, P, __VA_ARGS__)

// FOR_EACH macros ends here

// These macros are for client codes to define interfaces easily

#define SCOPE(S, X) S::X

#define IFACE_BEGIN(interface_name) struct interface_name: public interface<interface_name> {                               \
using IBase = interface <interface_name>;

#define IFACE_END(interface_name,...)                                                                                       \
    static const size_t num_vmfs;                                                                                           \
    template <typename ImplType>                                                                                            \
    static constexpr size_t add_impl(){                                                                                     \
        IBase::add_dt(typeid(ImplType));                                                                                    \
        return IBase::set_all_handlers(typeid(ImplType), FOR_EACH(SCOPE, ImplType, __VA_ARGS__));                           \
    }                                                                                                                       \
};                                                                                                                          \
const size_t interface_name::num_vmfs = interface_name::IBase::add_vmfs(FOR_EACH(SCOPE, interface_name, __VA_ARGS__));

#define DEF_VF(interface_name, vf_name, ...)                                                                                \
    auto me = IBase::get_handler(this, interface_name::vf_name);                                                            \
    if (me == &interface_name::vf_name) throw runtime_error("not implemented");                                             \
    return (this->*(me))(__VA_ARGS__);

// interface macros end here

#endif // INTERFACE_H_INCLUDED
