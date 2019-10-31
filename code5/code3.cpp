#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <string>
#include <chrono>
#include <boost/asio.hpp>
#include <msgpack.hpp>

using boost::asio::ip::tcp;

class connection
    : public std::enable_shared_from_this<connection>
{
public:
    connection(boost::asio::io_service& ios)
        : ios_(ios), socket_(ios)
    {
        head_[0] = '\0';
        data_.clear();
    }

    void start()
    {
        read_head();
    }

    tcp::socket& socket() {
        return socket_;
    }

    ~connection() {
        close();
    }

private:
    void read_head()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(head_, 4),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout <<" length = "<< length << std::endl;
                    const int body_len = boost::asio::detail::socket_ops::network_to_host_long(*((int*)(head_)));
                    if (body_len > 0 && body_len < 1024) {
                        if (data_.size() < body_len) { data_.resize(body_len); }
                        read_body(body_len);

                        std::tuple<int, std::string> src(10, "hello client");
                        std::stringstream buffer;
                        msgpack::pack(buffer, src);

                        std::string str(buffer.str());

                        int size = str.size();
                        const char* content = str.data();
                        std::vector<boost::asio::const_buffer> message;
                        message.push_back(boost::asio::buffer(&size, 4));
                        message.push_back(boost::asio::buffer(content,size));
                        boost::asio::write(self->socket_, message);
                    }
                }
                else {
                    close();
                }
            });
    }

    void read_body(std::size_t size) {
        auto self(this->shared_from_this());
        boost::asio::async_read(
            socket_, boost::asio::buffer(data_.data(), size),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    msgpack::object_handle  msg;
                    try {
                        msg = msgpack::unpack(data_.data(), data_.size());

                        auto tp = msg.get().as<std::tuple<int, std::string>>();
                        std::cout << "server: " << std::get<0>(tp) << " " << std::get<1>(tp) << " " << std::endl;
                    }
                    catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                        return;
                    }

                    read_head();
                }
                else {
                    std::cout << ec.message() << std::endl;
                }
            });
    }

    void close() {
        if (socket_.is_open()) {
            boost::system::error_code ignored_ec;
            socket_.shutdown(tcp::socket::shutdown_both, ignored_ec);
            socket_.close(ignored_ec);
        }
    }

    boost::asio::io_service& ios_;
    tcp::socket socket_;
    char head_[4];
    std::vector<char> data_;
};

class server {
public:
    server(boost::asio::io_service& ios, short port) : ios_(ios),
        acceptor_(ios, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

    void do_accept()
    {
        std::cout << "begin to listen and accept" << std::endl;
        auto conn = std::make_shared<connection>(ios_);
        acceptor_.async_accept(conn->socket(), [this, conn](boost::system::error_code ec)
            {
                if (ec) {
                    std::cout << ec.message() << std::endl;
                }
                else {
                    conn->start();
                    std::cout << "new connection coming" << std::endl;
                }

                do_accept();
            });
    }


private:
    boost::asio::io_service& ios_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<tcp::socket>> conns_;
};

int main() {
    boost::asio::io_service ios;

    boost::asio::deadline_timer timer(ios);
    timer.expires_from_now(boost::posix_time::seconds(5));
    timer.async_wait([&timer, &ios](const boost::system::error_code& ec) {
        if (ec) {
            std::cout << ec.message() << std::endl;
            return;
        }

        ios.stop();
        std::cout << "server stoped" << std::endl;
        });
    server s(ios, 9000);
    ios.run();

    return 0;
}

