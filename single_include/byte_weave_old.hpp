#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <bit>
#include <cstring>
#include <span>
#include <utility>
#include <ranges>
#include <iostream>
// SPDX-License-Identifier: MIT
// temporal implementation
// namespace std {
//     namespace byte_weave_detail
//     {
//         template<std::size_t SpanExtend>
//         concept span_static_size = (SpanExtend != std::dynamic_extent);
//     }
//     template<typename T, std::size_t Size> requires byte_weave_detail::span_static_size<Size>
//     struct tuple_size<std::span<T, Size>> : public std::integral_constant<std::size_t, Size>{};
// }


namespace byte_weave{
//===========================================Type Traits===========================================
template<typename T>
struct is_scalar_field: public std::bool_constant<
    (std::is_integral_v<T> && !std::is_same_v<T, bool>) ||
    std::is_floating_point_v<T> ||
    std::is_enum_v<T>>
{};
template<typename T>
inline constexpr bool is_scalar_field_v = is_scalar_field<T>::value;

template<typename T>
struct is_unsafe_native_blob: public std::bool_constant<
    std::is_trivially_copyable_v<T> &&
    std::is_standard_layout_v<T>
>
{};

template<typename T>
inline constexpr bool is_unsafe_native_blob_v = is_unsafe_native_blob<T>::value && !is_scalar_field_v<T>;


//-------contains----------------
template<typename Tag, typename... Fields>
struct contains
{
    static constexpr bool value = (std::is_same_v<Tag, typename Fields::tag> || ...);
};

template<typename Tag, typename... Fields>
inline constexpr bool contains_v = contains<Tag, Fields...>::value;

// ------------------------typeof-----------------------
template<typename Tag, typename... Fields>
struct TypeOf;

struct TypeOfTagNotFound{};

template<typename Tag>
struct TypeOf<Tag>
{
    using type = TypeOfTagNotFound;
};

template<typename Tag, typename U, typename... Tail>
struct TypeOf<Tag, U, Tail...>
{
    using type = std::conditional_t<std::is_same_v<Tag, typename U::tag>, typename U::type, typename TypeOf<Tag, Tail...>::type>;
};

template<typename Tag, typename... Fields>
using TypeOf_t = typename TypeOf<Tag, Fields...>::type;
// -----------------------------------------------

// ------------------------offsetof-----------------------
template<typename Tag, typename... Fields>
struct offsetof;

template<typename Tag>
struct offsetof<Tag>
{
    static constexpr std::size_t value = 0;
};

template<typename Tag, typename U, typename... Tail>
struct offsetof<Tag, U, Tail...>
{
    static constexpr std::size_t value = std::is_same_v<Tag, typename U::tag> ? 0 : (U::size + offsetof<Tag, Tail...>::value);
};

template<typename Tag, typename...Fields>
constexpr std::size_t offsetof_v = offsetof<Tag, Fields...>::value;

template<typename Tag, typename... Field>
struct layout_offset_of
{
    static constexpr std::size_t value = offsetof_v<Tag, Field...>;
};

template<typename Tag, typename... Fields>
inline constexpr std::size_t layout_offset_of_v = layout_offset_of<Tag, Fields...>::value;

// -----------------------unique---------------------------------
template<typename... Args>
struct unique_types;

template<>
struct unique_types<>: std::true_type
{};

template<typename T>
struct unique_types<T>: std::true_type
{};

template<typename T, typename... Args>
struct unique_types<T, Args...>: std::bool_constant<(!(std::is_same_v<T, Args> || ...)) && unique_types<Args...>::value>
{};

template<typename... Fields>
inline constexpr bool unique_tags_v = unique_types<typename Fields::tag ...>::value;

template<typename... Layouts>
inline constexpr bool unique_layouts_v = unique_types<Layouts...>::value;

template<typename PrimaryLayout, typename... AlternativeLayouts>
inline constexpr bool unique_schema_layouts_v = unique_types<PrimaryLayout, AlternativeLayouts...>::value;

// =============================================================================================



//================================endians=====================================
struct EndiansPolicy
{
    struct Little{};
    struct Big{};
    struct Native{};
};

template<std::size_t N>
void byteswap(std::byte* p) noexcept
{
    if constexpr (N <= 1)
        return;
    else if constexpr(N == 2)
    {
        std::byte tmp = p[0]; p[0] = p[1]; p[1] = tmp;
    } 
    else if constexpr(N == 4)
    {
        std::byte tmp = p[3]; p[3] = p[0]; p[0] = tmp;
                  tmp = p[2]; p[2] = p[1]; p[1] = tmp;
    }
    else if constexpr(N == 8)
    {
        std::byte tmp = p[7]; p[7] = p[0]; p[0] = tmp;
                  tmp = p[6]; p[6] = p[1]; p[1] = tmp;
                  tmp = p[5]; p[5] = p[2]; p[2] = tmp;
                  tmp = p[4]; p[4] = p[3]; p[3] = tmp;
    }
    else if constexpr(N == 16)
    {
        std::byte tmp = p[15]; p[15] = p[0]; p[0] = tmp;
                  tmp = p[14]; p[14] = p[1]; p[1] = tmp;
                  tmp = p[13]; p[13] = p[2]; p[2] = tmp;
                  tmp = p[12]; p[12] = p[3]; p[3] = tmp;
                  tmp = p[11]; p[11] = p[4]; p[4] = tmp;
                  tmp = p[10]; p[10] = p[5]; p[5] = tmp;
                  tmp = p[9];  p[9]  = p[6]; p[6] = tmp;
                  tmp = p[8];  p[8]  = p[7]; p[7] = tmp;
    }
    else    
    {
        std::ranges::reverse(p, p + N);
    }
}

template<typename T>
concept ByteSwapType = is_scalar_field_v<T>;

template<typename T>
requires ByteSwapType<T>
inline void byteswap(T& val) noexcept
{
    byteswap<sizeof(T)>(reinterpret_cast<std::byte*>(&val));
}

// --------------------------------------
//====================================Traits Endian policy ====================================
template<typename T, typename EPolicy>
struct is_field_compatible_endians;

template<typename T>
struct is_field_compatible_endians<T, EndiansPolicy::Native>: std::bool_constant<
                                                                        is_scalar_field_v<T> ||
                                                                        is_unsafe_native_blob_v<T>
>
{};

template<typename T>
struct is_field_compatible_endians<T, EndiansPolicy::Little>: std::bool_constant<is_scalar_field_v<T>>
{};

template<typename T>
struct is_field_compatible_endians<T, EndiansPolicy::Big>: std::bool_constant<is_scalar_field_v<T>>
{};

template<typename T, typename EPolicy>
inline constexpr bool is_field_compatible_endians_v = is_field_compatible_endians<T, EPolicy>::value;
//==========================================================================================

template<typename Tag, typename T>
struct FieldBase
{
    using tag = Tag;
    using type = T;
    static constexpr std::size_t size = sizeof(T);
};

template<typename Tag, typename T>
struct Field: FieldBase<Tag, T>
{
    static_assert(is_scalar_field_v<T>, "Field type must be scalar type");
};

template<typename Tag, typename T>
struct FieldNativeBlob: FieldBase<Tag, T>
{
    // static_assert(false, "[INFO] unsafe data type, need check padding blob and use native platform endian");
    static_assert(is_unsafe_native_blob_v<T>, "Field type must be native blob type, if use native blob check alignment ant paddings");
};


struct LayoutCommonProp
{
    using base_element_buffer_type = std::byte;

