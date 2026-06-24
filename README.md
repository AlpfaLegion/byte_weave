# ByteWeave

ByteWeave is a header-only C++ library for describing and manipulating fixed-size
binary protocols with compile-time schemas.

It is aimed at low-level systems where the wire format is already known or must
be controlled byte by byte: network protocols, market data feeds, device
communication, embedded systems, robotics, gateways, and other latency-sensitive
code.

ByteWeave does not define a serialization format. It does not add metadata,
field descriptors, tags, varints, alignment padding, or runtime reflection.
Instead, it lets you describe your own binary layout in the C++ type system and
then access fields by semantic tags rather than raw offsets.

## Why

Low-level protocol code often starts as direct buffer manipulation:

```cpp
std::memcpy(buffer + 8, &trans_id, sizeof(trans_id));
std::memcpy(&value, buffer + 8, sizeof(value));
```

This is fast, but it usually spreads magic offsets through the codebase. As the
protocol grows, field order, sizes, endian conversions, and buffer requirements
become easy to break.

ByteWeave keeps the same low-level representation, but moves the layout
definition into compile-time types:

```cpp
using Header =
    byte_weave::Layout<
        byte_weave::Field<TagVersion, std::uint16_t>,
        byte_weave::Field<TagType,    std::uint8_t>,
        byte_weave::Field<TagId,      std::uint32_t>>;
```

The compiler can then derive field offsets, layout sizes, message capacity, and
many validity checks before the program runs.

## Current Guarantees

ByteWeave validates the following at compile time:

- Field offsets
- Layout sizes
- Message buffer size
- Duplicate field tags inside a layout
- Duplicate layouts inside a schema
- Unsupported field types
- Invalid endian policy for native blobs
- Packed blob alignment and expected size
- Static buffer overflow for statically sized buffers

Runtime buffers such as dynamic `std::span<std::byte>` or `std::vector<std::byte>`
must still be large enough for the schema. Use `MessageSpan::bind_check(...)`
when binding external storage whose size is not guaranteed by the type.

## Requirements

- C++23 target in the current CMake configuration
- Standard library support for `std::span`, concepts, ranges, and `std::endian`
- CMake 3.15 or newer when using the provided build files

## Quick Example

```cpp
#include <cstdint>
#include <byte_weave/byte_weave.hpp>
#include <iostream>

struct Version {};
struct MsgType {};
struct RequestId {};
struct Price {};
struct Quantity {};

using HeaderLayout =
    byte_weave::Layout<
        byte_weave::Field<Version,   std::uint16_t>,
        byte_weave::Field<MsgType,   std::uint8_t>,
        byte_weave::Field<RequestId, std::uint32_t>>;

using OrderLayout =
    byte_weave::Layout<
        byte_weave::Field<Price,    std::uint64_t>,
        byte_weave::Field<Quantity, std::uint32_t>>;

using OrderMessage =
    byte_weave::CompositeMessage<HeaderLayout, OrderLayout>;

int main()
{
    byte_weave::Message<OrderMessage> msg;

    msg.set_primary<Version, byte_weave::EndiansPolicy::Big>(1);
    msg.set_primary<MsgType>(42);
    msg.set_primary<RequestId, byte_weave::EndiansPolicy::Big>(1001);

    msg.set<OrderLayout, Price, byte_weave::EndiansPolicy::Big>(1234500);
    msg.set<OrderLayout, Quantity, byte_weave::EndiansPolicy::Big>(10);

    auto bytes = msg.buffer();

    auto version = msg.get_primary<Version, byte_weave::EndiansPolicy::Big>();
    auto msg_type = msg.get_primary<MsgType>();
    auto req_id = msg.get_primary<RequestId, byte_weave::EndiansPolicy::Big>();
    auto price = msg.get<OrderLayout, Price, byte_weave::EndiansPolicy::Big>();
    auto quantity = msg.get<OrderLayout, Quantity, byte_weave::EndiansPolicy::Big>();

    std::cout<<"Version:        "<<version<<std::endl;
    std::cout<<"Message type:   "<<int(msg_type)<<std::endl;
    std::cout<<"Request id:     "<<req_id<<std::endl;
    std::cout<<"Price:          "<<price<<std::endl;
    std::cout<<"Quantity:       "<<quantity<<std::endl;
    std::cout<<"Buffer size:    "<<bytes.size()<<std::endl;
    return 1;
}
```

