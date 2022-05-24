//
// Created by Caio W on 2/2/22.
//

#ifndef CREBON_TYPES_HPP
#define CREBON_TYPES_HPP

#include <future>
#include <memory>

using float_t = float;

namespace cr {
template <typename T> using compute_resource = std::future<T>;
}

#endif // CREBON_TYPES_HPP
