#pragma once

#include "RNG.h"
#include <cstddef>
#include <stdexcept>

template <typename T>
const T& RNG::sample(const std::vector<T>& pool) {
    if(pool.empty())
        throw std::out_of_range("RNG::sample: empty pool");

    std::uniform_int_distribution<std::size_t> index_dist(0, pool.size() - 1);
    return pool[index_dist(generator)];
}
