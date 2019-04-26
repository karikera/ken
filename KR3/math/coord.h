#pragma once

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <initializer_list>

#include <memory.h>
#include <cmath>

#include "../main.h"
#include "coord/vector.h"
#include "coord/vector_data.inl"
#include "coord/vector_method.inl"
#include "coord/vector.inl"

#include "coord/quaternion.h"

#include "coord/matrix.h"
#include "coord/matrix_data.inl"
#include "coord/matrix_method.inl"

extern template class kr::math::vector<kr::byte, 4>;
extern template class kr::math::vector<short, 2>;
extern template class kr::math::vector<float, 2>;
extern template class kr::math::vector<float, 3>;
extern template class kr::math::vector<float, 4>;
extern template class kr::math::vector<int, 2>;
extern template class kr::math::vector<int, 3>;
extern template class kr::math::vector<int, 4>;