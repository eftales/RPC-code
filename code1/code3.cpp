#include <iostream>
#include <string>
#include <functional>

std::function<void()> test(){
    std::string str = "hello";
    auto lambda = [&str]{
        std::cout<<str<<" "<<"lambda";
    };

    return lambda;
}

int main() {
    auto lambda1 = test();
    lambda1();
}
