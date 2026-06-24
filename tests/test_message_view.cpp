#include <algorithm>
#include <cstddef>
#include <cstring>
#include <random>
#include <utility>
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

TEST_CASE("Message view")
{
    using Schema = TestMsgSchemas::Schema;


    SECTION("ctor")
    {   
        auto buffer1 = Schema::make_buffer();
        auto buffer2 = Schema::make_buffer();
        byte_weave::MessageView<Schema> view1(buffer1);
        byte_weave::MessageView<Schema> view2(buffer2);
        view1.fill(std::byte{0xAA});
        view2.fill(std::byte{0xFC});

        // check copy ctor
        SECTION("copy ctor")
        {
            byte_weave::MessageView<Schema> view3(view2);
            const auto ret_b3 = view3.buffer();
            
            bool is_eq = ret_b3.size() == buffer2.size();
            REQUIRE(is_eq);
            for(std::size_t i = 0; i < ret_b3.size(); ++i)
            {
                is_eq &= (ret_b3[i] == buffer2[i]);
            }
            CHECK(is_eq);
        }
        
        SECTION("move ctor")
        {
            byte_weave::MessageView<Schema> view3(std::move(view2));
            CHECK_FALSE(view2.valid());
            CHECK(view3.valid());

            const auto ret_buf = view3.buffer();
            bool is_eq = ret_buf.size() == buffer2.size();
            REQUIRE(is_eq);
            for(std::size_t i = 0; i < ret_buf.size(); ++i)
            {
                is_eq &= (ret_buf[i] == buffer2[i]);
            }
            CHECK(is_eq);
        }
    }

    SECTION("assignment")
    {
        auto buffer1 = Schema::make_buffer();
        auto buffer2 = Schema::make_buffer();
        byte_weave::MessageView<Schema> view1(buffer1);
        byte_weave::MessageView<Schema> view2(buffer2);
        view1.fill(std::byte{0xAA});
        view2.fill(std::byte{0xFC});

        SECTION("copy")
        {   
            auto buffer3 = Schema::make_buffer();
            byte_weave::MessageView<Schema> view3(buffer3);
            
            view3 = view2;
            CHECK(view2.valid());
            CHECK(view3.valid());

            auto ret_b3 = view3.buffer();
            
            bool is_eq = ret_b3.size() == buffer2.size();
            REQUIRE(is_eq);
            for(std::size_t i = 0; i < ret_b3.size(); ++i)
            {
                is_eq &= (ret_b3[i] == buffer2[i]);
            }
            CHECK(is_eq);
        }
        
        SECTION("move")
        {
            auto buffer3 = Schema::make_buffer();
            byte_weave::MessageView<Schema> view3(buffer3);
            
            view3 = std::move(view2);
            CHECK_FALSE(view2.valid());
            CHECK(view3.valid());
            
            auto ret_b3 = view3.buffer();
            
            bool is_eq = ret_b3.size() == buffer2.size();
            REQUIRE(is_eq);
            for(std::size_t i = 0; i < ret_b3.size(); ++i)
            {
                is_eq &= (ret_b3[i] == buffer2[i]);
            }
            CHECK(is_eq);
        }
    }

    SECTION("size")
    {
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> msg_view(buffer);
        CHECK(msg_view.size<TestMsgSchemas::Body1::Layout>() == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body1::size_off);
        CHECK(msg_view.size<TestMsgSchemas::Body2::Layout>() == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body2::size_off);
        CHECK(msg_view.size<TestMsgSchemas::Body3::Layout>() == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body3::size_off);
    }
    SECTION("capacity")
    {
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> msg_view(buffer);
        CHECK(msg_view.capacity() == TestMsgSchemas::Header::size_off + TestMsgSchemas::Body2::size_off);
    }
    SECTION("clear")
    {
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> msg_view(buffer);
        auto buffer_other = Schema::make_buffer();
        for (size_t i =0;i<buffer_other.size();++i)
        {
            buffer_other[i] = std::byte(255);
        }
        msg_view.bind(buffer_other);
        msg_view.clear();
        auto ret_buff = msg_view.buffer();
        bool all_is_null = std::all_of(ret_buff.begin(), ret_buff.end(),[](std::byte b){return b == std::byte{0};});
        CHECK(all_is_null);
    }

    SECTION("bind")
    {
        auto b1 = Schema::make_buffer();
        auto b2 = Schema::make_buffer();
        byte_weave::MessageView<Schema> view(b1);

        std::byte v{0xAf};
        std::byte other = ~v;
        view.fill(v);
        view.bind(b2);
        view.fill(v);

        bool is_eq = (b1.size() == b2.size());
        CHECK(is_eq);
        for(std::size_t i =0; i < b1.size(); ++i)
        {
            is_eq &= (b1[i] == b2[i]);
        }
        CHECK(is_eq);

        view.bind(b1);
        view.fill(other);
        bool is_not_eq = false;
        for(std::size_t i =0; i < b1.size(); ++i)
        {
            is_not_eq |= (b1[i] == b2[2]);
        }
        CHECK_FALSE(is_not_eq);
    }

    SECTION("fill")
    {
        auto buff = Schema::make_buffer();
        byte_weave::MessageView<Schema> view(buff);
        std::byte v{0xAf};
        view.fill(v);
        bool is_all = std::all_of(buff.begin(), buff.end(), [&v](std::byte b){return b == v;});
        CHECK(is_all);
    } 

    
    SECTION("span")
    {
        std::mt19937 rng(123);
        std::uniform_int_distribution<std::uint8_t> dist(0, 255);
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> view(buffer);
        for(std::size_t i = 0; i < buffer.size(); i++)
        {
            buffer[i] = static_cast<std::byte>(dist(rng));
        }

        std::mt19937 rng_idx(123);
        std::uniform_int_distribution<std::size_t> dist_idx(0, buffer.size());
        for (std::size_t i = 0; i < 100; i++)
        {
            std::size_t first_idx = dist_idx(rng_idx);
            std::size_t last_idx = dist_idx(rng_idx);
            if (first_idx > last_idx)
                std::swap(first_idx, last_idx);
            
            auto slice = view.span(first_idx, last_idx);
            bool is_eq = true;
            REQUIRE(slice.size() == (last_idx - first_idx));
            for (std::size_t j = 0; j < slice.size(); j++)
            {
                std::size_t idx = j + first_idx;
                is_eq &= (slice[j] == buffer[idx]);
            }
            REQUIRE(is_eq);
        }

        // check bounds
        auto slice_full = view.span(0, buffer.size());
        CHECK(slice_full.size() == buffer.size());

        auto slice_single_byte = view.span(0, 1);
        CHECK(slice_single_byte.size() == 1);
        CHECK(slice_single_byte[0] == buffer[0]);

        auto slice_empty = view.span(3,3);
        CHECK(slice_empty.size() == 0);
        
        // auto slice = view.span<0, 5>();
        
    }

    SECTION("span template")
    {
        std::mt19937 rng(123);
        std::uniform_int_distribution<std::uint8_t> dist(0, 255);
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> view(buffer);
        for(std::size_t i = 0; i < buffer.size(); i++)
        {
            buffer[i] = static_cast<std::byte>(dist(rng));
        }
        
        constexpr std::size_t first = 0;
        constexpr std::size_t last = buffer.size() - 4;
        auto slice = view.span<first, last>();
        REQUIRE(slice.size() == (last - first));
        bool is_eq=true;
        for (std::size_t j = 0; j < slice.size(); j++)
        {
            std::size_t idx = j + first;
            is_eq &= (slice[j] == buffer[idx]);
        }
        CHECK(is_eq);

        auto slice_full = view.span<0, buffer.size()>();
        CHECK(slice_full.size() == buffer.size());

        auto slice_single_byte = view.span<0, 1>();
        CHECK(slice_single_byte.size() == 1);
        CHECK(slice_single_byte[0] == buffer[0]);

        auto slice_empty = view.span<3,3>();
        CHECK(slice_empty.size() == 0);
    }

    SECTION("set_primary / get_primary")
    {
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> view(buffer);
        
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag1, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f1 = -123456;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag2, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f2 = 12345677654322221;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag3, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f3 = 123456.433;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag4, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f4 = -0.2325323423523442;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag5, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f5 = 212;

        view.set_primary<TestMsgSchemas::Header::Tag1>(f1);
        view.set_primary<TestMsgSchemas::Header::Tag2>(f2);
        view.set_primary<TestMsgSchemas::Header::Tag3>(f3);
        view.set_primary<TestMsgSchemas::Header::Tag4>(f4);
        view.set_primary<TestMsgSchemas::Header::Tag5>(f5);

        auto ret_f1 = view.get_primary<TestMsgSchemas::Header::Tag1>();
        auto ret_f2 = view.get_primary<TestMsgSchemas::Header::Tag2>();
        auto ret_f3 = view.get_primary<TestMsgSchemas::Header::Tag3>();
        auto ret_f4 = view.get_primary<TestMsgSchemas::Header::Tag4>();
        auto ret_f5 = view.get_primary<TestMsgSchemas::Header::Tag5>();

        CHECK(std::memcmp(&f1, &ret_f1, sizeof(f1)) == 0);
        CHECK(std::memcmp(&f2, &ret_f2, sizeof(f2)) == 0);
        CHECK(std::memcmp(&f3, &ret_f3, sizeof(f3)) == 0);
        CHECK(std::memcmp(&f4, &ret_f4, sizeof(f4)) == 0);
        CHECK(std::memcmp(&f5, &ret_f5, sizeof(f5)) == 0);
    }

    SECTION("set / get")
    {
        auto buffer = Schema::make_buffer();
        byte_weave::MessageView<Schema> view(buffer);
        
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag1, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f1 = -123456;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag2, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f2 = 12345677654322221;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag3, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f3 = 123456.433;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag4, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f4 = -0.2325323423523442;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag5, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f5 = 212;


        view.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(f1);
        view.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(f2);
        view.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(f3);
        view.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(f4);
        view.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(f5);

        auto ret_f1 = view.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>();
        auto ret_f2 = view.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>();
        auto ret_f3 = view.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>();
        auto ret_f4 = view.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>();
        auto ret_f5 = view.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>();

        CHECK(std::memcmp(&f1, &ret_f1, sizeof(f1)) == 0);
        CHECK(std::memcmp(&f2, &ret_f2, sizeof(f2)) == 0);
        CHECK(std::memcmp(&f3, &ret_f3, sizeof(f3)) == 0);
        CHECK(std::memcmp(&f4, &ret_f4, sizeof(f4)) == 0);
        CHECK(std::memcmp(&f5, &ret_f5, sizeof(f5)) == 0);

        byte_weave::detail::TypeOf_t<TestMsgSchemas::Body1::Tag1, TestMsgSchemas::Body1::Field1, TestMsgSchemas::Body1::Field2> b1f1 = 21233321;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Body1::Tag2, TestMsgSchemas::Body1::Field1, TestMsgSchemas::Body1::Field2> b1f2 = 1278908764321;


        Schema::set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(buffer, b1f1);
        Schema::set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(buffer, b1f2);
        
        auto ret_b1f1 = Schema::get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(buffer);
        auto ret_b1f2 = Schema::get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(buffer);

        CHECK(memcmp(&ret_b1f1, &b1f1, sizeof(b1f1)) == 0);
        CHECK(memcmp(&ret_b1f2, &b1f2, sizeof(b1f2)) == 0);
    }
}