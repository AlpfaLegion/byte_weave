// SPDX-License-Identifier: MIT

#pragma once
#include <type_traits>
#include <cstddef>
#include <ranges>
#include <concepts>

namespace byte_weave
{
    namespace detail
    {

        // Internal implementation details.
        // Not part of the public API.
        // May change without notice.

        template<typename T>
        struct is_scalar_field: public std::bool_constant<
            (std::is_integral_v<T> && !std::is_same_v<T, bool>) ||
            std::is_floating_point_v<T> ||
            std::is_enum_v<T>>
        {};
        template<typename T>
        inline constexpr bool is_scalar_field_v = is_scalar_field<T>::value;

        template<typename T>
        struct is_unsafe_native_blob: public std::bool_constant<
            std::is_trivially_copyable_v<T> &&
            std::is_standard_layout_v<T>
        >
        {};

        template<typename T>
        inline constexpr bool is_unsafe_native_blob_v = 
            is_unsafe_native_blob<T>::value && 
            !is_scalar_field_v<T>;


        //-------contains----------------
        template<typename Tag, typename... Fields>
        struct contains
        {
            static constexpr bool value = (std::is_same_v<Tag, typename Fields::tag> || ...);
        };

        template<typename Tag, typename... Fields>
        inline constexpr bool contains_v = contains<Tag, Fields...>::value;

        // ------------------------typeof-----------------------
        template<typename Tag, typename... Fields>
        struct TypeOf;

        struct TypeOfTagNotFound{};

        template<typename Tag>
        struct TypeOf<Tag>
        {
            using type = TypeOfTagNotFound;
        };

        template<typename Tag, typename U, typename... Tail>
        struct TypeOf<Tag, U, Tail...>
        {
            using type = std::conditional_t<std::is_same_v<Tag, typename U::tag>, typename U::type, typename TypeOf<Tag, Tail...>::type>;
        };

        template<typename Tag, typename... Fields>
        using TypeOf_t = typename TypeOf<Tag, Fields...>::type;
        // -----------------------------------------------

        // ------------------------offsetof-----------------------
        template<typename Tag, typename... Fields>
        struct offsetof;

        template<typename Tag>
        struct offsetof<Tag>
        {
            static constexpr std::size_t value = 0;
        };

        template<typename Tag, typename U, typename... Tail>
        struct offsetof<Tag, U, Tail...>
        {
            static constexpr std::size_t value = std::is_same_v<Tag, typename U::tag> ? 0 : (U::size + offsetof<Tag, Tail...>::value);
        };

        template<typename Tag, typename...Fields>
        constexpr std::size_t offsetof_v = offsetof<Tag, Fields...>::value;

        template<typename Tag, typename... Field>
        struct layout_offset_of
        {
            static constexpr std::size_t value = offsetof_v<Tag, Field...>;
        };

        template<typename Tag, typename... Fields>
        inline constexpr std::size_t layout_offset_of_v = layout_offset_of<Tag, Fields...>::value;

        // -----------------------unique---------------------------------
        template<typename... Args>
        struct unique_types;

        template<>
        struct unique_types<>: std::true_type
        {};

        template<typename T>
        struct unique_types<T>: std::true_type
        {};

        template<typename T, typename... Args>
        struct unique_types<T, Args...>: std::bool_constant<(!(std::is_same_v<T, Args> || ...)) && unique_types<Args...>::value>
        {};

        template<typename... Fields>
        inline constexpr bool unique_tags_v = unique_types<typename Fields::tag ...>::value;

        template<typename... Layouts>
        inline constexpr bool unique_layouts_v = unique_types<Layouts...>::value;

        template<typename PrimaryLayout, typename... AlternativeLayouts>
        inline constexpr bool unique_schema_layouts_v = unique_types<PrimaryLayout, AlternativeLayouts...>::value;

        template<typename CheckLayout, typename PrimaryLayout, typename... AlternativeLayouts>
        struct ContainsLayout
        {
            static constexpr bool check_primary = std::is_same_v<CheckLayout, PrimaryLayout>;
            static constexpr bool check_alternative = (std::is_same_v<CheckLayout, AlternativeLayouts> || ...);
            static constexpr bool value = check_primary || check_alternative;
        };

        template<typename CheckLayout, typename PrimaryLayout, typename... AlternativeLayouts>
        inline constexpr bool is_contains_layout_v = ContainsLayout<CheckLayout, PrimaryLayout, AlternativeLayouts...>::value;

        template<typename Range>
        concept ByteBuffer = 
            (std::ranges::contiguous_range<Range>) &&
            (std::ranges::sized_range<Range>) &&
            //(std::ranges::borrowed_range<Range>) &&
            (std::is_same_v<std::ranges::range_value_t<Range>, std::byte>) &&
            requires(std::remove_cvref_t<Range> b)
        {
            b.data();
            //{b.data()} -> std::convertible_to<std::byte*>;
            {b.size()} -> std::convertible_to<std::size_t>;
        };

        template<typename T>
        concept ReadableByteBuffer = 
            ByteBuffer<T> &&
            requires(T buff, const T cbuff, std::size_t idx)
        {
            {buff.data()} -> std::convertible_to<const std::byte*>;
            {cbuff.data()} -> std::convertible_to<const std::byte*>;

            {buff.size()} -> std::convertible_to<std::size_t>;
            {cbuff.size()} -> std::convertible_to<std::size_t>;
            
            requires std::same_as<std::remove_cvref_t<decltype(buff[idx])>, std::byte>;
            requires std::same_as<std::remove_cvref_t<decltype(cbuff[idx])>, std::byte>;
        };

        template<typename T>
        concept WritableByteBuffer = 
            ByteBuffer<T> &&
            requires(T buff, std::size_t  idx)
        {
            {buff.data()} -> std::same_as<std::byte*>;
            {buff.size()} -> std::convertible_to<std::size_t>;

            buff[idx] = std::byte{};
        };

        // static size
        template<typename T>
        struct static_size;

        template<typename T, std::size_t Nm>
        struct static_size<std::array<T, Nm>>: std::integral_constant<std::size_t, Nm>{};

        template<typename T, std::size_t Nm>
        struct static_size<T[Nm]>: std::integral_constant<std::size_t, Nm> {};

        template<typename T, std::size_t Nm>
        requires (Nm != std::dynamic_extent)
        struct static_size<std::span<T, Nm>>: std::integral_constant<std::size_t, Nm>{};

        template<typename T>
        inline constexpr std::size_t static_size_v = static_size<std::remove_cvref_t<T>>::value;

        template<typename BufferType>
        concept has_static_size = 
            requires { typename static_size<std::remove_cvref_t<BufferType>>::type; };

        template<typename StaticBufferType>
        constexpr std::size_t get_static_size()
        {
            using CleanType = std::remove_cvref_t<StaticBufferType>;
            if constexpr (std::is_array_v<CleanType>) 
            {
                return std::extent_v<CleanType>;
            }
            else
            {
                return static_size_v<CleanType>;
            }
        }

        template<std::size_t Extent>
        concept DynamicExtent = (Extent == std::dynamic_extent);

        template<std::size_t Nm, std::size_t SizeMsg>
        concept CheckStaticArraySize = (Nm >= SizeMsg);
    }
}