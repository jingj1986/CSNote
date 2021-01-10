## 基本使用

[C++ 并发编程实战 第二版](https://www.bookstack.cn/books/CPP-Concurrency-In-Action-2ed-2019) 前5章有比较多的基础例子；
[C++ 并发编程（从C++11到C++17）](https://paul.pub/cpp-concurrency/)算是更加快速的方式梳理了一遍并发编程的使用。

本处仅仅选用 生产-消费 模型，来说明多线程的使用方式， 其基本逻辑如下：

```
// 定义需要的锁与条件变量
mutex m_mtx;  
condition_variable m_cnd;

// 生产者
void product() {
    while (循环的条件) {
        // ...  前面的准备

        // 锁与条件变量
        {
            unique_lock<mutex> lk(m_mtx);
            m_cnd.wait(lk, []{return 需要的条件;});
            // ... 一些临界数据设置，如 生产数据入队列
            notify_one();
        }

        //... 其他处理， 减少锁的区域
    }   
}

// 消费者
void custom() {
    while (循环条件) {
        // ... 前面的准备

        // 锁与条件变量
        {
            unique_lock<mutex> lk(m_mtx);
            m_cnd.wait(lk, []{return 需要的条件;});
            // ... 从临界数据获取数据，如队列取，并去掉已取的数据
            notify_one();
        }

        // ... 其他处理
    }
}

// 使用锁，就限制对临界数据的竞争操作
// 使用条件变量，在某一方条件满足时，另一方可执行相应操作
```

可参考本文档提供的[源码例子](src/product-consum/v1.cpp) 


## 线程池

[C++ 并发编程实战 第二版](https://www.bookstack.cn/books/CPP-Concurrency-In-Action-2ed-2019) 中讲述了几种线程池的使用模式

- 简单线程池

    使用vector<thred>存放线程（worker），使用之前的线程安全队列`thread_safe_queue`来存放待执行的任务，其内部使用了锁机制，可以直接调用其接口执行任务的增加与获取。
```
// worker 的执行线程
void worker_thread()
{
    while(!done)  // 4
    {
        std::function<void()> task; 
        if(work_queue.try_pop(task)) {  // 从任务队列中取
            task();  // 执行任务
        } else {
            std::this_thread::yield();
        }
    }
}

// 添加任务
template<typename FunctionType>
void submit(FunctionType f)  {
    work_queue.push(std::function<void()>(f));  // 直接向任务队列push即可
}
```

- 线程池自己实现锁

    如果不使用程安全队列`thread_safe_queue`，而是线程池内部自己通过锁与条件变量来控制任务的添加与获取，这种方式也比较常见。在讨论Linux Phtread时，就有实现。而现在有C++11 的thread库，可以充分借助C++11特性，将一些风骚的操作发挥出来，以下是 [ progschj/ThreadPool ](https://github.com/progschj/ThreadPool)实现方式的主要逻辑

```
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
        m_tasks.emplace_back([task]() { (*task)(); });  // 添加task，函数形式的task
    }
    m_cnd.notify_one(); //
    return res;
}
```
-
-
## 无锁