    template<std::size_t Nm>
    using base_buffer_type = std::array<base_element_buffer_type, Nm>;

    template<std::size_t Extent = std::dynamic_extent>
    using span_type = std::span<base_element_buffer_type, Extent>;

    template<std::size_t Extent = std::dynamic_extent>
    using const_span_type = std::span<const base_element_buffer_type, Extent>;
 
};

struct LayoutEmpty
{
    using type = void;

    constexpr static std::size_t size = 0;

    static constexpr  std::size_t offset(){return 0;}
};

template<typename CheckLayout, typename PrimaryLayout, typename... AlternativeLayouts>
struct ContainsLayout
{
    static constexpr bool check_primary = std::is_same_v<CheckLayout, PrimaryLayout>;
    static constexpr bool check_alternative = (std::is_same_v<CheckLayout, AlternativeLayouts> || ...);
    static constexpr bool value = check_primary || check_alternative;
};

template<typename CheckLayout, typename PrimaryLayout, typename... AlternativeLayouts>
inline constexpr bool is_contains_layout_v = ContainsLayout<CheckLayout, PrimaryLayout, AlternativeLayouts...>::value;

template<typename Range>
concept ByteBuffer = 
    (std::ranges::contiguous_range<Range>) &&
    (std::ranges::sized_range<Range>) &&
    //(std::ranges::borrowed_range<Range>) &&
    (std::is_same_v<std::ranges::range_value_t<Range>, std::byte>) &&
    requires(std::remove_cvref_t<Range> b)
{
    b.data();
    //{b.data()} -> std::convertible_to<std::byte*>;
    {b.size()} -> std::convertible_to<std::size_t>;
};

template<typename T>
concept ReadableByteBuffer = 
    ByteBuffer<T> &&
    requires(T buff, const T cbuff, std::size_t idx)
{
    {buff.data()} -> std::convertible_to<const std::byte*>;
    {cbuff.data()} -> std::convertible_to<const std::byte*>;

    {buff.size()} -> std::convertible_to<std::size_t>;
    {cbuff.size()} -> std::convertible_to<std::size_t>;
    
    requires std::same_as<std::remove_cvref_t<decltype(buff[idx])>, std::byte>;
    requires std::same_as<std::remove_cvref_t<decltype(cbuff[idx])>, std::byte>;
};

template<typename T>
concept WritableByteBuffer = 
    ByteBuffer<T> &&
    requires(T buff, std::size_t  idx)
{
    {buff.data()} -> std::same_as<std::byte*>;
    {buff.size()} -> std::convertible_to<std::size_t>;

    buff[idx] = std::byte{};
};

// static size
template<typename T>
struct static_size;

template<typename T, std::size_t Nm>
struct static_size<std::array<T, Nm>>: std::integral_constant<std::size_t, Nm>{};

template<typename T, std::size_t Nm>
struct static_size<T[Nm]>: std::integral_constant<std::size_t, Nm> {};

template<typename T, std::size_t Nm>
requires (Nm != std::dynamic_extent)
struct static_size<std::span<T, Nm>>: std::integral_constant<std::size_t, Nm>{};

template<typename T>
inline constexpr std::size_t static_size_v = static_size<std::remove_cvref_t<T>>::value;


template<typename BufferType>
concept has_static_size = 
    requires { typename static_size<std::remove_cvref_t<BufferType>>::type; };


template<typename StaticBufferType>
constexpr std::size_t get_static_size()
{
    using CleanType = std::remove_cvref_t<StaticBufferType>;
    if constexpr (std::is_array_v<CleanType>) 
    {
        return std::extent_v<CleanType>;
    }
    else
    {
        return static_size_v<CleanType>;
    }
}


template<typename... Fields>
struct Layout
{
    using fields = std::tuple<Fields...>;

