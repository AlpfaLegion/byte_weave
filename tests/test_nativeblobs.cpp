#include <bit>
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <byte_weave/byte_weave.hpp>
#include <cstdint>
#include <cstring>

struct TestMsgSchemas
{
    struct Header
    {
        struct Tag1{};
        struct Version
        {
            std::uint32_t major;
            std::uint32_t minor;
        };

        struct Tag2{};
        
        struct Tag3{};
        #pragma pack(push, 1)
        struct Somebody
        {
            short a;
            float b;
            std::uint16_t c;
        };
        #pragma pack(pop)

        using BlobField1 = byte_weave::FieldNativeBlob<Tag1, Version>;
        using BlobField2 = byte_weave::Field<Tag2, std::uint64_t>;
        using BlobField3 = byte_weave::FieldNativeBlob<Tag3, Somebody>;

        using Layout = byte_weave::Layout<BlobField1, BlobField2, BlobField3>;
    };

    struct BodyRequest
    {
        struct Tag1{};
        struct Tag2{};

        using Field1 = byte_weave::Field<Tag1, std::uint8_t>;
        using Field2 = byte_weave::Field<Tag2, std::uint32_t>;

        using Layout = byte_weave::Layout<Field1, Field2>;

    };

    struct BodyResponce
    {
        struct Tag1{};

        struct Tag2{};
        #pragma pack(push, 1)
        struct BlolbHeader
        {
            std::uint16_t a; 
            std::uint32_t b;
            float c;
        };
        #pragma pack(pop)

        struct Tag3{};
        struct Arr
        {
            struct Point
            {
                float x;
                float y;
            };
            std::array<Point, 100> points;
        };
        using Field1 = byte_weave::Field<Tag1, std::uint16_t>;

        using BlobField2 = byte_weave::FieldPackedBlob<Tag2, BlolbHeader, 10>;
        using BlobField3 = byte_weave::FieldNativeBlob<Tag3, Arr>;

        using Layout = byte_weave::Layout<Field1, BlobField2, BlobField3>;
    };

};

TEST_CASE("Field")
{
    STATIC_CHECK(alignof(TestMsgSchemas::BodyResponce::BlobField2::type) == 1);
    STATIC_CHECK(TestMsgSchemas::BodyResponce::BlobField2::size == 10);
    STATIC_CHECK(TestMsgSchemas::BodyResponce::BlobField3::size == 800);
    
    STATIC_CHECK(byte_weave::is_field_compatible_endians_v<TestMsgSchemas::BodyResponce::BlobField3::type, byte_weave::EndiansPolicy::Native>);
    STATIC_CHECK(!byte_weave::is_field_compatible_endians_v<TestMsgSchemas::BodyResponce::BlobField3::type, byte_weave::EndiansPolicy::Little>);
    STATIC_CHECK(!byte_weave::is_field_compatible_endians_v<TestMsgSchemas::BodyResponce::BlobField3::type, byte_weave::EndiansPolicy::Big>);

    STATIC_CHECK(byte_weave::is_field_compatible_endians_v<TestMsgSchemas::BodyResponce::Field1::type, byte_weave::EndiansPolicy::Native>);
    STATIC_CHECK(byte_weave::is_field_compatible_endians_v<TestMsgSchemas::BodyResponce::Field1::type, byte_weave::EndiansPolicy::Little>);
    STATIC_CHECK(byte_weave::is_field_compatible_endians_v<TestMsgSchemas::BodyResponce::Field1::type, byte_weave::EndiansPolicy::Big>);
}

TEST_CASE("Single message")
{
    using Schema = byte_weave::SingleLayoutMessage<TestMsgSchemas::BodyResponce::Layout>;

    const std::uint16_t n = 58;
    byte_weave::Message<Schema> msg;
    msg.set_primary<TestMsgSchemas::BodyResponce::Tag1>(n);

    TestMsgSchemas::BodyResponce::BlolbHeader blobheader {65000,12312313,44.332};
    msg.set_primary<TestMsgSchemas::BodyResponce::Tag2>(blobheader);

    TestMsgSchemas::BodyResponce::Arr points;
    for (std::size_t i = 0; i < n; i ++)
    {
        points.points[i].x = 0.1 * i;
        points.points[i].y = -0.2 * i;
    }
    msg.set_primary<TestMsgSchemas::BodyResponce::Tag3>(points);

    auto get_v1 = msg.get_primary<TestMsgSchemas::BodyResponce::Tag1>();
    auto get_v2 = msg.get_primary<TestMsgSchemas::BodyResponce::Tag2>();
    auto get_v3 = msg.get_primary<TestMsgSchemas::BodyResponce::Tag3>();
    
    CHECK(get_v1 == n);
    STATIC_CHECK(sizeof(get_v2) == sizeof(blobheader));
    CHECK(memcmp(&blobheader, &get_v2, sizeof(get_v2))==0);

    auto span = msg.span<sizeof(get_v1) + sizeof(get_v2),  sizeof(get_v1) + sizeof(get_v2) + n * sizeof(TestMsgSchemas::BodyResponce::Arr::Point)>();
    CHECK(span.size() == n * sizeof(TestMsgSchemas::BodyResponce::Arr::Point));
    CHECK(memcmp(&points, span.data(), n * sizeof(TestMsgSchemas::BodyResponce::Arr::Point)) == 0);
    CHECK(memcmp(&points, &get_v3, n * sizeof(TestMsgSchemas::BodyResponce::Arr::Point)) == 0);
}