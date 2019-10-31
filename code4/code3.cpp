#include <iostream>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try{
        boost::asio::io_service io_service;

        tcp::socket s(io_service);
        tcp::resolver resolver(io_service); // 存储服务器的地址信息（IP，端口）。

        boost::asio::connect(s, resolver.resolve({"127.0.0.1", "9000"})); // 连接服务器，执行 TCP 三次握手的过程。
        std::this_thread::sleep_for(std::chrono::seconds(2)); // 本线程睡眠两秒

    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