    using self_type = Layout<Fields...>;

    template<typename Tag>
    using type = TypeOf_t<Tag, Fields...>;
    
    constexpr static std::size_t size = (Fields::size + ...);

    using buffer_type = LayoutCommonProp::base_buffer_type<size>;
    // static constexpr buffer_type make_buffer() noexcept {return {};}
    
    template<typename Tag>
    static constexpr  std::size_t offset()
    {
        static_assert(contains_v<Tag, Fields...>, "Tag not found");
        return  layout_offset_of_v<Tag, Fields...>;
    }

    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Unsupported mixed-endian platform");
    static_assert(unique_tags_v<Fields...>, "Duplicate field tags in Layout");

    template<typename EPolicy>
    static constexpr bool need_reverse_v = 
        ((std::endian::native == std::endian::little) && (std::is_same_v<EPolicy, EndiansPolicy::Big>)) ||
        ((std::endian::native == std::endian::big) && (std::is_same_v<EPolicy, EndiansPolicy::Little>));


    template<typename Tag, typename BufferType, std::size_t OffsetBuf = 0, typename EPolicy = EndiansPolicy::Native>
    requires WritableByteBuffer<BufferType>
    static constexpr void set(BufferType& buff, const type<Tag>& value) noexcept
    {
        static_assert(contains_v<Tag, Fields...>, "Tag not found");
        static_assert(is_field_compatible_endians_v<type<Tag>, EPolicy>,"Field has invalid type to selected endian policy");

        constexpr std::size_t off = offset<Tag>();
        constexpr std::size_t sff = sizeof(type<Tag>);
        //

        if constexpr (has_static_size<BufferType>) 
        {
           static_assert(OffsetBuf + off + sff <= get_static_size<BufferType>(), "Out of range buffer");
        }
        
        std::memcpy(buff.data() + OffsetBuf + off, &value, sff);

        if constexpr ((sizeof(type<Tag>) > 1) && need_reverse_v<EPolicy>)
        {
            byteswap<sff>(buff.data() + OffsetBuf + off);
        }
    }

