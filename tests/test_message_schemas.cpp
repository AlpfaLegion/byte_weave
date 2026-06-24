#include <cstring>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <byte_weave/byte_weave.hpp>


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

TEST_CASE("CompositeMessage schema")
{
    using Schema = TestMsgSchemas::Schema;
    auto buffer = Schema::make_buffer();
    
    SECTION("buffer_size")
    {
        CHECK(Schema::buffer_size == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body2::size_off);
    }

    SECTION("max size alternative layout")
    {
        CHECK(Schema::max_alt_size == TestMsgSchemas::Body2::size_off);
    }

    SECTION("message size")
    {
        CHECK(Schema::message_size<TestMsgSchemas::Body1::Layout> == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body1::size_off);
        CHECK(Schema::message_size<TestMsgSchemas::Body2::Layout> == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body2::size_off);
        CHECK(Schema::message_size<TestMsgSchemas::Body3::Layout> == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body3::size_off);
    }


    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag1, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f1 = -123456;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag2, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f2 = 12345677654322221;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag3, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f3 = 123456.433;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag4, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f4 = -0.2325323423523442;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag5, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f5 = 212;

    byte_weave::detail::TypeOf_t<TestMsgSchemas::Body1::Tag1, TestMsgSchemas::Body1::Field1, TestMsgSchemas::Body1::Field2> b1f1 = 21233321;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Body1::Tag2, TestMsgSchemas::Body1::Field1, TestMsgSchemas::Body1::Field2> b1f2 = 1278908764321;

    byte_weave::detail::TypeOf_t<TestMsgSchemas::Body2::Tag3, TestMsgSchemas::Body2::Field3, TestMsgSchemas::Body2::Field3> b2f3 = 3212.3321;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Body2::Tag4, TestMsgSchemas::Body2::Field4, TestMsgSchemas::Body2::Field4> b2f4 = 12345678764321.4423143;
    
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Body3::Tag5, TestMsgSchemas::Body3::Field5> b3f5 = 222;

    SECTION("only set")
    {
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(buffer, f1);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(buffer, f2);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(buffer, f3);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(buffer, f4);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(buffer, f5);
        
        Schema::set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(buffer, b1f1);
        Schema::set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(buffer, b1f2);

        decltype(f1) ret_f1;
        decltype(f2) ret_f2;
        decltype(f3) ret_f3;
        decltype(f4) ret_f4;
        decltype(f5) ret_f5;

        decltype(b1f1) ret_b1f1;
        decltype(b1f2) ret_b1f2;

        memcpy(&ret_f1, buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag1>(),sizeof(f1));
        memcpy(&ret_f2, buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag2>(),sizeof(f2));
        memcpy(&ret_f3, buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag3>(),sizeof(f3));
        memcpy(&ret_f4, buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag4>(),sizeof(f4));
        memcpy(&ret_f5, buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag5>(),sizeof(f5));
        
        memcpy(&ret_b1f1, buffer.data() + TestMsgSchemas::Header::Layout::size + TestMsgSchemas::Body1::Layout::offset<TestMsgSchemas::Body1::Tag1>(), sizeof(b1f1));
        memcpy(&ret_b1f2, buffer.data() + TestMsgSchemas::Header::Layout::size + TestMsgSchemas::Body1::Layout::offset<TestMsgSchemas::Body1::Tag2>(), sizeof(b1f2));
        
        CHECK(memcmp(&ret_f1, &f1, sizeof(f1)) == 0);
        CHECK(memcmp(&ret_f2, &f2, sizeof(f2)) == 0);
        CHECK(memcmp(&ret_f3, &f3, sizeof(f3)) == 0);
        CHECK(memcmp(&ret_f4, &f4, sizeof(f4)) == 0);
        CHECK(memcmp(&ret_f5, &f5, sizeof(f5)) == 0);

        CHECK(memcmp(&ret_b1f1, &b1f1, sizeof(b1f1)) == 0);
        CHECK(memcmp(&ret_b1f2, &b1f2, sizeof(b1f2)) == 0);

        Schema::set<TestMsgSchemas::Body3::Layout, TestMsgSchemas::Body3::Tag5>(buffer, b3f5);
        decltype(b3f5) ret_b3f5;

        memcpy(&ret_b3f5, buffer.data() + TestMsgSchemas::Header::Layout::size + TestMsgSchemas::Body3::Layout::offset<TestMsgSchemas::Body3::Tag5>(), sizeof(ret_b3f5));
        CHECK(memcmp(&ret_b3f5, &b3f5, sizeof(b3f5)) == 0);
    }


    SECTION("only get")
    {
        memcpy( buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag1>(), &f1,sizeof(f1));
        memcpy( buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag2>(), &f2,sizeof(f2));
        memcpy( buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag3>(), &f3,sizeof(f3));
        memcpy( buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag4>(), &f4,sizeof(f4));
        memcpy( buffer.data() + TestMsgSchemas::Header::Layout::offset<TestMsgSchemas::Header::Tag5>(), &f5,sizeof(f5));
        
        memcpy(buffer.data() + TestMsgSchemas::Header::Layout::size + TestMsgSchemas::Body1::Layout::offset<TestMsgSchemas::Body1::Tag1>(), &b1f1,sizeof(b1f1));
        memcpy(buffer.data() + TestMsgSchemas::Header::Layout::size + TestMsgSchemas::Body1::Layout::offset<TestMsgSchemas::Body1::Tag2>(), &b1f2,sizeof(b1f2));
        
        auto ret_f1 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(buffer);
        auto ret_f2 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(buffer);
        auto ret_f3 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(buffer);
        auto ret_f4 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(buffer);
        auto ret_f5 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(buffer);
        
        auto ret_b1f1 = Schema::get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(buffer);
        auto ret_b1f2 = Schema::get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(buffer);

        CHECK(memcmp(&ret_f1, &f1, sizeof(f1)) == 0);
        CHECK(memcmp(&ret_f2, &f2, sizeof(f2)) == 0);
        CHECK(memcmp(&ret_f3, &f3, sizeof(f3)) == 0);
        CHECK(memcmp(&ret_f4, &f4, sizeof(f4)) == 0);
        CHECK(memcmp(&ret_f5, &f5, sizeof(f5)) == 0);

        CHECK(memcmp(&ret_b1f1, &b1f1, sizeof(b1f1)) == 0);
        CHECK(memcmp(&ret_b1f2, &b1f2, sizeof(b1f2)) == 0);
    }

    SECTION("set / get")
    {
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(buffer, f1);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(buffer, f2);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(buffer, f3);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(buffer, f4);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(buffer, f5);

        Schema::set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(buffer, b1f1);
        Schema::set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(buffer, b1f2);

        
        auto get_v1 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(buffer);
        auto get_v2 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(buffer);
        auto get_v3 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(buffer);
        auto get_v4 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(buffer);
        auto get_v5 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(buffer);
        
        auto get_v6 = Schema::get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(buffer);
        auto get_v7 = Schema::get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(buffer);

        CHECK(get_v1 == f1);
        CHECK(get_v2 == f2);
        CHECK(get_v3 == f3);
        CHECK(get_v4 == f4);
        CHECK(get_v5 == f5);
        CHECK(get_v6 == b1f1);
        CHECK(get_v7 == b1f2);

        Schema::set<TestMsgSchemas::Body2::Layout, TestMsgSchemas::Body2::Tag3>(buffer, b2f3);
        Schema::set<TestMsgSchemas::Body2::Layout, TestMsgSchemas::Body2::Tag4>(buffer, b2f4);

        auto get_v8 = Schema::get<TestMsgSchemas::Body2::Layout, TestMsgSchemas::Body2::Tag3>(buffer);
        auto get_v9 = Schema::get<TestMsgSchemas::Body2::Layout, TestMsgSchemas::Body2::Tag4>(buffer);

        CHECK(get_v8 == b2f3);
        CHECK(get_v9 == b2f4);

        Schema::set<TestMsgSchemas::Body3::Layout, TestMsgSchemas::Body3::Tag5>(buffer, b3f5);
        auto get_v10 = Schema::get<TestMsgSchemas::Body3::Layout, TestMsgSchemas::Body3::Tag5>(buffer);

        CHECK(get_v10 == b3f5);
    }
}

