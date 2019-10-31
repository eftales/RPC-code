#include <msgpack.hpp>
#include <string>
#include <iostream>
#include <sstream>

int main()
{
    msgpack::type::tuple<bool, char, std::string> src(true, 'i', "shiyanlou");

    std::stringstream buffer;
    // 序列化
    msgpack::pack(buffer, src);


    std::string str(buffer.str());

    // 反序列化
    msgpack::object_handle oh = msgpack::unpack(str.data(), str.size());

    msgpack::object deserialized = oh.get();

    std::cout << deserialized << std::endl;

    // 两种把 msgpack::object_handle 转化为 msgpack::type::tuple 的方法
    msgpack::type::tuple<bool, char, std::string> dst;
    deserialized.convert(dst);

    msgpack::type::tuple<bool, char, std::string> dst2 =    deserialized.as<msgpack::type::tuple<bool, char, std::string> >();

    return 0;
}
