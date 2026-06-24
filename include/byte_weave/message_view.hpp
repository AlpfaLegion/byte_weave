// SPDX-License-Identifier: MIT

#pragma once
#include <byte_weave/endians.hpp>

namespace byte_weave 
{
    template<typename Schema>
    class MessageView
    {
    public:
        using buffer_type = typename Schema::buffer_type;

    private: 
        buffer_type*   view;
    protected:
        MessageView(): view(nullptr){}

    public:
        MessageView(typename Schema::buffer_type& buffer):
            view(&buffer)
        {}

        MessageView(const MessageView& other):
            view(other.view)
        {}

        MessageView(MessageView&& other):
            view(other.view)
        {
            other.view = nullptr;
        }

        MessageView& operator= (const MessageView& other) noexcept
        {
            if(this != &other)
            {
                view = other.view;
            }
            return *this;
        }
        MessageView& operator= (MessageView&& other) noexcept
        {
            if (this != &other)
            {
                view = std::move(other.view);
                other.view = nullptr;
            }
            return *this;
        }
        
        template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr void set(const typename Layout::template type<Tag>& value) noexcept
        {   
            Schema::template set<Layout, Tag, buffer_type, EPolicy>(*view, value);
        }

        template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr typename Layout::template type<Tag> get() const noexcept
        {   
            return Schema::template get<Layout, Tag, buffer_type, EPolicy>(*view);
        }
        
        template<typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr void set_primary(const typename Schema::Primary::template type<Tag>& value) noexcept
        {
            Schema::template set<typename Schema::Primary, Tag, buffer_type, EPolicy>(*view, value);
        }

        template<typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr typename Schema::Primary::template type<Tag> get_primary() const noexcept
        {
            return Schema::template get<typename Schema::Primary, Tag, buffer_type, EPolicy>(*view);
        }

        constexpr const buffer_type& buffer() const noexcept{return *view;}

        constexpr buffer_type& buffer() noexcept {return *view;}

        constexpr std::size_t capacity() const noexcept { return view ? view->size() : 0;}
        
        template<typename ActiveAlternativeLayout>
        constexpr std::size_t size() const noexcept
        {
            return Schema::template message_size<ActiveAlternativeLayout>;
        }

        // invariant of class
        bool valid() const noexcept
        {
            return view;
        }

        constexpr void clear() noexcept
        {
            view->fill(std::byte(0));
        }

        void fill(std::byte fill_val) noexcept
        {
            view->fill(fill_val);
        }
        
        void bind(buffer_type& other) noexcept
        {
            view = &other;
        }

        std::span<std::byte> span(std::size_t first, std::size_t last) noexcept
        {
            return std::span(*view).subspan(first, last - first);
        }

        template<std::size_t First, std::size_t Last>
        std::span<std::byte, Last - First> span() noexcept
        {
            return std::span(*view).template subspan<First, Last - First>();
        }
        
        ~MessageView() = default;
    };
}