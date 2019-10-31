#ifndef __RPC_H__
#define __RPC_H__

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
#define ADD 1
#define MINUS 2
#define MULTI 3
#define DIV 4

#endif
