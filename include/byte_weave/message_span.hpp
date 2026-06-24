// SPDX-License-Identifier: MIT

#pragma once
#include <byte_weave/endians.hpp>

namespace byte_weave
{
    template<typename Schema, std::size_t Extent = std::dynamic_extent>
    requires ((Extent == std::dynamic_extent) || (Schema::buffer_size <= Extent))
    class MessageSpan
    {
    public:
        using span_type = typename Schema::template span_type<Extent>;
    private:
        span_type         view;
    public:

        constexpr MessageSpan() noexcept
        requires detail::DynamicExtent<Extent>:
            view()
        {}
        
        template<typename Iter>
        explicit(Extent != std::dynamic_extent) 
        constexpr MessageSpan(Iter first, Iter last) noexcept:
            view(first, last)
        {

        }

        template<typename Range>
        requires (detail::ByteBuffer<Range> &&    
                !std::same_as<std::remove_cvref_t<Range>, MessageSpan<Schema, Extent>>)
        MessageSpan(Range& range) noexcept: 
            view(range)
        {

        }
        
        template<std::size_t ArrExtend>
        constexpr explicit MessageSpan(typename Schema::template base_buffer_type<ArrExtend>& arr) noexcept
            requires detail::CheckStaticArraySize<ArrExtend, Schema::buffer_size>:
                view(arr)
        {
            //std::cout<<"stat buff"<<std::endl;
        }

        MessageSpan(const MessageSpan& other) noexcept:
            view(other.view)
        {
            //std::cout<<"call ctor"<<std::endl;
        }

        MessageSpan(MessageSpan&& other):
            view(std::move(other.view))
        {
            // behavior thet complies with the std::span standart
            // other.view = span_type();   
        }

        MessageSpan& operator= (const MessageSpan& other) noexcept
        {
            if(this != &other)
            {
                view = other.view;
            }
            return *this;
        }
        MessageSpan& operator= (MessageSpan&& other) noexcept
        {
            if (this != &other)
            {
                // behavior thet complies with the std::span standart
                view = std::move(other.view);
            }
            return *this;
        }
        
        template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr void set(const typename Layout::template type<Tag>& value) noexcept
        {   
            Schema::template set<Layout, Tag, span_type, EPolicy>(view, value);
        }

        template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr typename Layout::template type<Tag> get() const noexcept
        {   
            return Schema::template get<Layout, Tag, span_type, EPolicy>(view);
        }
        
        template<typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr void set_primary(const typename Schema::Primary::template type<Tag>& value) noexcept
        {
            Schema::template set<typename Schema::Primary, Tag, span_type, EPolicy>(view, value);
        }

        template<typename Tag, typename EPolicy = EndiansPolicy::Native>
        constexpr typename Schema::Primary::template type<Tag> get_primary() const noexcept
        {
            return Schema::template get<typename Schema::Primary, Tag, span_type, EPolicy>(view);
        }
        
        constexpr std::size_t size() const noexcept
        {
            return view.size();
        }

        void clear() noexcept
        {
            fill(std::byte{0});
        }

        void fill(std::byte fill_val) noexcept
        {
            std::fill(view.begin(), view.end(), fill_val);
        }

        template<typename Range>
        void bind(Range& buffer)
        requires (detail::DynamicExtent<Extent>) && detail::ByteBuffer<Range>
        {
            view = std::span(buffer);
        }

        void bind(std::byte* storage_begin, std::size_t n)
        requires detail::DynamicExtent<Extent>
        {
            view = typename Schema::span_type(storage_begin, n);
        }

        template<typename Iter>
        requires (std::contiguous_iterator<Iter> && detail::DynamicExtent<Extent>)
        void bind(Iter first, Iter last)
        {
            view = span_type(first, last);
        }

        bool bind_check(std::byte* storage_begin, std::size_t n)
        requires detail::DynamicExtent<Extent>
        {
            if (Schema::buffer_size <= n)
            {
                view = typename Schema::span_type(storage_begin, Schema::buffer_size);
                return true;
            }
            return false;
        }

        template<typename Range>
        bool bind_check(Range& buffer)
        requires (detail::DynamicExtent<Extent>) && detail::ByteBuffer<Range>
        {
            if (Schema::buffer_size <= buffer.size())
            {
                view = span_type(buffer.data(), Schema::buffer_size);
                return true;
            }
            return false;
        }

        template<typename Iter>
        requires (std::contiguous_iterator<Iter> && detail::DynamicExtent<Extent>)
        bool bind_check(Iter first, Iter last)
        {
            if (Schema::buffer_size <= static_cast<std::size_t>(std::distance(first, last)))
            {
                view = span_type(first, first + Schema::buffer_size);
                return true;
            }
            return false;
        }
        
        typename Schema::template span_type<> span() noexcept
        {
            return view;
        }

        typename Schema::template const_span_type<> span() const noexcept
        {
            return view;
        }

        typename Schema::template const_span_type<> span(std::size_t first, std::size_t last) const noexcept
        {
            return view.subspan(first, last - first);
        }

        typename Schema::template span_type<> span(std::size_t first, std::size_t last) noexcept
        {
            return view.subspan(first, last - first);
        }

        template<std::size_t First, std::size_t Last>
        constexpr typename Schema::template span_type<Last - First> span() noexcept
        {
            return view.template subspan<First, Last - First>();
        }

        template<std::size_t First, std::size_t Last>
        constexpr typename Schema::template const_span_type<Last - First> span() const noexcept
        {
            return view.template subspan<First, Last - First>();
        }
        
        std::byte* data() noexcept 
        {
            return view.data();
        }

        const std::byte* data() const noexcept 
        {
            return view.data();
        }
        ~MessageSpan() = default;
    };

}