## Core Concepts

### Field

A field binds a tag type to a value type:

```cpp
struct RequestId {};

using FRequestId =
    byte_weave::Field<RequestId, std::uint32_t>;
```

Supported scalar fields are integral types except `bool`, floating-point types,
and enum types. For portable wire formats, prefer fixed-width integer types such
as `std::uint16_t`, `std::uint32_t`, and `std::uint64_t`.

### Layout

A layout is an ordered sequence of fields:

```cpp
using HeaderLayout =
    byte_weave::Layout<FVersion, FMsgType, FRequestId>;
```

The order of fields is the wire order. ByteWeave computes:

- `HeaderLayout::size`
- `HeaderLayout::offset<Tag>()`
- `HeaderLayout::set<Tag>(buffer, value)`
- `HeaderLayout::get<Tag>(buffer)`

### MessageSchema

A schema describes a complete message shape:

```cpp
using Schema =
    byte_weave::CompositeMessage<HeaderLayout, RequestBody, ResponseBody>;
```

The first layout is the primary layout. Alternative layouts are placed after the
primary layout and share the same storage region. The schema buffer size is:

```text
primary layout size + max(alternative layout sizes)
```

For a single fixed layout, use:

```cpp
using Schema = byte_weave::SingleLayoutMessage<HeaderLayout>;
```

## Message Types

### `Message<Schema>`

`Message` owns its storage. It is the easiest type to use when the message buffer
should live together with the object.

```cpp
byte_weave::Message<Schema> msg;
msg.set_primary<Tag>(value);
auto value = msg.get_primary<Tag>();
```

### `MessageView<Schema>`

`MessageView` references an existing schema-sized buffer:

```cpp
auto buffer = Schema::make_buffer();
byte_weave::MessageView<Schema> view(buffer);
```

It is lightweight and non-owning. After move, the moved-from view is invalid and
`valid()` should be checked before use.

### `MessageSpan<Schema>`

`MessageSpan` references external byte storage such as `std::span`,
`std::array`, or `std::vector`.

```cpp
std::vector<std::byte> storage(Schema::buffer_size);

byte_weave::MessageSpan<Schema> span;
if (span.bind_check(storage))
{
    span.set_primary<Tag>(value);
}
```

For dynamic storage, prefer `bind_check(...)` unless the buffer size is already
guaranteed by construction.

## Endian Policies

Field access can specify the byte order used in the buffer:

```cpp
msg.set_primary<RequestId, byte_weave::EndiansPolicy::Big>(1001);

auto id =
    msg.get_primary<RequestId, byte_weave::EndiansPolicy::Big>();
```

Available policies:

- `byte_weave::EndiansPolicy::Native`
- `byte_weave::EndiansPolicy::Little`
- `byte_weave::EndiansPolicy::Big`

Non-native endian policies are available for scalar fields. Native blobs are
only compatible with native endian policy.

## Native And Packed Blobs

`FieldNativeBlob<Tag, T>` can be used for trivially copyable standard-layout
types:

```cpp
struct RawHeader
{
    std::uint16_t version;
    std::uint16_t flags;
};

using FRawHeader =
    byte_weave::FieldNativeBlob<RawHeaderTag, RawHeader>;
```

Native blobs are copied as raw object representation and are only compatible with
`EndiansPolicy::Native`. ByteWeave does not byte-swap fields inside the blob.

This is intentionally unsafe for portable wire formats. Native blobs may include
padding and may depend on compiler, ABI, platform endian, and structure layout.
Use `FieldNativeBlob` only when the native representation is exactly what the
protocol requires.

`FieldPackedBlob<Tag, T, ExpectedSize>` is a stricter native blob wrapper for
packed structures. It checks two extra conditions at compile time:

- `alignof(T) == 1`
- `sizeof(T) == ExpectedSize`

This is useful when the protocol contains a fixed binary structure and you want
the compiler to reject accidental padding or size drift.

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <byte_weave/byte_weave.hpp>

