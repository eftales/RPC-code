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

    auto lambda2 = [a,b,c]() mutable{
        a = 1;
        b = 2;
        c = 3;
        std::cout<<"in lambda2 :"<<a<<" "<<b<<" "<<c<<std::endl;
    };

    lambda2();

    std::cout<<a<<" "<<b<<" "<<c<<std::endl;

    auto lambda3 = [=]() mutable{
        a = 10;
        b = 20;
        c = 30;
        std::cout<<"in lambda3 :"<<a<<" "<<b<<" "<<c<<std::endl;
    };

    lambda3();

    std::cout<<a<<" "<<b<<" "<<c<<std::endl;

}
