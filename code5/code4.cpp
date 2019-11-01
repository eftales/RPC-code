#include<boost/asio/io_service.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/bind.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/enable_shared_from_this.hpp>

#include<boost/asio/streambuf.hpp>

#include<boost/asio/placeholders.hpp>
#include<boost/asio.hpp>
using boost::asio::ip::tcp;
using boost::asio::ip::address;
#include <msgpack.hpp>


#define NOTAPPLICATED -3000
#define MAXPACKSIZE 1024

#include<string>
#include<iostream>

class client : public boost::enable_shared_from_this<client> {
public:
    client(boost::asio::io_service& io_service, tcp::endpoint& endpoint)
        : io_service_(io_service), socket_(io_service), endpoint_(endpoint)
    {
        buffer = std::make_shared<std::array<char, MAXPACKSIZE>>();
        result = 0;
    }

    int start(int a, int b) {
        boost::system::error_code ec;
        socket_.connect(endpoint_, ec);
        if (!ec)
        {
            static tcp::no_delay option(true);
            socket_.set_option(option);

            construct_rpc_data( a , b);
            send_recive_rpc_data(ec);
            std::cout << "send_recive_rpc_data返回值：" << result << std::endl;
            return result;
        }

        else
        {
            std::cerr << boost::system::system_error(ec).what() << std::endl;
        }
        return NOTAPPLICATED;
    }

private:
    void construct_rpc_data(int a, int b)
    {

//#define protocol_to_client std::tuple<int>
        std::tuple<int, int>  src(1,2);
        std::stringstream sbuffer;
        msgpack::pack(sbuffer, src);
        std::string strbuf(sbuffer.str());

        std::cout << " len " << strbuf.size() << std::endl;
        size_t len_bigend = boost::asio::detail::socket_ops::host_to_network_long(strbuf.size());
        memcpy(buffer->data(), &len_bigend, 4);
        memcpy(buffer->data() + 4, strbuf.data(), strbuf.size());
    }
    void send_recive_rpc_data(const boost::system::error_code& error) {

        auto self = this->shared_from_this();
        auto async_buffer = buffer;


        boost::asio::async_write(socket_, boost::asio::buffer(*async_buffer, MAXPACKSIZE),
            [this,self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {
                recive_rpc_data(ec);

                io_service_.stop();
            });
        io_service_.run();

    }

    void recive_rpc_data(const boost::system::error_code& error) {
        std::cout << "发送完毕，开始接受数据" << std::endl;
        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        boost::asio::async_read(socket_, boost::asio::buffer(*async_buffer, async_buffer->size()),
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {

                std::cout << "数据读取完成" << std::endl;
                handle_rpc_data(ec);
                io_service_.stop();

            });
        io_service_.run();


    }

    void handle_rpc_data(const boost::system::error_code& error) {

        std::cout << "读到数据：" << buffer->data() << std::endl;
        msgpack::object_handle  msg = msgpack::unpack(buffer->data(), buffer->size());
        auto tp = msg.get().as<std::tuple<int>>();
        std::cout << " magpack " << std::get<0>(tp) << std::endl;
        result = std::get<0>(tp);

    }

private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    tcp::endpoint& endpoint_;
    std::shared_ptr<std::array<char, MAXPACKSIZE>> buffer;
    int result;
};

typedef boost::shared_ptr<client> client_ptr;

int main()
{
    boost::asio::io_service io_service;
    tcp::endpoint endpoint(address::from_string("127.0.0.1"), 2019);

    client_ptr new_session(new client(io_service, endpoint));
    new_session->start(1,2);
    io_service.run();

    return 0;
}
