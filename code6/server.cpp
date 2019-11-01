#include"interface.hpp"

int main(int argc, char* argv[])
{
	boost::asio::io_service io_service;
	tcp::endpoint endpoint(tcp::v4(), 2019);

	server s(io_service, endpoint);
	s.run();
	return 0;
}
