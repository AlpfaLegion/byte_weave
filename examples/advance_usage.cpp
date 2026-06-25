#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <vector>
#include <byte_weave/byte_weave.hpp>
#include <iomanip>
#include <cstdint>
#include <byte_weave/byte_weave.hpp>

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

int main()
{
    using Schema = byte_weave::SingleLayoutMessage<BodyResponce::Layout>;

    std::uint16_t n = 58;
    byte_weave::Message<Schema> msg;
    msg.set_primary<BodyResponce::Tag1>(n);
    msg.set_primary<BodyResponce::Tag2, byte_weave::EndiansPolicy::Native>({65000,12312313,44.332});

    BodyResponce::Arr points;
    for (std::size_t i = 0; i < n; i ++)
    {
        points.points[i].x = 0.1 * i;
        points.points[i].y = -0.2 * i;
    }
    msg.set_primary<BodyResponce::Tag3>(points);

    // ------------------------- send  buffer
    auto buff = msg.buffer();
    // ------------------------- get fields
    auto get_v1 = msg.get_primary<BodyResponce::Tag1>();
    auto get_v2 = msg.get_primary<BodyResponce::Tag2>();
    auto get_v3 = msg.get_primary<BodyResponce::Tag3>();
    
    return 1;
}