#ifndef TIME_H
#define TIME_H

#include <vector>
#include "tsc.h"

using std::vector;

template<typename F>
constexpr auto cycles(const F& func, vector<uint64_t>* V) {
    return [func, V](auto&&... args) {
        uint64_t start = read_tsc();
        auto result = func(std::forward<decltype(args)>(args)...);
        uint64_t end = read_tsc();
        V->push_back(end-start);
        return result;
    };
}

#endif
