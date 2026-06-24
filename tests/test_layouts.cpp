#include <bit>
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

TEST_CASE("Test layouts common prop")
{
    REQUIRE(TestLayout::Layout::size == (sizeof(int)+sizeof(std::uint64_t)+sizeof(float)+sizeof(double)+sizeof(std::uint8_t)));
}

TEST_CASE("Test layouts set<big> / get<little>")
{
    TestLayout::Layout::buffer_type buffer{};
    byte_weave::detail::TypeOf_t<TestLayout::Tag1, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f1 = 123456;
    byte_weave::detail::TypeOf_t<TestLayout::Tag2, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f2 = 12345677654322221;
    byte_weave::detail::TypeOf_t<TestLayout::Tag3, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f3 = 123456.433;
    byte_weave::detail::TypeOf_t<TestLayout::Tag4, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f4 = 12345623434.5323423523442;
    byte_weave::detail::TypeOf_t<TestLayout::Tag5, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f5 = 212;

    TestLayout::Layout::set<TestLayout::Tag1, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f1);
    TestLayout::Layout::set<TestLayout::Tag2, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f2);
    TestLayout::Layout::set<TestLayout::Tag3, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f3);
    TestLayout::Layout::set<TestLayout::Tag4, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f4);
    TestLayout::Layout::set<TestLayout::Tag5, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f5);

    auto get_v1 = TestLayout::Layout::get<TestLayout::Tag1, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Little>(buffer);
    auto get_v2 = TestLayout::Layout::get<TestLayout::Tag2, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Little>(buffer);
    auto get_v3 = TestLayout::Layout::get<TestLayout::Tag3, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Little>(buffer);
    auto get_v4 = TestLayout::Layout::get<TestLayout::Tag4, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Little>(buffer);
    auto get_v5 = TestLayout::Layout::get<TestLayout::Tag5, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Little>(buffer);

    if (std::endian::native == std::endian::little)
    {
        byte_weave::byteswap(f1);
        byte_weave::byteswap(f2);
        byte_weave::byteswap(f3);
        byte_weave::byteswap(f4);
        byte_weave::byteswap(f5);
    }

    REQUIRE(get_v1 == f1);
    REQUIRE(get_v2 == f2);
    REQUIRE(get_v3 == f3);
    REQUIRE(get_v4 == f4);
    REQUIRE(get_v5 == f5);
}

TEST_CASE("Test layouts set<big> / get<big>")
{
    TestLayout::Layout::buffer_type buffer{};
    byte_weave::detail::TypeOf_t<TestLayout::Tag1, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f1 = 123456;
    byte_weave::detail::TypeOf_t<TestLayout::Tag2, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f2 = 12345677654322221;
    byte_weave::detail::TypeOf_t<TestLayout::Tag3, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f3 = 123456.433;
    byte_weave::detail::TypeOf_t<TestLayout::Tag4, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f4 = 12345623434.5323423523442;
    byte_weave::detail::TypeOf_t<TestLayout::Tag5, TestLayout::Field1, TestLayout::Field2, TestLayout::Field3, TestLayout::Field4, TestLayout::Field5> f5 = 212;

    TestLayout::Layout::set<TestLayout::Tag1, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f1);
    TestLayout::Layout::set<TestLayout::Tag2, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f2);
    TestLayout::Layout::set<TestLayout::Tag3, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f3);
    TestLayout::Layout::set<TestLayout::Tag4, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f4);
    TestLayout::Layout::set<TestLayout::Tag5, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer, f5);

    auto get_v1 = TestLayout::Layout::get<TestLayout::Tag1, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer);
    auto get_v2 = TestLayout::Layout::get<TestLayout::Tag2, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer);
    auto get_v3 = TestLayout::Layout::get<TestLayout::Tag3, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer);
    auto get_v4 = TestLayout::Layout::get<TestLayout::Tag4, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer);
    auto get_v5 = TestLayout::Layout::get<TestLayout::Tag5, TestLayout::Layout::buffer_type, byte_weave::EndiansPolicy::Big>(buffer);

    REQUIRE(get_v1 == f1);
    REQUIRE(get_v2 == f2);
    REQUIRE(get_v3 == f3);
    REQUIRE(get_v4 == f4);
    REQUIRE(get_v5 == f5);
}

