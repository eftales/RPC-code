#include <iostream>
#include <functional>

int main() {

    auto lambda1 = [](int i)->int { printf("%d\n", i);  return i; };
    std::function<int(int)> lambdafunc = (std::function<int(int)>)lambda1;
    lambdafunc(11);
    return 0;

}
