#ifndef _THREAD_POOL_HPP_
#define _THREAD_POOL_HPP_

#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>

class ThreadPool {
public:
    using task_type = std::function<void()>;

    explicit ThreadPool(int nthread);
    ~ThreadPool();

    template<class F, class... Args>
    auto add_task(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    //C++11 或者采用这样定义
    //template<class F, class... Args>
    //std::future<typename std::result_of<F(Args...)>::type>
    //add_job(F&& f, Args&&... args);
    
private:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) =delete ;
    ThreadPool& operator=(ThreadPool&&) = delete;

private:    
    std::vector<std::thread> m_threads;
    std::deque<task_type> m_tasks; 

    std::mutex m_mtx;
    std::condition_variable m_cnd;
    bool m_stop;      // 是否需要原子类型?
};

inline ThreadPool::ThreadPool(int nthread) 
    : m_stop(false) {
    for (int i = 0; i < nthread; i++) {
        m_threads.emplace_back( [this] {  // 采用emplace，传入function会隐式转为thread
            // 采用this，实际是采用传值方式，具体见ModernEffective中讨论
            while(!this->m_stop) {
                task_type task;
                {
                    std::unique_lock<std::mutex> lk(this->m_mtx);
                    this->m_cnd.wait(lk, [this] {
                        return this->m_stop || !this->m_tasks.empty(); });
                    if (this->m_stop) { return ; }
                    // 到这个地方，有锁保护，有m_cnd的判断，那么tasks就不会为空
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop_front();
                }
                task();
            }
        } );
    }
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        m_stop = true;
    }
    m_cnd.notify_all();
    for (auto& worker : m_threads) {
        worker.join();
    }
}

template<class F, class... Args> 
auto ThreadPool::add_task (F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = std::future<typename std::result_of<F(Args...)>::stype>;
    auto task = std::make_shared<std::packaged_task<return_type>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...) );

    //std::future<return_type> res = task->get_future();
    auto res = task->get_future();
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        if (m_stop) {
            throw std::runtime_error("add_task on stopped ThreadPool");
        }
        //m_tasks.emplace_back(task);
        m_tasks.emplace_back([task]() { (*task)(); });
    }
    m_cnd.notify_one();
    return res;
}

#endif
