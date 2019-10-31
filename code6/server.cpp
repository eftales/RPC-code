#include<string>
#include<iostream>
#include "rpc.h"


class session
	: 	public boost::enable_shared_from_this<session> {
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

int main(int argc, char* argv[])
{
	boost::asio::io_service io_service;
	tcp::endpoint endpoint(tcp::v4(), 2019);

	server s(io_service, endpoint);
	s.run();
	return 0;
}