TEST_CASE("SingleMessage schema")
{
    using Schema = TestMsgSchemas::SingleSchema;
    auto buffer = Schema::make_buffer();
    
    SECTION("buffer_size")
    {
        CHECK(Schema::buffer_size == TestMsgSchemas::Header::size_off);
    }

    SECTION("max size alternative layout")
    {
        CHECK(Schema::max_alt_size == 0);
    }

    SECTION("message size")
    {
        CHECK(Schema::message_size == TestMsgSchemas::Header::size_off);
    }
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag1, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f1 = -123456;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag2, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f2 = 12345677654322221;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag3, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f3 = 123456.433;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag4, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f4 = -0.2325323423523442;
    byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag5, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f5 = 212;

    SECTION("set / get")
    {
        
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(buffer, f1);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(buffer, f2);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(buffer, f3);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(buffer, f4);
        Schema::set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(buffer, f5);

        auto get_v1 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(buffer);
        auto get_v2 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(buffer);
        auto get_v3 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(buffer);
        auto get_v4 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(buffer);
        auto get_v5 = Schema::get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(buffer);
    
        CHECK(get_v1 == f1);
        CHECK(get_v2 == f2);
        CHECK(get_v3 == f3);
        CHECK(get_v4 == f4);
        CHECK(get_v5 == f5);
    }
}
