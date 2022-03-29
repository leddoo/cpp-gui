#pragma once

#pragma warning(disable: 4458) // shadow class members
#pragma warning(disable: 4459) // shadow globals


#define UNUSED(x) ((void)x)

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define _MY_CONCAT(a, b) a##b
#define CONCAT(a, b) _MY_CONCAT(a, b)


#define NOMINMAX

#include <cassert>

#include <string>
#include <vector>
#include <array>
#include <cstdint>

using Bool = bool;

using Uint8  = uint8_t;
using Uint16 = uint16_t;
using Uint32 = uint32_t;
using Uint64 = uint64_t;
using Uint   = uintptr_t;

using Sint8  = int8_t;
using Sint16 = int16_t;
using Sint32 = int32_t;
using Sint64 = int64_t;
using Sint   = intptr_t;

using Float32 = float;
using Float64 = double;


using String = std::string;

template <typename T, Uint n>
using Array = std::array<T, n>;

template <typename T>
using List = std::vector<T>;


struct Unimplemented : public std::exception {};
struct Unreachable : public std::exception {};
struct Invalid_Dynamic_Type : public std::exception {};


template <typename F>
struct _Defer {
    F f;

    _Defer(F f) : f(f) {}
    ~_Defer() { f(); }
};

struct _Defer_Maker {
    template<typename F>
    _Defer<F> operator+(F &&f)
    {
        return _Defer<F>(f);
    }
};

#define defer auto CONCAT(_defer, __LINE__) = _Defer_Maker {} + [&]()



#define BYTE_TO_UINT_COUNT(count) (((count) + (  sizeof(Uint) - 1)) /    sizeof(Uint))
#define BIT_TO_UINT_COUNT(count)  (((count) + (8*sizeof(Uint) - 1)) / (8*sizeof(Uint)))


template <typename T>
struct Range {
    T _begin;
    T _end;

    T count() const { return this->_end - this->_begin; }
};

template <typename T>
struct Range_Iterator {
    T value;

    T operator*() const { return this->value; }

    void operator++() { this->value += 1; }

    Bool operator!=(const Range_Iterator<T>& other) const {
        return this->value < other.value;
    }
};

template <typename T>
Range_Iterator<T> begin(const Range<T>& range) {
    return Range_Iterator<T> { range._begin };
}

template <typename T>
Range_Iterator<T> end(const Range<T>& range) {
    return Range_Iterator<T> { range._end };
}


template <typename B, typename A>
B safe_dynamic_cast(A a) {
    auto b = dynamic_cast<B>(a);
    if(b == nullptr) {
        throw Invalid_Dynamic_Type();
    }
    return b;
}


template <typename B, typename A, typename F>
List<B> map_list(const List<A>& list, const F& f) {
    auto result = List<B>();
    result.reserve(list.size());

    for(const auto& e : list) {
        result.push_back(f(e));
    }

    return result;
}

template <typename B, typename A>
List<B> map_list_cast(const List<A>& list) {
    return map_list<B>(list, [](auto a) { return (B)(a); });
}

template <typename B, typename A>
List<B> map_list_safe_dynamic_cast(const List<A>& list) {
    return map_list<B>(list, [](auto a) { return safe_dynamic_cast<B>(a); });
}


template <typename T>
void safe_delete(T** pointer) {
    if((*pointer) != nullptr) {
        delete (*pointer);
        (*pointer) = nullptr;
    }
}


template <typename Container, typename T>
bool contains(const Container& container, const T& value) {
    return container.find(value) != container.end();
}


template <typename T>
inline Bool is_power_of_2(T value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}

inline Uint aligned_pointer(Uint pointer, Uint alignment) {
    assert(is_power_of_2(alignment));
    return (pointer + (alignment - 1u)) & ~(alignment - 1u);
}

inline Uint alignment_offset(Uint pointer, Uint alignment) {
    auto aligned = aligned_pointer(pointer, alignment);
    auto offset  = aligned - pointer;
    return offset;
}



template <typename T>
struct Distinct {
    T value;

    Distinct() = default;
    explicit constexpr Distinct(T value) : value(value) {}

    Bool operator< (Distinct<T> other) const { return this->value <  other.value; }
    Bool operator<=(Distinct<T> other) const { return this->value <= other.value; }
    Bool operator> (Distinct<T> other) const { return this->value >  other.value; }
    Bool operator>=(Distinct<T> other) const { return this->value >= other.value; }
    Bool operator==(Distinct<T> other) const { return this->value == other.value; }
    Bool operator!=(Distinct<T> other) const { return this->value != other.value; }
};

#define def_distinct(name, base)                                            \
    struct name : public Distinct<base> {                                   \
        using Distinct<base>::Distinct;                                     \
    };


