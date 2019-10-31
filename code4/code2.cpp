#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class server{
public:
    server(boost::asio::io_service& ios, short port) : ios_(ios),
    acceptor_(ios, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

    void do_accept()
    {
        std::cout<<"begin to listen and accept"<<std::endl;
        auto socket = std::make_shared<tcp::socket>(ios_); // 使用智能指针保证安全回调。
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec)
                {
                    if (ec){ // 如果发送错误，则打印错误信息，并关闭套接字。
                        std::cout<<ec.message()<<std::endl;
                        socket->close();
                    }else{ // 没有错误，将套接字存入套接字队列中。
                        conns_.push_back(socket);
                        std::cout<<"new connection coming"<<std::endl;
                    }

                   do_accept(); // 无论是否发送错误，都要自己调用自己，继续监听端口。
                });
    }

private:
    boost::asio::io_service& ios_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<tcp::socket>> conns_;
};

int main() {
    boost::asio::io_service ios;

    server s(ios, 9000); //实例化一个服务器类，监听端口 9000

    boost::asio::deadline_timer timer(ios);
    timer.expires_from_now(boost::posix_time::seconds(5)); // 设置超时时间为 5s。

    timer.async_wait([&timer,&ios](const boost::system::error_code& ec){ // 异步等待超时
    if(ec){
        std::cout<<ec.message()<<std::endl;
        return;
    }

    ios.stop(); // 到 5s 之后手动停止 io_service。因为 server.do_accept() 一直调用自己，所以 io_service 上一直会有事件发生，不会自动停止。
    std::cout<<"server stoped"<<std::endl;
    });

    ios.run();


    return 0;
}
