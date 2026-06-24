#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
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

template<typename Buffer>
void fill_byte_buffer(Buffer& buff)
{
    for (std::size_t i = 0; i < buff.size(); ++i)
    {
        buff[i] = std::byte(i);
    }
}

TEST_CASE("Message span")
{
    using Schema = TestMsgSchemas::Schema;

    SECTION("ctor")
    {
        std::vector<std::byte> other_buff{30, std::byte{0xAf}};
        std::fill(other_buff.begin(), other_buff.end(), std::byte{0});
        byte_weave::MessageSpan<Schema> msg_span(other_buff);
        auto span = msg_span.span();
        
        bool is_eq = span.size() == other_buff.size();
        REQUIRE(is_eq);
        for(std::size_t i = 0; i < other_buff.size(); ++i)
        {
            is_eq &= (span[i] == other_buff[i]);
        }
        CHECK(is_eq);
    }

    SECTION("move / cpy ctors")
    {
        // move and cpy ctors have same behavior (std::span standart)
        std::vector<std::byte> other_buff{30, std::byte{0xAf}};
        fill_byte_buffer(other_buff);

        byte_weave::MessageSpan<Schema> mspan1(other_buff);
        byte_weave::MessageSpan<Schema> mspan2(std::move(mspan1));

        CHECK(mspan1.size() == other_buff.size());
        REQUIRE(mspan2.size() == other_buff.size());
        CHECK(std::memcmp(mspan1.data(), other_buff.data(), other_buff.size()) == 0);
  
        byte_weave::MessageSpan<Schema> mspan3(mspan2);
        REQUIRE(mspan2.size() == mspan3.size());
        CHECK(mspan3.data() == mspan2.data());

        CHECK(std::memcmp(mspan2.data(), mspan3.data(), mspan3.size()) == 0);
    }

    SECTION("move / cpy assignments")
    {
        std::vector<std::byte> other_buff{30, std::byte{0xAf}};
        fill_byte_buffer(other_buff);

        byte_weave::MessageSpan<Schema> mspan1(other_buff);
        byte_weave::MessageSpan<Schema> mspan2;

        mspan2 = std::move(mspan1);

        CHECK(mspan1.size() == other_buff.size());
        REQUIRE(mspan2.size() == other_buff.size());
        CHECK(std::memcmp(mspan1.data(), other_buff.data(), other_buff.size()) == 0);
  
        byte_weave::MessageSpan<Schema> mspan3;

        mspan3 = mspan2;
        REQUIRE(mspan2.size() == mspan3.size());
        CHECK(mspan2.data() == mspan3.data());

        CHECK(std::memcmp(mspan2.data(), mspan3.data(), mspan3.size()) == 0);
    
    }

    SECTION("size")
    {
        std::vector<std::byte> other_buff{30, std::byte{0xAf}};
        byte_weave::MessageSpan<Schema> mspan1(other_buff);
        CHECK(mspan1.size() == other_buff.size());
    }

    SECTION("clear")
    {
        std::vector<std::byte> other_buff{30, std::byte{0xAf}};
        fill_byte_buffer(other_buff);
        byte_weave::MessageSpan<Schema> mspan1(other_buff);
        mspan1.clear();
        bool is_eq = true;
        for (auto v: other_buff)
        {
            if (v != std::byte{0})
            {
                is_eq = false;
                break;
            }
        }
        CHECK(is_eq);
    }

    SECTION("fill")
    {
        std::vector<std::byte> other_buff{30};
        fill_byte_buffer(other_buff);
        byte_weave::MessageSpan<Schema> mspan1(other_buff);
        mspan1.fill(std::byte{0xAf});
        for (auto v: other_buff)
        {
            if (v != std::byte{0xAf})
            {
                CHECK(false);
            }
        }
    }

    SECTION("bind")
    {
        std::size_t n = 30;
        std::vector<std::byte> other_buff{n};
        fill_byte_buffer(other_buff);
        byte_weave::MessageSpan<Schema> mspan1;
        CHECK(mspan1.size() == 0);
        CHECK(mspan1.data() == nullptr);
        mspan1.bind(other_buff);
        CHECK(mspan1.size() == other_buff.size());
        CHECK(mspan1.data() == other_buff.data());

        // raw allocate buffer
        std::byte* raw_buff = new std::byte[n];
        mspan1.bind(raw_buff, n);

        CHECK(mspan1.data() != other_buff.data());
        CHECK(mspan1.data() == raw_buff);
        REQUIRE(mspan1.size() == n);
        CHECK(memcmp(mspan1.data(), raw_buff, n) == 0);
        delete [] raw_buff;

        std::vector<std::byte> other_buff2{50};
        fill_byte_buffer(other_buff2);
        mspan1.bind(other_buff2.begin(), other_buff2.end());
        CHECK(mspan1.data() == other_buff2.data());
        REQUIRE(mspan1.size() ==  other_buff2.size());
        CHECK(memcmp(mspan1.data(), other_buff2.data(), mspan1.size()) == 0);
    }

    SECTION("bind check")
    {
        std::size_t n = 50;
        std::vector<std::byte> other_buff{n};
        fill_byte_buffer(other_buff);
        byte_weave::MessageSpan<Schema> mspan1;
        CHECK(mspan1.size() == 0);
        CHECK(mspan1.data() == nullptr);

        REQUIRE(mspan1.bind_check(other_buff));
        CHECK(mspan1.size() == Schema::buffer_size);
        CHECK(mspan1.data() == other_buff.data());

        // raw allocate buffer
        std::byte* raw_buff = new std::byte[n];
        REQUIRE(mspan1.bind_check(raw_buff, n));

        CHECK(mspan1.data() != other_buff.data());
        CHECK(mspan1.data() == raw_buff);
        REQUIRE(mspan1.size() ==  Schema::buffer_size);
        CHECK(memcmp(mspan1.data(), raw_buff, n) == 0);
        delete [] raw_buff;

        std::vector<std::byte> smal_buff(10);
        byte_weave::MessageSpan<Schema> mspan2;
        CHECK(!mspan2.bind_check(smal_buff));
        CHECK(mspan2.size() == 0);
        CHECK(mspan2.data() == nullptr);

        CHECK(!mspan2.bind_check(raw_buff, 10));
        CHECK(mspan2.size() == 0);
        CHECK(mspan2.data() == nullptr);

        // bind iterators
        std::vector<std::byte> other_buff2{50};
        fill_byte_buffer(other_buff2);
        CHECK(mspan2.bind_check(other_buff2.begin(), other_buff2.end()));
        CHECK(mspan2.data() == other_buff2.data());
        REQUIRE(mspan2.size() ==  Schema::buffer_size);
        CHECK(memcmp(mspan2.data(), other_buff2.data(),  Schema::buffer_size) == 0);
    }

    SECTION("span")
    {
        std::size_t n = 30;
        std::vector<std::byte> other_buff{n};
        fill_byte_buffer(other_buff);
        byte_weave::MessageSpan<Schema> mspan(other_buff);
        
        auto span_full = mspan.span();
        REQUIRE(span_full.size() == mspan.size());
        CHECK(span_full.data() == mspan.data());
        CHECK(memcmp(mspan.data(), span_full.data(), span_full.size()) == 0);

        auto subspan = mspan.span(5, 20);
        REQUIRE(subspan.size() == 15);
        CHECK(subspan.data() == mspan.data() + 5);
        CHECK(memcmp(mspan.data() + 5, subspan.data(), subspan.size()) == 0);

        auto subspan_stat = mspan.span<2, 22>();
        REQUIRE(subspan_stat.size() == 20);
        CHECK(subspan_stat.data() == mspan.data() + 2);
        CHECK(memcmp(mspan.data() + 2, subspan_stat.data(), subspan_stat.size()) == 0);
    }

    SECTION("set_primary / get_primary")
    {
        auto buffer = Schema::make_buffer();
        byte_weave::MessageSpan<Schema> mspan(buffer);

        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag1, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f1 = -123456;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag2, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f2 = 12345677654322221;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag3, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f3 = 123456.433;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag4, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f4 = -0.2325323423523442;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag5, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f5 = 212;


        mspan.set_primary<TestMsgSchemas::Header::Tag1>(f1);
        mspan.set_primary<TestMsgSchemas::Header::Tag2>(f2);
        mspan.set_primary<TestMsgSchemas::Header::Tag3>(f3);
        mspan.set_primary<TestMsgSchemas::Header::Tag4>(f4);
        mspan.set_primary<TestMsgSchemas::Header::Tag5>(f5);

        std::vector<std::byte> other_buff;
        std::copy(buffer.begin(), buffer.end(), std::back_inserter(other_buff));

        byte_weave::MessageSpan<Schema> mspan2;
        REQUIRE(mspan2.bind_check(other_buff));
        REQUIRE(mspan2.size() == other_buff.size());
        REQUIRE(mspan2.data() == other_buff.data());

        auto ret_f1 = mspan2.get_primary<TestMsgSchemas::Header::Tag1>();
        auto ret_f2 = mspan2.get_primary<TestMsgSchemas::Header::Tag2>();
        auto ret_f3 = mspan2.get_primary<TestMsgSchemas::Header::Tag3>();
        auto ret_f4 = mspan2.get_primary<TestMsgSchemas::Header::Tag4>();
        auto ret_f5 = mspan2.get_primary<TestMsgSchemas::Header::Tag5>();
        
        CHECK(std::memcmp(&f1, &ret_f1, sizeof(f1)) == 0);
        CHECK(std::memcmp(&f2, &ret_f2, sizeof(f2)) == 0);
        CHECK(std::memcmp(&f3, &ret_f3, sizeof(f3)) == 0);
        CHECK(std::memcmp(&f4, &ret_f4, sizeof(f4)) == 0);
        CHECK(std::memcmp(&f5, &ret_f5, sizeof(f5)) == 0);
    }

    SECTION("set / get")
    {

        auto buffer = Schema::make_buffer();
        byte_weave::MessageSpan<Schema> mspan(buffer);

        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag1, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f1 = -123456;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag2, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f2 = 12345677654322221;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag3, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f3 = 123456.433;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag4, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f4 = -0.2325323423523442;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Header::Tag5, TestMsgSchemas::Header::Field1, TestMsgSchemas::Header::Field2, TestMsgSchemas::Header::Field3, TestMsgSchemas::Header::Field4, TestMsgSchemas::Header::Field5> f5 = 212;

        byte_weave::detail::TypeOf_t<TestMsgSchemas::Body1::Tag1, TestMsgSchemas::Body1::Field1, TestMsgSchemas::Body1::Field2> b1f1 = 21233321;
        byte_weave::detail::TypeOf_t<TestMsgSchemas::Body1::Tag2, TestMsgSchemas::Body1::Field1, TestMsgSchemas::Body1::Field2> b1f2 = 1278908764321;

        mspan.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>(f1);
        mspan.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>(f2);
        mspan.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>(f3);
        mspan.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>(f4);
        mspan.set<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>(f5);

        mspan.set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>(b1f1);
        mspan.set<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>(b1f2);

        std::array<std::byte, 100> other_buff;
        std::copy(buffer.begin(), buffer.end(), other_buff.begin());

        byte_weave::MessageSpan<Schema, 90> mspan2(other_buff.begin(), other_buff.begin() + 90);
        REQUIRE(!(mspan2.size() == other_buff.size()));
        REQUIRE(mspan2.data() == other_buff.data());

        auto ret_f1 = mspan2.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag1>();
        auto ret_f2 = mspan2.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag2>();
        auto ret_f3 = mspan2.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag3>();
        auto ret_f4 = mspan2.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag4>();
        auto ret_f5 = mspan2.get<TestMsgSchemas::Header::Layout, TestMsgSchemas::Header::Tag5>();
        
        CHECK(std::memcmp(&f1, &ret_f1, sizeof(f1)) == 0);
        CHECK(std::memcmp(&f2, &ret_f2, sizeof(f2)) == 0);
        CHECK(std::memcmp(&f3, &ret_f3, sizeof(f3)) == 0);
        CHECK(std::memcmp(&f4, &ret_f4, sizeof(f4)) == 0);
        CHECK(std::memcmp(&f5, &ret_f5, sizeof(f5)) == 0);

        auto ret_b1f1 = mspan2.get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag1>();
        auto ret_b1f2 = mspan2.get<TestMsgSchemas::Body1::Layout, TestMsgSchemas::Body1::Tag2>();

        CHECK(memcmp(&ret_b1f1, &b1f1, sizeof(b1f1)) == 0);
        CHECK(memcmp(&ret_b1f2, &b1f2, sizeof(b1f2)) == 0);
    }
}