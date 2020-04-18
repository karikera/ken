#pragma once

#include "value.h"

namespace kr
{
	namespace meta
	{
        namespace _pri_
        {
            struct FalseDummy {};
            template<typename T> FalseDummy operator== (const T&, const T&);

            template<typename T>
            using HasEquals = bool_value<!is_same<decltype(declval<T>() == declval<T>), FalseDummy>::value>;
        }

        template <typename T>
        using has_equals = _pri_::HasEquals<T>;
	}
}