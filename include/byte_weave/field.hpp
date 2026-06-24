// SPDX-License-Identifier: MIT

#pragma once
#include <byte_weave/detail/traits.hpp>

namespace byte_weave 
{
    template<typename Tag, typename T>
    struct FieldBase
    {
        using tag = Tag;
        using type = T;
        static constexpr std::size_t size = sizeof(T);
    };

    template<typename Tag, typename T>
    struct Field: FieldBase<Tag, T>
    {
        static_assert(detail::is_scalar_field_v<T>, "Field type must be scalar type");
    };

    template<typename Tag, typename T>
    struct FieldNativeBlob: FieldBase<Tag, T>
    {
        static_assert(detail::is_unsafe_native_blob_v<T>, "Field type must be native blob type, if use native blob check alignment ant paddings");
    };

    template<typename Tag, typename T, std::size_t ExpectedSize>
    struct FieldPackedBlob: FieldNativeBlob<Tag, T>
    {
        static_assert(alignof(T) == 1, "Packed blob must have alignof(T) == 1");

        static_assert(sizeof(T) == ExpectedSize, "Packed blob size does not match expected size");
    };
    
}