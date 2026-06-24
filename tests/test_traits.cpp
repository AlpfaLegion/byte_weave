#include <cstdint>
#include <array>
#include <vector>
#include <list>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <byte_weave/byte_weave.hpp>


struct TestLayout
{
    struct Tag1{};
    struct Tag2{};
    struct Tag3{};
    struct Tag4{};
    struct Tag5{};
    struct Unknow{};

    using Field1 = byte_weave::Field<Tag1, int>;
    using Field2 = byte_weave::Field<Tag2, std::uint64_t>;
    using Field3 = byte_weave::Field<Tag3, float>;
    using Field4 = byte_weave::Field<Tag4, double>;
    using Field5 = byte_weave::Field<Tag5, std::uint8_t>;
    
    using Layout = byte_weave::Layout<Field1, Field2, Field3, Field4, Field5>;

};

TEST_CASE("Contains trait")
{
    STATIC_REQUIRE(byte_weave::detail::contains_v<TestLayout::Tag1, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>);
    STATIC_REQUIRE(byte_weave::detail::contains_v<TestLayout::Tag2, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>);
    STATIC_REQUIRE(byte_weave::detail::contains_v<TestLayout::Tag3, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>);
    STATIC_REQUIRE(byte_weave::detail::contains_v<TestLayout::Tag4, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>);
    STATIC_REQUIRE(byte_weave::detail::contains_v<TestLayout::Tag5, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>);

    STATIC_REQUIRE_FALSE(byte_weave::detail::contains_v<TestLayout::Unknow, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>);
}

TEST_CASE("Typeof trait")
{
    STATIC_REQUIRE(std::is_same_v<byte_weave::detail::TypeOf_t<TestLayout::Tag1, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>, int>);
    STATIC_REQUIRE(std::is_same_v<byte_weave::detail::TypeOf_t<TestLayout::Tag2, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>, std::uint64_t>);
    STATIC_REQUIRE(std::is_same_v<byte_weave::detail::TypeOf_t<TestLayout::Tag3, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>, float>);
    STATIC_REQUIRE(std::is_same_v<byte_weave::detail::TypeOf_t<TestLayout::Tag4, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>, double>);
    STATIC_REQUIRE(std::is_same_v<byte_weave::detail::TypeOf_t<TestLayout::Tag5, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>, std::uint8_t>);

    STATIC_REQUIRE(std::is_same_v<byte_weave::detail::TypeOf_t<TestLayout::Unknow, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>, byte_weave::detail::TypeOfTagNotFound>);
}

TEST_CASE("Ofsetof trait")
{
    REQUIRE(byte_weave::detail::offsetof_v<TestLayout::Tag1, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> == 0);
    REQUIRE(byte_weave::detail::offsetof_v<TestLayout::Tag2, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> == sizeof(int));
    REQUIRE(byte_weave::detail::offsetof_v<TestLayout::Tag3, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> == sizeof(int) + sizeof(std::uint64_t));
    REQUIRE(byte_weave::detail::offsetof_v<TestLayout::Tag4, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> == sizeof(int) + sizeof(std::uint64_t) + sizeof(float));
    REQUIRE(byte_weave::detail::offsetof_v<TestLayout::Tag5, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> == sizeof(int) + sizeof(std::uint64_t) + sizeof(float) + sizeof(double));
}

TEST_CASE("unique type trait")
{
    STATIC_REQUIRE(byte_weave::detail::unique_types<TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>::value);

    STATIC_REQUIRE_FALSE(byte_weave::detail::unique_types<TestLayout::Field1, TestLayout::Field1, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>::value);
    STATIC_REQUIRE_FALSE(byte_weave::detail::unique_types<TestLayout::Field1, TestLayout::Field2, TestLayout::Field1, TestLayout::Field4, TestLayout::Field5>::value);
    STATIC_REQUIRE_FALSE(byte_weave::detail::unique_types<TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field1>::value);
    STATIC_REQUIRE_FALSE(byte_weave::detail::unique_types<TestLayout::Field1, TestLayout::Field1, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5>::value);
    STATIC_REQUIRE_FALSE(byte_weave::detail::unique_types<TestLayout::Field1, TestLayout::Field2, TestLayout::Field1, TestLayout::Field4, TestLayout::Field1>::value);
    STATIC_REQUIRE_FALSE(byte_weave::detail::unique_types<TestLayout::Field2, TestLayout::Field2, TestLayout::Field3, TestLayout::Field1, TestLayout::Field1>::value);
}

