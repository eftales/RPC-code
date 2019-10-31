#include <iostream>

int main() {
    auto lambda = [] {
        std::cout << "hello lambda!" << std::endl;
    };
    lambda();
}