    template<typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
    requires WritableByteBuffer<BufferType>
    static constexpr void set(BufferType& buff, const type<Tag>& value) noexcept
    {
        set<Tag, BufferType, 0, EPolicy>(buff, value);
    }

    template<typename Tag, typename BufferType, std::size_t OffsetBuf = 0, typename EPolicy = EndiansPolicy::Native>
    requires ReadableByteBuffer<BufferType>
    static constexpr type<Tag> get(const BufferType& buff) noexcept
    {
        static_assert(contains_v<Tag, Fields...>, "Tag not found");
        static_assert(is_field_compatible_endians_v<type<Tag>, EPolicy>,"Field has invalid type to selected endian policy");

        constexpr std::size_t off = offset<Tag>();
        constexpr std::size_t sff = sizeof(type<Tag>);
        //
        if constexpr (has_static_size<BufferType>) 
        {
           static_assert(OffsetBuf + off + sff <= get_static_size<BufferType>(), "Out of range buffer");
        }

        type<Tag> value;

        std::memcpy(&value, buff.data() + OffsetBuf + off, sff);

        if constexpr ((sizeof(type<Tag>) > 1) && need_reverse_v<EPolicy>)
        {
            byteswap(value);
        }
        return value;
    }

    template<typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
    requires ReadableByteBuffer<BufferType>
    static constexpr type<Tag> get(const BufferType& buff) noexcept
    {
        return get<Tag, BufferType, 0, EPolicy>(buff);
    }
};

//====================================Message=================================================

template<typename PrimaryLayout, typename... AlternativeLayouts>
struct MessageSchemaBase: public LayoutCommonProp
{
private:
    template<typename T, typename... Args>
    static constexpr std::size_t max_size()
    {
        std::size_t value = T::size;
        ((value = value > Args::size ? value : Args::size), ...);
        return value;
    }

    static constexpr std::size_t get_alternative_max_size()
    {
        if constexpr (sizeof...(AlternativeLayouts) == 0)
        {
            return 0;
        }
        else
        {
            return max_size<AlternativeLayouts...>();
        }
    }

public:
    static_assert(unique_schema_layouts_v<PrimaryLayout, AlternativeLayouts...>, "Duplicate layouts in MessageSchema");

    static constexpr std::size_t max_alt_size = get_alternative_max_size();

    static constexpr std::size_t buffer_size = PrimaryLayout::size + max_alt_size;

    using buffer_type = LayoutCommonProp::base_buffer_type<buffer_size>;
    
