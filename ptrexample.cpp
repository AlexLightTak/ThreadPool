#include <iostream>
#include <vector>
#include <chrono>
#include <format>

#include "ptrthreadpool.h"

int main()
{
    int a=1,b=2,c=3;
    PtrThreadPool pool({&a, &b, &c});
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                auto p = (int *)(*PtrThreadPool::ptr());
                std::cout << std::format("p:{}\n", *p);
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    
    return 0;
}
