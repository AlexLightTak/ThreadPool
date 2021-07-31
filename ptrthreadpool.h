// alight@3107.ru
// Initial version from https://github.com/progschj/ThreadPool/blob/master/ThreadPool.h
// Added worker assosiated ptr and c++20
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class PtrThreadPool
{
public:
    static void **ptr()
    {
        thread_local void *p;
        return &p;
    }
    PtrThreadPool(size_t);
    // We can add parameters for concrete worker
    PtrThreadPool(std::vector<void *>);
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<decltype(std::declval<F>()(std::declval<Args>()...))>;
    ~PtrThreadPool();

private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<std::function<void()>> tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
inline PtrThreadPool::PtrThreadPool(size_t threads)
    : PtrThreadPool(std::vector<void *>(threads, nullptr))
{
}

inline PtrThreadPool::PtrThreadPool(std::vector<void *> ptrs)
    : stop(false)
{
    for (auto p : ptrs)
        workers.emplace_back(
            [this, p]
            {
                // Set ptr to thread_local var
                *ptr() = p;
                for (;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                                             [this]
                                             { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            });
}

// add new work item to the pool
template <class F, class... Args>
auto PtrThreadPool::enqueue(F &&f, Args &&...args)
    -> std::future<decltype(std::declval<F>()(std::declval<Args>()...))>
{
    using return_type = decltype(std::declval<F>()(std::declval<Args>()...));

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]()
                      { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline PtrThreadPool::~PtrThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

#endif
