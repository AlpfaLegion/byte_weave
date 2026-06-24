// SPDX-License-Identifier: MIT

#pragma once
#include <cstddef>
#include <array>
#include <span>
#include <byte_weave/endians.hpp>
#include <byte_weave/detail/traits.hpp>
#include <tuple>
#include <bit>
#include <cstring>

namespace byte_weave
{
    struct LayoutCommonProp
    {
        using base_element_buffer_type = std::byte;

        template<std::size_t Nm>
        using base_buffer_type = std::array<base_element_buffer_type, Nm>;

        template<std::size_t Extent = std::dynamic_extent>
        using span_type = std::span<base_element_buffer_type, Extent>;

        template<std::size_t Extent = std::dynamic_extent>
        using const_span_type = std::span<const base_element_buffer_type, Extent>;
    
    };

    struct LayoutEmpty
    {
        using type = void;

        constexpr static std::size_t size = 0;

        static constexpr  std::size_t offset(){return 0;}
    };


    template<typename... Fields>
    struct Layout
    {
        using fields = std::tuple<Fields...>;

        using self_type = Layout<Fields...>;

        template<typename Tag>
        using type = detail::TypeOf_t<Tag, Fields...>;
        
        constexpr static std::size_t size = (Fields::size + ...);

        using buffer_type = LayoutCommonProp::base_buffer_type<size>;
        // static constexpr buffer_type make_buffer() noexcept {return {};}
        
        template<typename Tag>
        static constexpr  std::size_t offset()
        {
            static_assert(detail::contains_v<Tag, Fields...>, "Tag not found");
            return  detail::layout_offset_of_v<Tag, Fields...>;
        }

        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Unsupported mixed-endian platform");
        static_assert(detail::unique_tags_v<Fields...>, "Duplicate field tags in Layout");

        template<typename EPolicy>
        static constexpr bool need_reverse_v = 
            ((std::endian::native == std::endian::little) && (std::is_same_v<EPolicy, EndiansPolicy::Big>)) ||
            ((std::endian::native == std::endian::big) && (std::is_same_v<EPolicy, EndiansPolicy::Little>));


        template<typename Tag, typename BufferType, std::size_t OffsetBuf = 0, typename EPolicy = EndiansPolicy::Native>
        requires detail::WritableByteBuffer<BufferType>
        static constexpr void set(BufferType& buff, const type<Tag>& value) noexcept
        {
            static_assert(detail::contains_v<Tag, Fields...>, "Tag not found");
            static_assert(is_field_compatible_endians_v<type<Tag>, EPolicy>,"Field has invalid type to selected endian policy");

            constexpr std::size_t off = offset<Tag>();
            constexpr std::size_t sff = sizeof(type<Tag>);
            //

            if constexpr (detail::has_static_size<BufferType>) 
            {
            static_assert(OffsetBuf + off + sff <= detail::get_static_size<BufferType>(), "Out of range buffer");
            }
            
            std::memcpy(buff.data() + OffsetBuf + off, &value, sff);

            if constexpr ((sizeof(type<Tag>) > 1) && need_reverse_v<EPolicy>)
            {
                byteswap<sff>(buff.data() + OffsetBuf + off);
            }
        }

        template<typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
        requires detail::WritableByteBuffer<BufferType>
        static constexpr void set(BufferType& buff, const type<Tag>& value) noexcept
        {
            set<Tag, BufferType, 0, EPolicy>(buff, value);
        }

        template<typename Tag, typename BufferType, std::size_t OffsetBuf = 0, typename EPolicy = EndiansPolicy::Native>
        requires detail::ReadableByteBuffer<BufferType>
        static constexpr type<Tag> get(const BufferType& buff) noexcept
        {
            static_assert(detail::contains_v<Tag, Fields...>, "Tag not found");
            static_assert(is_field_compatible_endians_v<type<Tag>, EPolicy>,"Field has invalid type to selected endian policy");

            constexpr std::size_t off = offset<Tag>();
            constexpr std::size_t sff = sizeof(type<Tag>);
            //
            if constexpr (detail::has_static_size<BufferType>) 
            {
                static_assert(OffsetBuf + off + sff <= detail::get_static_size<BufferType>(), "Out of range buffer");
            }

            type<Tag> value;

            std::memcpy(&value, buff.data() + OffsetBuf + off, sff);

            if constexpr ((sizeof(type<Tag>) > 1) && need_reverse_v<EPolicy>)
            {
                byteswap(value);
            }
            return value;
        }

        template<typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
        requires detail::ReadableByteBuffer<BufferType>
        static constexpr type<Tag> get(const BufferType& buff) noexcept
        {
            return get<Tag, BufferType, 0, EPolicy>(buff);
        }
    };
}