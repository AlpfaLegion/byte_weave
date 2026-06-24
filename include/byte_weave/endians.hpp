// SPDX-License-Identifier: MIT

#pragma once
#include <algorithm>
#include <byte_weave/detail/traits.hpp>

namespace byte_weave 
{
    struct EndiansPolicy
    {
        struct Little{};
        struct Big{};
        struct Native{};
    };

    template<std::size_t N>
    void byteswap(std::byte* p) noexcept
    {
        if constexpr (N <= 1)
            return;
        else if constexpr(N == 2)
        {
            std::byte tmp = p[0]; p[0] = p[1]; p[1] = tmp;
        } 
        else if constexpr(N == 4)
        {
            std::byte tmp = p[3]; p[3] = p[0]; p[0] = tmp;
                    tmp = p[2]; p[2] = p[1]; p[1] = tmp;
        }
        else if constexpr(N == 8)
        {
            std::byte tmp = p[7]; p[7] = p[0]; p[0] = tmp;
                    tmp = p[6]; p[6] = p[1]; p[1] = tmp;
                    tmp = p[5]; p[5] = p[2]; p[2] = tmp;
                    tmp = p[4]; p[4] = p[3]; p[3] = tmp;
        }
        else if constexpr(N == 16)
        {
            std::byte tmp = p[15]; p[15] = p[0]; p[0] = tmp;
                    tmp = p[14]; p[14] = p[1]; p[1] = tmp;
                    tmp = p[13]; p[13] = p[2]; p[2] = tmp;
                    tmp = p[12]; p[12] = p[3]; p[3] = tmp;
                    tmp = p[11]; p[11] = p[4]; p[4] = tmp;
                    tmp = p[10]; p[10] = p[5]; p[5] = tmp;
                    tmp = p[9];  p[9]  = p[6]; p[6] = tmp;
                    tmp = p[8];  p[8]  = p[7]; p[7] = tmp;
        }
        else    
        {
            std::ranges::reverse(p, p + N);
        }
    }

    template<typename T>
    concept ByteSwapType = detail::is_scalar_field_v<T>;

    template<typename T>
    requires ByteSwapType<T>
    inline void byteswap(T& val) noexcept
    {
        byteswap<sizeof(T)>(reinterpret_cast<std::byte*>(&val));
    }

    //====================================Traits Endian policy ====================================
    template<typename T, typename EPolicy>
    struct is_field_compatible_endians;

    template<typename T>
    struct is_field_compatible_endians<T, EndiansPolicy::Native>: std::bool_constant<
                                                                            detail::is_scalar_field_v<T> ||
                                                                            detail::is_unsafe_native_blob_v<T>
    >
    {};

    template<typename T>
    struct is_field_compatible_endians<T, EndiansPolicy::Little>: std::bool_constant<detail::is_scalar_field_v<T>>
    {};

    template<typename T>
    struct is_field_compatible_endians<T, EndiansPolicy::Big>: std::bool_constant<detail::is_scalar_field_v<T>>
    {};

    template<typename T, typename EPolicy>
    inline constexpr bool is_field_compatible_endians_v = is_field_compatible_endians<T, EPolicy>::value;
    //==========================================================================================

}