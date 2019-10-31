#include <iostream>

int main() {
    int a = 1;
    int b = 2;
    int c = 3;

    auto lambda1 = [&]{
        a = 4;
        b = 5;
        c = 6;
    };

    lambda1();

    std::cout<<a<<" "<<b<<" "<<c<<std::endl;
}
