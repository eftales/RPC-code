#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

// 包含 asio 和 msgpack 的头文件
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


#include<string>
#include<iostream>

// 双方需要约定的特殊常量
#define NOTAPPLICATED -3000
#define MAXPACKSIZE 1024

// 函数映射表
#define ADD 1
#define MINUS 2
#define MULTI 3
#define DIV 4

// 客户端发送给服务端的报文格式：4 字节的整数代表函数映射表，4 字节的整数表示 msgpack 的长度，后面不定长的部分为 msgpack 包。
// 服务端发送给客户端的报文格式：1 个整数序列化之后的 msgpack 包



// 客户端类
class client : public boost::enable_shared_from_this<client> {
public:
    client(boost::asio::io_service& io_service, tcp::endpoint& endpoint)
        : io_service_(io_service), socket_(io_service), endpoint_(endpoint)
    {
        buffer = std::make_shared<std::array<char, MAXPACKSIZE>>();
        result = 0;
    }
    int add(int a,int b)
    {
        return start(ADD,a,b);
    }
    int minus(int a, int b)
    {
        return start(MINUS, a, b);
    }
    int multi(int a, int b)
    {
        return start(MULTI, a, b);
    }
    int div(int a, int b)
    {
        return start(DIV, a, b);
    }
    int start(int opt, int a, int b) {
        boost::system::error_code ec;
        socket_.connect(endpoint_, ec);
        if (!ec)
        {
            static tcp::no_delay option(true);
            socket_.set_option(option);

            construct_rpc_data(opt, a , b);
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
    void construct_rpc_data(size_t opt, int a, int b)
    {
        std::cout<< " opt " << opt << std::endl;
        size_t opt_bigend = boost::asio::detail::socket_ops::host_to_network_long(opt);
        memcpy(buffer->data(), &opt_bigend, 4);

//#define protocol_to_client std::tuple<int>
        std::tuple<int, int>  src(a,b);
        std::stringstream sbuffer;
        msgpack::pack(sbuffer, src);
        std::string strbuf(sbuffer.str());

        std::cout << " len " << strbuf.size() << std::endl;
        size_t len_bigend = boost::asio::detail::socket_ops::host_to_network_long(strbuf.size());
        memcpy(buffer->data()+4, &len_bigend, 4);
        memcpy(buffer->data() + 8, strbuf.data(), strbuf.size());
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


// 服务端类

class session
    :   public boost::enable_shared_from_this<session> {
public:
    session(boost::asio::io_service &io_service) : io_service_(io_service),socket_(io_service)
    {
        buffer = std::make_shared<std::array<char, MAXPACKSIZE>>();
        *len_ = '\0';
        *opt_ = '\0';

    }

    void start() {

        static tcp::no_delay option(true);
        socket_.set_option(option);
        start_chains();

    }

    tcp::socket& socket() {
        return socket_;
    }

private:
    void start_chains()
    {
        read_opt();
    }
    void read_opt()
    {
        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        boost::asio::async_read(socket_, boost::asio::buffer(opt_, 4),
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {


                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " opt 数据接收完成" << std::endl;
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " 原始数据 " << opt_ << std::endl;

                opt = boost::asio::detail::socket_ops::network_to_host_long(int(*(int *)opt_));
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " opt " << opt <<std::endl;

                read_msgpack_len();
            });
        //io_service_.run();

    }

    void read_msgpack_len()
    {
        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        boost::asio::async_read(socket_, boost::asio::buffer(len_, 4),
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {


                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " len 数据接收完成" << std::endl;
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " 原始数据 " << len_ << std::endl;

                len = boost::asio::detail::socket_ops::network_to_host_long(int(*(int*)len_));
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " len " << opt << std::endl;

                read_msgpack();
            });
        //io_service_.run();


    }

    void read_msgpack()
    {
        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        boost::asio::async_read(socket_, boost::asio::buffer(async_buffer->data(), len),
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {
                if (ec)
                {
                    std::cout << ec.message() << std::endl;
                    return;
                }

                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " magpack 数据接收完成" << std::endl;
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " 原始数据 " << async_buffer->data() << std::endl;


                msg = msgpack::unpack(async_buffer->data(), len);

                rpc_caculate_return();
            });

        //io_service_.run();
    }

    void rpc_caculate_return()
    {
        std::cout << "进入rpc_caculate_return" << std::endl;
        auto tp = msg.get().as<std::tuple<int, int> >();
        std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " magpack " << std::get<0>(tp) << " " << std::get<1>(tp) << std::endl;


        int result = 0;
        switch (opt)
        {
        case ADD:
            result = std::get<0>(tp) + std::get<1>(tp);
            break;

        case MINUS:
            result = std::get<0>(tp) - std::get<1>(tp);
            break;

        case MULTI:
            result = std::get<0>(tp) * std::get<1>(tp);
            break;
        case DIV:
            if (std::get<1>(tp) == 0)
                result = NOTAPPLICATED;
            result = std::get<0>(tp) / std::get<1>(tp);
            break;

        }
        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        std::tuple<int>  src(result);
        std::stringstream sbuffer;
        msgpack::pack(sbuffer, src);

        std::string strbuff(sbuffer.str());

        memcpy(async_buffer->data(), strbuff.data(), strbuff.size());

        std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " rpc 计算完成" << std::endl;
        boost::asio::async_write(socket_, boost::asio::buffer(async_buffer->data(), async_buffer->size()),//
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {
                if (ec)
                {
                    std::cout << ec.message() << std::endl;
                    return;
                }
                std::cout << socket_.remote_endpoint().address()<< ":" <<socket_.remote_endpoint().port()<<" rpc 成功" << std::endl;


                //io_service_.stop();
            });
        //io_service_.run();

    }

private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    boost::asio::streambuf sbuf_;
    std::shared_ptr<std::array<char, MAXPACKSIZE>> buffer;
    char len_[4];
    int len;
    char opt_[4];
    int opt;
    msgpack::object_handle  msg;
};

typedef boost::shared_ptr<session> session_ptr;

class server {
public:
    server(boost::asio::io_service& io_service, tcp::endpoint& endpoint)
        : io_service_(io_service), acceptor_(io_service, endpoint)
    {
        session_ptr new_session(new session(io_service_));
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept,
                this,
                new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session_ptr new_session, const boost::system::error_code& error) {
        if (error) {
            return;
        }

        new_session->start();
        //io_service_.restart();

        new_session.reset(new session(io_service_));
        acceptor_.async_accept(new_session->socket(), boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));

        io_service_.run();
    }

    void run() {
        io_service_.run();
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};
#endif
