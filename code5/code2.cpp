#include <iostream>
#include <string>
#include <sstream>
#include <msgpack.hpp>

class person {
public:
    //person() :name("") { age = 0; id = 0; };
    person(int id_ = 0, std::string name_ = "", int age_ = 0) :name(name_) { age = age_; id = id_; };

    int id;
    std::string name;
    int age;
    MSGPACK_DEFINE(id, name, age); // 申明这个类需要序列化
    void disply() {
        std::cout << id << " " << name << " " << age << std::endl;
    };
};

void test() {

    person src(1, "tom", 20 );
    std::stringstream buffer;
    msgpack::pack(buffer, src); // 将自定义类序列化

    std::string str(buffer.str());

    msgpack::object_handle oh = msgpack::unpack(str.data(), str.size()); // 反序列化
    msgpack::object deserialized = oh.get();
    try {
        person dst = deserialized.as<person>(); // 得到类的实例
        dst.disply(); // 调用类的方法

    }
    catch (...)
    {
        throw std::invalid_argument("Args not match!");
    }


}

int main(void)
{
    test();
    return 0;
}
