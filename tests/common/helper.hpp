#pragma once
#include <cstddef>
#include <cstring>

struct TestMsgSchemas
{
    struct Header
    {
        struct Tag1{};
        struct Tag2{};
        struct Tag3{};
        struct Tag4{};
        struct Tag5{};

        using Field1 = byte_weave::Field<Tag1, int>;
        using Field2 = byte_weave::Field<Tag2, std::uint64_t>;
        using Field3 = byte_weave::Field<Tag3, float>;
        using Field4 = byte_weave::Field<Tag4, double>;
        using Field5 = byte_weave::Field<Tag5, std::uint8_t>;
        
        using Layout = byte_weave::Layout<Field1, Field2, Field3, Field4, Field5>;

        // for test
        static constexpr std::size_t size_off = sizeof(Field1::type) + sizeof(Field2::type) + sizeof(Field3::type) + sizeof(Field4::type) + sizeof(Field5::type);
    }; // 

    struct Body1
    {
        struct Tag1{};
        struct Tag2{};
  
        using Field1 = byte_weave::Field<Tag1, int>;
        using Field2 = byte_weave::Field<Tag2, std::uint64_t>;

        using Layout = byte_weave::Layout<Field1, Field2>;
        static constexpr std::size_t size_off = sizeof(Field1::type) + sizeof(Field2::type);
    };

    struct Body2
    {
        struct Tag3{};
        struct Tag4{};
        
        using Field3 = byte_weave::Field<Tag3, double>;
        using Field4 = byte_weave::Field<Tag4, double>;
        using Layout = byte_weave::Layout<Field3, Field4>;
        static constexpr std::size_t size_off = sizeof(Field3::type) + sizeof(Field4::type) ;
    };

    struct Body3
    {
        struct Tag5{};

        using Field5 = byte_weave::Field<Tag5, std::uint16_t>;
        using Layout = byte_weave::Layout<Field5>;
        static constexpr std::size_t size_off = sizeof(Field5::type);
    };
    using Schema = byte_weave::CompositeMessage<Header::Layout, Body1::Layout, Body2::Layout, Body3::Layout>;
    using SingleSchema = byte_weave::SingleLayoutMessage<Header::Layout>;
};