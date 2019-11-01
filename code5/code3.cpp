#include<string>
#include<iostream>
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



class session
    :   public boost::enable_shared_from_this<session> {
public:
    session(boost::asio::io_service &io_service) : io_service_(io_service),socket_(io_service)
    {
        buffer = std::make_shared<std::array<char, MAXPACKSIZE>>();
        *len_ = '\0'; // 初始化成员变量
        *opt_ = '\0';

    }

    void start() {

        static tcp::no_delay option(true);
        socket_.set_option(option); // 设置 socket 为无延时模式
        start_chains(); // 开始 读取头部 -> 读取 msgpack 包 -> 读取头部 的循环

    }

    tcp::socket& socket() {
        return socket_;
    }

private:
    void start_chains()
    {
        read_msgpack_len();
    }


    void read_msgpack_len() // 读取包的长度
    {
        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        boost::asio::async_read(socket_, boost::asio::buffer(len_, 4),
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {


                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " len 数据接收完成" << std::endl;
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " 原始数据 " << len_ << std::endl;

                len = boost::asio::detail::socket_ops::network_to_host_long(int(*(int*)len_)); // 转换为主机字节序
                std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " len " << opt << std::endl;

                read_msgpack(); // 读取 msgpack 包
            });

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


                msg = msgpack::unpack(async_buffer->data(), len); // 反序列化

                send_to_client();

            });

    }

    void send_to_client()
    {
        std::cout << "进入 send_to_client" << std::endl;
        auto tp = msg.get().as<std::tuple<int, int> >();
        std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " magpack " << std::get<0>(tp) << " " << std::get<1>(tp) << std::endl;


        int result = 23333; // 要发送的数据

        auto self = this->shared_from_this();
        auto async_buffer = buffer;

        std::tuple<int>  src(result);
        std::stringstream sbuffer;
        msgpack::pack(sbuffer, src); // 序列化

        std::string strbuff(sbuffer.str());

        memcpy(async_buffer->data(), strbuff.data(), strbuff.size());

        std::cout << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << " 服务端序列化完成" << std::endl;
        boost::asio::async_write(socket_, boost::asio::buffer(async_buffer->data(), async_buffer->size()),//
            [this, self, async_buffer](const boost::system::error_code& ec, std::size_t size)
            {
                if (ec)
                {
                    std::cout << ec.message() << std::endl;
                    return;
                }
                std::cout << socket_.remote_endpoint().address()<< ":" <<socket_.remote_endpoint().port()<<" 服务端发送成功" << std::endl;

            });

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
        session_ptr new_session(new session(io_service_)); // 指向 session 类
        acceptor_.async_accept(new_session->socket(), // 异步接收连接，如果有连接就调用 handle_accept() 成员函数
            boost::bind(&server::handle_accept,
                this,
                new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session_ptr new_session, const boost::system::error_code& error) {
        if (error) {
            return;
        }

        new_session->start(); // 调用 session 类的 start() 成员函数


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

int main(int argc, char* argv[])
{
    boost::asio::io_service io_service; // 定义 io_service
    tcp::endpoint endpoint(tcp::v4(), 2019); // 设置协议与端口号

    server s(io_service, endpoint); // 实例化服务器，并开始运行
    s.run();
    return 0;
}
