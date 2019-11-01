#include"interface.hpp"
#include<iostream>
typedef boost::shared_ptr<client> client_ptr;

int main()
{
    boost::asio::io_service io_service;
    tcp::endpoint endpoint(address::from_string("127.0.0.1"), 2019);

    client_ptr new_session(new client(io_service, endpoint));

    std::cout<<"RPC 结果是："<<new_session->add(1,2)<<std::endl;


    return 0;
}
