ThreadPool
==========

A simple C++11 Thread Pool implementation.

Basic usage:
```c++
// create thread pool with 4 worker threads
ThreadPool pool(4);

// enqueue and store future
auto result = pool.enqueue([](int answer) { return answer; }, 42);

// get result from future
std::cout << result.get() << std::endl;

```

PtrThreadPool
Added c++20 support and thread assosiated ptr

Pass ptrs in ctor: PtrThreadPool(std::vector<void*>)
Get ptr with: static void **ptr() 
See example cpp
