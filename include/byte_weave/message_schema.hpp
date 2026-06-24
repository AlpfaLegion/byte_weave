// SPDX-License-Identifier: MIT

#pragma once
#include <byte_weave/layout.hpp>
#include <byte_weave/detail/message_api.hpp>

namespace byte_weave 
{
    template<typename PrimaryLayout, typename... AlternativeLayouts>
    struct MessageSchemaBase: public LayoutCommonProp
    {
    private:
        template<typename T, typename... Args>
        static constexpr std::size_t max_size()
        {
            std::size_t value = T::size;
            ((value = value > Args::size ? value : Args::size), ...);
            return value;
        }

        static constexpr std::size_t get_alternative_max_size()
        {
            if constexpr (sizeof...(AlternativeLayouts) == 0)
            {
                return 0;
            }
            else
            {
                return max_size<AlternativeLayouts...>();
            }
        }

    public:
        static_assert(detail::unique_schema_layouts_v<PrimaryLayout, AlternativeLayouts...>, "Duplicate layouts in MessageSchema");

        static constexpr std::size_t max_alt_size = get_alternative_max_size();

        static constexpr std::size_t buffer_size = PrimaryLayout::size + max_alt_size;

        using buffer_type = LayoutCommonProp::base_buffer_type<buffer_size>;
        
        template<std::size_t Nm>
        using any_buffer_type = LayoutCommonProp::base_buffer_type<Nm>;

        template<std::size_t Extent = std::dynamic_extent>
        using span_type = LayoutCommonProp::span_type<Extent>;

        using Primary = PrimaryLayout;

        static constexpr buffer_type make_buffer() noexcept {return {};}
        
        template<std::size_t Nm>
        static constexpr base_buffer_type<Nm> make_buffer() noexcept {return {};}

        // template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
        // static constexpr void set(buffer_type& buffer, const typename Layout::template type<Tag>& value) noexcept
        // {
        //     static_assert(is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

        //     constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
            
        //     Layout::template set<Tag, buffer_type, offset_buf, EPolicy>(buffer, value);
        // }

        // template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
        // static constexpr typename Layout::template type<Tag> get(const buffer_type& buffer) noexcept
        // {
        //     static_assert(is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

        //     constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
            
        //     return Layout::template get<Tag, buffer_type, offset_buf, EPolicy>(buffer);
        // }

        template<typename Layout, typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
        static constexpr void set(BufferType& buffer, const typename Layout::template type<Tag>& value) noexcept
        {
            static_assert(detail::is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

            constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
            
            Layout::template set<Tag, BufferType, offset_buf, EPolicy>(buffer, value);
        }

        template<typename Layout, typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
        static constexpr typename Layout::template type<Tag> get(const BufferType& buffer) noexcept
        {
            static_assert(detail::is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

            constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
            
            return Layout::template get<Tag, BufferType, offset_buf, EPolicy>(buffer);
        }
    };
    // -------------------------------------Api-------------------------------------



    //--------------------------------------------------------------------------------------

    template<typename PrimaryLayout, typename... AlternativeLayouts>
    struct MessageSchema: 
        public MessageSchemaBase<PrimaryLayout, AlternativeLayouts...>,
        public detail::MessageSizeApi<PrimaryLayout, AlternativeLayouts...>
    {};

    template<typename PrimaryLayout, typename... AlternativeLayouts>
    using CompositeMessage = MessageSchema<PrimaryLayout, AlternativeLayouts...>;

    template<typename Layout>
    using SingleLayoutMessage = MessageSchema<Layout>;

    using EmptyMessage = MessageSchema<LayoutEmpty>;
}