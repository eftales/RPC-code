#include <iostream>
#include <functional>
#include <thread>
#include <memory>

char msg1[10] = "hhh";
char msg2[10] = "aaa";
char msg3[10] = "ooo";

void test(void* msg) {

    std::cout << "hello thread " << (char*)msg << std::endl;
}

struct person {
    person() : thd_(std::bind(&person::test, this, (void*)msg2)) {
        thd_ptr_ = std::make_shared<std::thread>([this] {
            test((void*)msg3);
            });
    }

    ~person() {
        thd_.join();
        if (thd_ptr_.use_count() != 0)
            thd_ptr_->join();
        else
            std::cout << "thd_ptr_ 早已退出" << std::endl;
    }

    void test(void* msg) {
        std::cout << "hello thread " << (char*)msg << std::endl;
    }

    std::thread thd_;
    std::shared_ptr<std::thread> thd_ptr_;
};

int main() {

    std::thread thd(test, (void*)msg1);
    thd.detach();

    person p;

}