struct RecordCount {};
struct HeaderBlob {};
struct PointsBlob {};

#pragma pack(push, 1)
struct BlobHeader
{
    std::uint16_t a;
    std::uint32_t b;
    float c;
};
#pragma pack(pop)

struct Points
{
    struct Point
    {
        float x;
        float y;
    };

    std::array<Point, 100> points;
};

using ResponseLayout =
    byte_weave::Layout<
        byte_weave::Field<RecordCount, std::uint16_t>,
        byte_weave::FieldPackedBlob<HeaderBlob, BlobHeader, 10>,
        byte_weave::FieldNativeBlob<PointsBlob, Points>>;

using ResponseMessage =
    byte_weave::SingleLayoutMessage<ResponseLayout>;

int main()
{
    byte_weave::Message<ResponseMessage> msg;

    std::uint16_t count = 58;
    BlobHeader header{65000, 12312313, 44.332f};
    Points points{};

    for (std::size_t i = 0; i < count; ++i)
    {
        points.points[i].x = 0.1f * static_cast<float>(i);
        points.points[i].y = -0.2f * static_cast<float>(i);
    }

    msg.set_primary<RecordCount>(count);
    msg.set_primary<HeaderBlob>(header);
    msg.set_primary<PointsBlob>(points);

    auto stored_header = msg.get_primary<HeaderBlob>();
    auto stored_points = msg.get_primary<PointsBlob>();

}
```

Packed blobs are still native binary blobs. They make size and alignment
mistakes harder, but they do not make a structure endian-neutral or ABI-neutral.

When using `FieldNativeBlob` or `FieldPackedBlob`, the user is responsible for
all representation details: padding, packing pragmas or attributes, endian
layout of nested fields, ABI/compiler differences, floating-point
representation, initialization of padding bytes, and compatibility with the
external protocol. ByteWeave only validates the type-level constraints it can
see and copies the object representation into the message buffer.

## Building

ByteWeave is header-only, so there are two practical ways to use it.

The simplest option is to copy or vendor the single-header distribution from
`single_include/byte_weave/byte_weave.hpp`, add `single_include` to your include
path, and include the library directly:

```cpp
#include <byte_weave/byte_weave.hpp>
```

The second option is to use the repository with CMake. The provided CMake target
is an `INTERFACE` library that exposes the same header-only include path:

```sh
mkdir build
cd build

cmake .. \
    -DBYTE_WEAVE_BUILD_TESTS=ON \
    -DBYTE_WEAVE_BUILD_EXAMPLES=ON \
    -DBYTE_WEAVE_INSTALL_SINGLE_HEADER=ON \
    -DCMAKE_INSTALL_PREFIX=prefix/install/path

cmake --build ./ --parallel 4
cmake --install ./
```

Build options:

- `BYTE_WEAVE_BUILD_TESTS` builds the test suite. Default: `ON`.
- `BYTE_WEAVE_BUILD_EXAMPLES` builds example programs. Default: `OFF`.
- `BYTE_WEAVE_INSTALL_SINGLE_HEADER` installs `single_include/` instead of
  `include/`. Default: `OFF`.
- `CMAKE_INSTALL_PREFIX` sets the install destination.

## When To Use ByteWeave

ByteWeave is a good fit when:

- The protocol is fixed-size or mostly fixed-size
- The binary layout must match an external specification
- Every byte in the wire format matters
- You want compile-time offsets instead of handwritten constants
- You want type-checked field access without runtime serialization machinery
- You need explicit endian handling

It is not trying to replace Protocol Buffers, FlatBuffers, Cap'n Proto, or other
full serialization ecosystems. Those tools own a wire format and provide schema
evolution, rich data models, and cross-language tooling. ByteWeave is lower
level: it helps you build the exact wire format you already need.

## Project Direction

The current version is a compile-time toolkit for fixed-layout binary messages.
Future work may include:

- A protocol DSL
- Code generation
- Reflection utilities
- Stronger checked APIs for dynamic buffers
- More compile-time diagnostics

The guiding principle is simple:

**Full control over binary layout without giving up type safety where the
compiler can provide it.**

## License

ByteWeave is licensed under the MIT License. See [LICENSE](LICENSE).
