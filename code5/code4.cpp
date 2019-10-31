#include <string>
#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#include <msgpack.hpp>

class test_client
{
public:
    test_client(const test_client&) = delete;
    test_client& operator=(const test_client&) = delete;
    test_client(boost::asio::io_service& io_service)
        : io_service_(io_service),
        socket_(io_service) {}

    //根据 IP 和端口连接服务器
    void connect(const std::string& addr, const std::string& port) {
        tcp::resolver resolver(io_service_);
        tcp::resolver::query query(tcp::v4(), addr, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::asio::connect(socket_, endpoint_iterator);
    }

    //发起 RPC 调用
    void call(const char* data, size_t size) {
        //发送 rpc 请求
        bool r = send(data, size);
        if (!r) {
            throw std::runtime_error("call failed");
        }

        //读来自服务器返回的内容
        read_head();
    }

    void call(std::string content) {
        return call(content.data(), content.size());
    }

private:
    std::vector<char> data_;
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    //将请求打包为 msgpack 格式发送到服务器
    bool send(const char* data, size_t size) {
        size_t size_bigend = boost::asio::detail::socket_ops::host_to_network_long(size);
        std::vector<boost::asio::const_buffer> message;
        message.push_back(boost::asio::buffer(&size_bigend, 4));
        message.push_back(boost::asio::buffer(data, size));
        boost::system::error_code ec;
        boost::asio::write(socket_, message, ec);
        if (ec) {
            return false;
        }
        else {
            return true;
        }
    }
    void read_head()
    {
        char head_[4];

        boost::asio::read(socket_, boost::asio::buffer(head_, 4));

        const int body_len = *((int*)(head_));
        if (body_len > 0 && body_len < 1024)
        {
            if (data_.size() < body_len)
            {
                data_.resize(body_len);
            }
            read_body(body_len);
        }

    }
    void read_body(std::size_t size)
    {

        boost::asio::read(socket_, boost::asio::buffer(data_.data(), size));
        msgpack::object_handle  msg;
        try {
            msg = msgpack::unpack(data_.data(), data_.size());

            auto tp = msg.get().as<std::tuple<int, std::string>>();
            std::cout << "client: " << std::get<0>(tp) << " " << std::get<1>(tp) << " " << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }



};

void test() {
    try {
        boost::asio::io_service io_service;
        test_client client(io_service);
        client.connect("127.0.0.1", "9000");

        std::tuple<int, std::string> src(20, "hello tom");
        std::stringstream buffer;
        msgpack::pack(buffer, src);

        std::string str(buffer.str());
        client.call(str);
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int main() {
    test();
    std::cout << "client stoped" << std::endl;
}
