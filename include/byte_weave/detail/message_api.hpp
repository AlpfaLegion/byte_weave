// SPDX-License-Identifier: MIT

#pragma once
#include <byte_weave/detail/traits.hpp>

namespace byte_weave 
{
    namespace detail 
    {
        // Internal implementation details.
        // Not part of the public API.
        // May change without notice.

        // -------------------message size-------------------
        template<typename PrimaryLayout, typename... AlternativeLayouts>
        struct MessageSizeApi
        {
        private:
            template<typename ActiveLayout>
            struct MessageSize
            {
                static_assert(detail::is_contains_layout_v<ActiveLayout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");
                static constexpr std::size_t value = std::is_same_v<ActiveLayout, PrimaryLayout> ?
                    PrimaryLayout::size : PrimaryLayout::size + ActiveLayout::size;
            };
        public:
            template<typename AlternativeLayout>
            static constexpr std::size_t message_size = MessageSize<AlternativeLayout>::value;
        };

        template<typename PrimaryLayout>
        struct MessageSizeApi <PrimaryLayout>
        {
            static constexpr std::size_t message_size = PrimaryLayout::size;
        };

        //----------------------------------------------------------------------------

        
    }
}