//
// Created by reg on 7/29/22.
//

#ifndef CELLSIM_VECS_HPP
#define CELLSIM_VECS_HPP

#include <type_traits>

namespace CSIM::utils {

template <typename T>
requires std::is_arithmetic<T>::value struct Vec4 {
	T x = static_cast<T>(0);
	T y = static_cast<T>(0);
	T z = static_cast<T>(0);
	T w = static_cast<T>(0);
};

template <typename T>
requires std::is_arithmetic<T>::value struct Vec2 {
	T x = static_cast<T>(0);
	T y = static_cast<T>(0);
};

} // namespace CSIM::utils

#endif // CELLSIM_VECS_HPP
