#include <iostream>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>

class counter{
public:
    void increase(){
        std::this_thread::sleep_for(std::chrono::microseconds(20));
        std::cout<<"thread id: "<<std::this_thread::get_id()<<std::endl;
        mtx_.lock();
        counter_++;
        mtx_.unlock();
    }

    void increase1(){
        std::this_thread::sleep_for(std::chrono::microseconds(20));
        std::cout<<"thread id: "<<std::this_thread::get_id()<<std::endl;
        std::unique_lock<std::mutex> lock(mtx_);
        counter_++;
    }

    void print_counter() {
        std::unique_lock<std::mutex> lock(mtx_);
        std::cout<<"thread id: "<<std::this_thread::get_id()<<" counter: "<<counter_<<std::endl;
    }

private:
    int counter_ = 0;
    std::mutex mtx_;
};

int main(){
    counter ct;

    std::thread thd1([&ct]{
        ct.increase();
    });

    std::thread thd2([&ct]{
        ct.increase1();
    });

    std::thread thd3([&ct]{
        ct.print_counter();
    });

    thd1.join();
    thd2.join();
    thd3.join();
}
