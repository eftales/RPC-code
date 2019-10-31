#include <iostream>
#include <algorithm>
#include <vector>

int main() {
    auto lambda = []{ std::cout<<"hello lambda!"<<std::endl; };
    lambda();

    auto lambda1 = [](int i)->int { return i; };
    auto lambda2 = [](int i) { return i; };

    std::cout<<lambda1(1)<<std::endl;
    std::cout<<lambda2(2)<<std::endl;

    std::vector<int> v{1,2,3,4,5,6,7};
    std::for_each(v.begin(), v.end(), [](int i){
        std::cout<<i<<" ";
    });
    std::cout<<std::endl;
    return 0;
}
