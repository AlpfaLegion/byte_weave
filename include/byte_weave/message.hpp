// SPDX-License-Identifier: MIT

#pragma once
#include <byte_weave/message_view.hpp>

namespace byte_weave
{
    template<typename Schema>
    class Message: public MessageView<Schema>
    {
    public:
        using Base = MessageView<Schema>; 
        using BufferType = typename Schema::buffer_type;
    private:
        BufferType         m_buffer;

    public:
        Message(): Base(), m_buffer()
        {
            Base::bind(m_buffer);
        }
        Message(const Message& other):
            Base(),
            m_buffer(other.m_buffer)
        {
            Base::bind(m_buffer);
        }
        Message(Message&& other) noexcept:
            Base(),
            m_buffer(std::move(other.m_buffer))
        {
            Base::bind(m_buffer);
        }

        Message& operator=(const Message& other)
        {
            if (this != &other)
            {
                m_buffer = other.m_buffer;
                Base::bind(m_buffer);
            }
            return *this;
        }
        Message& operator=(Message&& other)
        {
            if(this != &other)
            {
                m_buffer = std::move(other.m_buffer);
                Base::bind(m_buffer);
            }
            return *this;
        }

        void bind(const BufferType& other) = delete;
        
        bool valid() const noexcept = delete;

        ~Message() = default;
    };
}