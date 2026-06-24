#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>
#include <byte_weave/byte_weave.hpp>

struct Protocol
{
    struct Header
    {
        struct TagVersion{};
        struct TagTimestamp{};
        struct TagTransactionType{}; // or TagTransactionType

        using Field1 = byte_weave::Field<TagVersion, std::uint16_t>;
        using Field2 = byte_weave::Field<TagTimestamp, std::uint64_t>;
        using Field3 = byte_weave::Field<TagTransactionType, std::uint8_t>;
        
        using Layout = byte_weave::Layout<Field1, Field2, Field3>;
    };
    struct BodyResponce
    {
        struct TagStatus{};
        struct TagErrorCode{};

        using Field1 = byte_weave::Field<TagStatus, std::uint8_t>;
        using Field2 = byte_weave::Field<TagErrorCode, std::uint16_t>;

        using Layout = byte_weave::Layout<Field1, Field2>;
    };

    struct BodyRequest
    {
        struct TagCmd{};
        struct TagParam1{};
        struct TagParam2{};

        using Field1 = byte_weave::Field<TagCmd, std::uint8_t>;
        using Field2 = byte_weave::Field<TagParam1, float>;
        using Field3 = byte_weave::Field<TagParam2, double>;

        using Layout = byte_weave::Layout<Field1, Field2, Field3>;
    };

    using Schema = byte_weave::CompositeMessage<
                        Header::Layout, 
                        BodyRequest::Layout, 
                        BodyResponce::Layout>;

    using Message = byte_weave::Message<Schema>;
    using MessageView = byte_weave::MessageView<Schema>;
};


Protocol::Message make_request()
{
    // processing and get cmd and params
    int cmd = 1;
    float param1 = 123.04312;
    double param2 = 32312321.4441231245;

    Protocol::Message msg;
    msg.set_primary<Protocol::Header::TagVersion>(1);
    msg.set_primary<Protocol::Header::TagTimestamp>(1234567890);
    msg.set_primary<Protocol::Header::TagTransactionType>(11); // 11 - request 22 - responce

    msg.set<Protocol::BodyRequest::Layout, Protocol::BodyRequest::TagCmd>(cmd); // cmd with id = 1
    msg.set<Protocol::BodyRequest::Layout, Protocol::BodyRequest::TagParam1>(param1);
    msg.set<Protocol::BodyRequest::Layout, Protocol::BodyRequest::TagParam2>(param2);

    return msg;
}

void example()
{
    // peer 1
    std::cout<<"Peer 1"<<std::endl;
    auto send_msg = make_request();
    
    //send and recieve buffer
    auto buffer = send_msg.buffer();
    
    // peer 2
    std::cout<<"Peer 2"<<std::endl;
    byte_weave::MessageSpan<Protocol::Schema> msg_span(buffer);

    std::cout<<"Header"<<std::endl;
    std::cout<< msg_span.get_primary<Protocol::Header::TagVersion>()<<std::endl;
    std::cout<< int(msg_span.get_primary<Protocol::Header::TagTransactionType>())<<std::endl;
    std::cout<< msg_span.get_primary<Protocol::Header::TagTimestamp>()<<std::endl;
    
    std::cout<<"Body"<<std::endl;
    std::cout<<int(msg_span.get<Protocol::BodyRequest::Layout, Protocol::BodyRequest::TagCmd>())<<std::endl;
    std::cout<<msg_span.get<Protocol::BodyRequest::Layout, Protocol::BodyRequest::TagParam1>()<<std::endl;
    std::cout<<msg_span.get<Protocol::BodyRequest::Layout, Protocol::BodyRequest::TagParam2>()<<std::endl;
    
    // make responce via current buffer
    
    msg_span.set_primary<Protocol::Header::TagVersion>(1);
    msg_span.set_primary<Protocol::Header::TagTimestamp>(33334444555);
    msg_span.set_primary<Protocol::Header::TagTransactionType>(22); // 11 - request 22 - responce

    msg_span.set<Protocol::BodyResponce::Layout, Protocol::BodyResponce::TagStatus>(10);
    msg_span.set<Protocol::BodyResponce::Layout, Protocol::BodyResponce::TagErrorCode>(44321);

    // send responce to peer 1
    std::cout<<"Peer 1"<<std::endl;

    std::vector<std::byte> rcv_buffer;
    std::copy(buffer.begin(), buffer.end(), std::back_inserter(rcv_buffer));
    byte_weave::MessageSpan<Protocol::Schema> mspan(rcv_buffer);
    std::cout<<"Header"<<std::endl;
    std::cout<<mspan.get_primary<Protocol::Header::TagVersion>()<<std::endl;
    std::cout<<int(mspan.get_primary<Protocol::Header::TagTransactionType>())<<std::endl;
    std::cout<<mspan.get_primary<Protocol::Header::TagTimestamp>()<<std::endl;
    if (mspan.get_primary<Protocol::Header::TagTransactionType>() == 22)
    {
        std::cout<<"responce (body)"<<std::endl;
        std::cout<<int(mspan.get<Protocol::BodyResponce::Layout, Protocol::BodyResponce::TagStatus>())<<std::endl;
        std::cout<<mspan.get<Protocol::BodyResponce::Layout, Protocol::BodyResponce::TagErrorCode>()<<std::endl;
    }
}   

int main()
{   
    example();
    return EXIT_SUCCESS;
}