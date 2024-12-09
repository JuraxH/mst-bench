#pragma once

#include <boost/container_hash/hash_fwd.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <bit>

// floor of log2(n)
template<typename T>
T log2(T n) {
    if (n == 0) {
        return 0;
    }
    return std::bit_width(n) - 1;
}


template<typename T>
std::string dump_vector(const std::vector<T>& arr) {
    auto res = std::string{};
    for (const auto& elem : arr) {
        res += std::to_string(elem) + " ";
    }
    return res + '\n';
}

// how is this not in std
template <typename T1, typename T2>
struct PairHash {
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.first);  // Combine hash of the first element
        boost::hash_combine(seed, p.second); // Combine hash of the second element
        return seed;
    }
};

// who added ranges to c++20 without collect????????
std::vector<std::string> collect(auto iter) {
    std::vector<std::string> res{};
    for (const auto& part : iter) {
        std::string s{};
        // WTF
        for (auto e : part) {
            s.push_back(e);
        }
        res.push_back(s);
    }
    return res;
}


inline bool is_close(double a, double b, double tol=0.001) {
    return std::fabs(a - b) <= tol;
}