TEST_CASE("Endian test")
{
    std::uint8_t ui8 = 231;
    byte_weave::byteswap(ui8);
    REQUIRE(ui8 == 231);

    std::uint16_t ui16 = 46125;
    byte_weave::byteswap(ui16);
    REQUIRE(ui16 == 11700);

    std::uint32_t ui32 = 38361254;
    byte_weave::byteswap(ui32);
    REQUIRE(ui32 == 2790803714);

    std::uint64_t ui64 = 543216789098;
    byte_weave::byteswap(ui64);
    REQUIRE(ui64 == 7666884111465709568);

    float f32 = 8654321.1234;
    float f32old = f32;
    byte_weave::byteswap(f32);
    byte_weave::byteswap(f32);
    REQUIRE(f32 == f32old);

    double f64 = 12332.321;
    double f64old = f64;
    byte_weave::byteswap(f64);
    byte_weave::byteswap(f64);
    REQUIRE(f64 == f64old);
}

TEST_CASE("Buffer concepts")
{
    using BytePtr       = std::byte*;
    using Span          = std::span<std::byte>;
    using Span12        = std::span<std::byte,33>;
    using BuffA         = std::array<std::byte, 12>;
    using BuffL         = std::list<std::byte>;
    using BuffV         = std::vector<std::byte>;
    using BuffVElem     = std::vector<int>;
    using BuffLelem     = std::list<double>;
    using BuffAElem     = std::array<short, 12>;
    using SpanElem      = std::span<char>;

    using ConstBytePtr       = const std::byte*;
    using ConstSpan          = const std::span<const std::byte>;
    using ConstSpan12        = const std::span<const std::byte,12>;
    using ConstBuffA         = const std::array<std::byte, 12>;
    using ConstBuffL         = const std::list<std::byte>;
    using ConstBuffV         = const std::vector<std::byte>;
    using ConstBuffVElem     = const std::vector<int>;
    using ConstBuffLelem     = const std::list<double>;
    using ConstBuffAElem     = const std::array<short, 12>;
    using ConstSpanElem      = const std::span<char>;

    SECTION("ReadableByteBuffer")
    {
        CHECK(byte_weave::detail::ReadableByteBuffer<Span>);
        CHECK(byte_weave::detail::ReadableByteBuffer<Span12>);
        CHECK(byte_weave::detail::ReadableByteBuffer<BuffA>);
        CHECK(byte_weave::detail::ReadableByteBuffer<BuffV>);

        CHECK(byte_weave::detail::ReadableByteBuffer<ConstSpan>);
        CHECK(byte_weave::detail::ReadableByteBuffer<ConstSpan12>);
        CHECK(byte_weave::detail::ReadableByteBuffer<ConstBuffA>);
        CHECK(byte_weave::detail::ReadableByteBuffer<ConstBuffV>);

        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<BuffL>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<BuffVElem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<BuffLelem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<BuffAElem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<SpanElem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<BytePtr>);

        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<ConstBytePtr>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<ConstBuffL>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<ConstBuffVElem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<ConstBuffAElem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<ConstSpanElem>);
        CHECK_FALSE(byte_weave::detail::ReadableByteBuffer<ConstBuffLelem>);
        
    }

    SECTION("WritableByteBuffer")
    {
        CHECK(byte_weave::detail::WritableByteBuffer<Span>);
        CHECK(byte_weave::detail::WritableByteBuffer<Span12>);
        CHECK(byte_weave::detail::WritableByteBuffer<BuffA>);
        CHECK(byte_weave::detail::WritableByteBuffer<BuffV>);
        
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<BuffL>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<BuffVElem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<BuffLelem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<BuffAElem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<SpanElem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<BytePtr>);

        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBytePtr>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBuffL>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBuffVElem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBuffAElem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstSpanElem>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBuffLelem>);

        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstSpan>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstSpan12>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBuffA>);
        CHECK_FALSE(byte_weave::detail::WritableByteBuffer<ConstBuffV>);
    }
}