    template<std::size_t Nm>
    using any_buffer_type = LayoutCommonProp::base_buffer_type<Nm>;

    template<std::size_t Extent = std::dynamic_extent>
    using span_type = LayoutCommonProp::span_type<Extent>;

    using Primary = PrimaryLayout;

    static constexpr buffer_type make_buffer() noexcept {return {};}
    
    template<std::size_t Nm>
    static constexpr base_buffer_type<Nm> make_buffer() noexcept {return {};}

    // template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
    // static constexpr void set(buffer_type& buffer, const typename Layout::template type<Tag>& value) noexcept
    // {
    //     static_assert(is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

    //     constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
        
    //     Layout::template set<Tag, buffer_type, offset_buf, EPolicy>(buffer, value);
    // }

    // template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
    // static constexpr typename Layout::template type<Tag> get(const buffer_type& buffer) noexcept
    // {
    //     static_assert(is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

    //     constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
        
    //     return Layout::template get<Tag, buffer_type, offset_buf, EPolicy>(buffer);
    // }

    template<typename Layout, typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
    static constexpr void set(BufferType& buffer, const typename Layout::template type<Tag>& value) noexcept
    {
        static_assert(is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

        constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
        
        Layout::template set<Tag, BufferType, offset_buf, EPolicy>(buffer, value);
    }

    template<typename Layout, typename Tag, typename BufferType, typename EPolicy = EndiansPolicy::Native>
    static constexpr typename Layout::template type<Tag> get(const BufferType& buffer) noexcept
    {
        static_assert(is_contains_layout_v<Layout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");

        constexpr std::size_t offset_buf = std::is_same_v<PrimaryLayout, Layout> ? 0 : PrimaryLayout::size;
        
        return Layout::template get<Tag, BufferType, offset_buf, EPolicy>(buffer);
    }
};
// -------------------------------------Api-------------------------------------

template<typename PrimaryLayout, typename... AlternativeLayouts>
struct MessageSizeApi
{
private:
    template<typename ActiveLayout>
    struct MessageSize
    {
        static_assert(is_contains_layout_v<ActiveLayout, PrimaryLayout, AlternativeLayouts...>, "Layout not found");
        static constexpr std::size_t value = std::is_same_v<ActiveLayout, PrimaryLayout> ?
            PrimaryLayout::size : PrimaryLayout::size + ActiveLayout::size;
    };
public:
    template<typename AlternativeLayout>
    static constexpr std::size_t message_size = MessageSize<AlternativeLayout>::value;
};

template<typename PrimaryLayout>
struct MessageSizeApi <PrimaryLayout>
{
    static constexpr std::size_t message_size = PrimaryLayout::size;
};

//--------------------------------------------------------------------------------------

template<typename PrimaryLayout, typename... AlternativeLayouts>
struct MessageSchema: 
    public MessageSchemaBase<PrimaryLayout, AlternativeLayouts...>,
    public MessageSizeApi<PrimaryLayout, AlternativeLayouts...>
{};

template<typename PrimaryLayout, typename... AlternativeLayouts>
using CompositeMessage = MessageSchema<PrimaryLayout, AlternativeLayouts...>;

template<typename Layout>
using SingleLayoutMessage = MessageSchema<Layout>;

using EmptyMessage = MessageSchema<LayoutEmpty>;
//======================================================================================================

//================================View level==========================================
template<typename Schema>
class MessageView
{
public:
    using buffer_type = typename Schema::buffer_type;

private: 
    buffer_type*   view;
protected:
    MessageView(): view(nullptr){}

public:
    MessageView(typename Schema::buffer_type& buffer):
        view(&buffer)
    {}

    MessageView(const MessageView& other):
        view(other.view)
    {}

    MessageView(MessageView&& other):
        view(other.view)
    {
        other.view = nullptr;
    }

    MessageView& operator= (const MessageView& other) noexcept
    {
        if(this != &other)
        {
            view = other.view;
        }
        return *this;
    }
    MessageView& operator= (MessageView&& other) noexcept
    {
        if (this != &other)
        {
            view = std::move(other.view);
            other.view = nullptr;
        }
        return *this;
    }
    
    template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr void set(const typename Layout::template type<Tag>& value) noexcept
    {   
        Schema::template set<Layout, Tag, buffer_type, EPolicy>(*view, value);
    }

    template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr typename Layout::template type<Tag> get() const noexcept
    {   
        return Schema::template get<Layout, Tag, buffer_type, EPolicy>(*view);
    }
    
    template<typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr void set_primary(const typename Schema::Primary::template type<Tag>& value) noexcept
    {
        Schema::template set<typename Schema::Primary, Tag, buffer_type, EPolicy>(*view, value);
    }

    template<typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr typename Schema::Primary::template type<Tag> get_primary() const noexcept
    {
        return Schema::template get<typename Schema::Primary, Tag, buffer_type, EPolicy>(*view);
    }

    constexpr const buffer_type& buffer() const noexcept{return *view;}

    constexpr buffer_type& buffer() noexcept {return *view;}

    constexpr std::size_t capacity() const noexcept { return view ? view->size() : 0;}
    
    template<typename ActiveAlternativeLayout>
    constexpr std::size_t size() const noexcept
    {
        return Schema::template message_size<ActiveAlternativeLayout>;
    }

    // invariant of class
    bool valid() const noexcept
    {
        return view;
    }

    constexpr void clear() noexcept
    {
        view->fill(std::byte(0));
    }

    void fill(std::byte fill_val) noexcept
    {
        view->fill(fill_val);
    }
    
    void bind(buffer_type& other) noexcept
    {
        view = &other;
    }

    std::span<std::byte> span(std::size_t first, std::size_t last) noexcept
    {
        return std::span(*view).subspan(first, last - first);
    }

    template<std::size_t First, std::size_t Last>
    std::span<std::byte, Last - First> span() noexcept
    {
        return std::span(*view).template subspan<First, Last - First>();
    }
    
    ~MessageView() = default;
};

// ------------------------------------Message Span------------------------------------
template<std::size_t Extent>
concept DynamicExtent = (Extent == std::dynamic_extent);

template<std::size_t Nm, std::size_t SizeMsg>
concept CheckStaticArraySize = (Nm >= SizeMsg);

template<typename Schema, std::size_t Extent = std::dynamic_extent>
requires ((Extent == std::dynamic_extent) || (Schema::buffer_size <= Extent))
class MessageSpan
{
public:
    using span_type = typename Schema::template span_type<Extent>;
private:
    span_type         view;
public:

    constexpr MessageSpan() noexcept
    requires DynamicExtent<Extent>:
        view()
    {}
    
    template<typename Iter>
    explicit(Extent != std::dynamic_extent) 
    constexpr MessageSpan(Iter first, Iter last) noexcept:
        view(first, last)
    {

    }

    template<typename Range>
    requires (ByteBuffer<Range> &&    
            !std::same_as<std::remove_cvref_t<Range>, MessageSpan<Schema, Extent>>)
    MessageSpan(Range& range) noexcept: 
        view(range)
    {

    }
    
    template<std::size_t ArrExtend>
    constexpr explicit MessageSpan(typename Schema::template base_buffer_type<ArrExtend>& arr) noexcept
        requires CheckStaticArraySize<ArrExtend, Schema::buffer_size>:
            view(arr)
    {
        //std::cout<<"stat buff"<<std::endl;
    }

    MessageSpan(const MessageSpan& other) noexcept:
        view(other.view)
    {
        //std::cout<<"call ctor"<<std::endl;
    }

    MessageSpan(MessageSpan&& other):
        view(std::move(other.view))
    {
        // behavior thet complies with the std::span standart
        // other.view = span_type();   
    }

    MessageSpan& operator= (const MessageSpan& other) noexcept
    {
        if(this != &other)
        {
            view = other.view;
        }
        return *this;
    }
    MessageSpan& operator= (MessageSpan&& other) noexcept
    {
        if (this != &other)
        {
            // behavior thet complies with the std::span standart
            view = std::move(other.view);
        }
        return *this;
    }
    
    template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr void set(const typename Layout::template type<Tag>& value) noexcept
    {   
        Schema::template set<Layout, Tag, span_type, EPolicy>(view, value);
    }

    template<typename Layout, typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr typename Layout::template type<Tag> get() const noexcept
    {   
        return Schema::template get<Layout, Tag, span_type, EPolicy>(view);
    }
    
    template<typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr void set_primary(const typename Schema::Primary::template type<Tag>& value) noexcept
    {
        Schema::template set<typename Schema::Primary, Tag, span_type, EPolicy>(view, value);
    }

    template<typename Tag, typename EPolicy = EndiansPolicy::Native>
    constexpr typename Schema::Primary::template type<Tag> get_primary() const noexcept
    {
        return Schema::template get<typename Schema::Primary, Tag, span_type, EPolicy>(view);
    }
    
    constexpr std::size_t size() const noexcept
    {
        return view.size();
    }

    void clear() noexcept
    {
        fill(std::byte{0});
    }

    void fill(std::byte fill_val) noexcept
    {
        std::fill(view.begin(), view.end(), fill_val);
    }

    template<typename Range>
    void bind(Range& buffer)
    requires (DynamicExtent<Extent>) && ByteBuffer<Range>
    {
        view = std::span(buffer);
    }

    void bind(std::byte* storage_begin, std::size_t n)
    requires DynamicExtent<Extent>
    {
        view = typename Schema::span_type(storage_begin, n);
    }

    template<typename Iter>
    requires (std::contiguous_iterator<Iter> && DynamicExtent<Extent>)
    void bind(Iter first, Iter last)
    {
        view = span_type(first, last);
    }

    bool bind_check(std::byte* storage_begin, std::size_t n)
    requires DynamicExtent<Extent>
    {
        if (Schema::buffer_size <= n)
        {
            view = typename Schema::span_type(storage_begin, Schema::buffer_size);
            return true;
        }
        return false;
    }

    template<typename Range>
    bool bind_check(Range& buffer)
    requires (DynamicExtent<Extent>) && ByteBuffer<Range>
    {
        if (Schema::buffer_size <= buffer.size())
        {
            view = span_type(buffer.data(), Schema::buffer_size);
            return true;
        }
        return false;
    }

    template<typename Iter>
    requires (std::contiguous_iterator<Iter> && DynamicExtent<Extent>)
    bool bind_check(Iter first, Iter last)
    {
        if (Schema::buffer_size <= static_cast<std::size_t>(std::distance(first, last)))
        {
            view = span_type(first, first + Schema::buffer_size);
            return true;
        }
        return false;
    }
    
    typename Schema::template span_type<> span() noexcept
    {
        return view;
    }

    typename Schema::template const_span_type<> span() const noexcept
    {
        return view;
    }

    typename Schema::template const_span_type<> span(std::size_t first, std::size_t last) const noexcept
    {
        return view.subspan(first, last - first);
    }

    typename Schema::template span_type<> span(std::size_t first, std::size_t last) noexcept
    {
        return view.subspan(first, last - first);
    }

    template<std::size_t First, std::size_t Last>
    constexpr typename Schema::template span_type<Last - First> span() noexcept
    {
        return view.template subspan<First, Last - First>();
    }

    template<std::size_t First, std::size_t Last>
    constexpr typename Schema::template const_span_type<Last - First> span() const noexcept
    {
        return view.template subspan<First, Last - First>();
    }
    
    std::byte* data() noexcept 
    {
        return view.data();
    }

    const std::byte* data() const noexcept 
    {
        return view.data();
    }
    ~MessageSpan() = default;
};

//---------------------------------------------------------------------------------------------

//--------------------------------------------Message Owning-----------------------------------
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
//

//---------------------------------------------------------------------------------------------
