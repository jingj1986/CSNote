## C++11 原子与线程API

### 1 thread

#### 1.1 thread与pthread对比

| C++11 lib |  pthread |
| --- | --- |
| hread() noexcept | pthread_create |
| ~thread() | pthread_destroy |
| thread::id get_id() | pthread_t pthread_self() |
| joinable |  |
| join | pthread_join | 
| detach | pthread_detach |
| native_handle | |
| hardware_concurrency[static] | |
| thread::swap |  |
| std::swap | |

#### 1.2 构造函数与赋值

```
// thread
default (1)              thread() noexcept;   
initialization (2)       template <class Fn, class... Args>  
                           explicit thread (Fn&& fn, Args&&... args);
copy [deleted] (3)     thread (const thread&) = delete;
move (4)               thread (thread&& x) noexcept;

move (1)                thread& operator= (thread&& rhs) noexcept;
copy [deleted] (2)      thread& operator= (const thread&) = delete;

//pthread
pthread_create
```

### 2 mutex

#### 2.1 包含的内容

|   | 类名称  | 类型 |
| --- | --- | --- |
| mutexes | mutex  recursive_mutex   timed_mutex   recursive_timed_mutex |  class |
| locks | lock_guard  unique_lock | class template |
| other types | once_flag  adopt_lock_t   defer_lock_t   try_to_lock_t | class |
| functions | try_lock  lock  call_once |  function template |

#### 2.2 mutexes 几个不同类包含的成员函数

|  | mutex | recursive_mutex | timed_mutex | recursive_timed_mutex |
| --- | --- | --- | --- | --- |
| lock | Y | Y | Y | Y |
| try_lock | Y | Y | Y | Y |
| try_lock_for |  |  | Y | Y |
| try_lock_until |  |  | Y | Y |
| unlock | Y | Y | Y | Y |
| native_handle | Y | Y | Y | Y |

#### 2.3 与pthread函数比较

| C++11 lib | pthread |
| --- | --- | --- |
| mutex() | pthread_mutex_init |
| ~mutex() | pthread_mutex_destroy | 
| lock | pthread_mutex_lock |
| try_lock | ptrhead_mutex_trylock | 
| unlock | pthread_mutex_unlock |
| native_handle | |

#### 2.4 lock 模板类

```
// 锁在当前区域内有效，该区域上下文结束，锁自动释放;仅构造函数与析构函数
template <class Mutex> class lock_guard;
// 在lock_guard基础上添加更多成员函数，更加灵活 
template <class Mutex> class unique_lock;
```
 可理解为智能指针，可以自动释放；这儿是自动解锁

##### 2.4.1 unique_lock 函数

| 功能 | 函数 |
| --- | --- |
| Locking/unlocking | lock  try_lock  try_lock_for  try_lock_until  unlock |
| Modifiers | operator=  swap  release |
| Observers | owns_lock  operator bool  mutex |
| Non-member | swap |

##### 2.4.2  unique_lock 构造、赋值函数

```
default       (1)    unique_lock() noexcept;
locking       (2)    explicit unique_lock (mutex_type& m);
try-locking   (3)    unique_lock (mutex_type& m, try_to_lock_t tag);
deferred      (4)    unique_lock (mutex_type& m, defer_lock_t tag) noexcept;
adopting      (5)    unique_lock (mutex_type& m, adopt_lock_t tag);
locking for   (6)    template <class Rep, class Period> 
                     unique_lock (mutex_type& m, const chrono::duration<Rep,Period>& rel_time);
locking until (7)    template <class Clock, class Duration>
                     unique_lock (mutex_type& m, const chrono::time_point<Clock,Duration>& abs_time);
copy [deleted] (8)   unique_lock (const unique_lock&) = delete;
move           (9)   unique_lock (unique_lock&& x);

move (1)             unique_lock& operator= (unique_lock&& x) noexcept;
copy [deleted] (2)   unique_lock& operator= (const unique_lock&) = delete;
```

#### 2.5 mutex 其他类型

| 类型 | 说明 |
| --- | --- |
| once_flag | 此类型的对象用作call_once的参数 |
| adopt_lock_t | 是一个空的类，用作adopt_lock的使用的类型 |
| defer_lock_t | 这是一个空类，用作延迟锁的类型 |
| try_to_lock_t | 用于try_to_lock类型的空类 |

#### 2.6 mutex中的函数模板

```
template <class Mutex1, class Mutex2, class... Mutexes>
      int try_lock (Mutex1& a, Mutex2& b, Mutexes&... cde);

template <class Mutex1, class Mutex2, class... Mutexes>
      void lock (Mutex1& a, Mutex2& b, Mutexes&... cde);

template <class Fn, class... Args>
          void call_once (once_flag& flag, Fn&& fn, Args&&... args);
```


### 3 condition_variable

包含的两个类，一个枚举类、一个函数，结构定义类似：

```
namespace std {
 
    class condition_variable;		// 提供了std::unique_lock 相关联的条件变量
    class condition_variable_any;	// 提供与任何锁类型相关联的条件变量

	//当这个线程完全完成(函数)时，调度调用notify_all来调用 
    void notify_all_at_thread_exit(condition_variable& cond, unique_lock<mutex> lk);
 
    enum class cv_status {
        no_timeout,
        timeout
    };
}
```

#### 3.1 成员函数与对比

| 功能 |  condition_variable  |  condition_variable_any | pthread |
| --- | --- | --- | --- |
| 创建 | 构造函数 | 构造函数 | pthread_cond_init |
| 销毁 | 析构函数 | 析构函数 | pthread_cond_destory |
| 通知 | notify_all<br>notify_one | notify_all<br>notify_one | pthread_cond_broadcast<br>pthread_cond_signal |
| 等待 | wait<br>wait_for<br>wait_util |wait<br>wait_for<br>wait_util | pthread_cond_wait |



### 4 atomic

包括 `atomic` 和 `atomic_flag` 两个类，后者是无锁布尔原子类。

#### 4.1 封装类型


基本逻辑为`using atomic_int = atomic<int>`，会有一些转变，包括 :`unsigned xxx` -> `uxxx`, `long long`-> `llong`；
还有一个`signed char` -> `schar`的转变。
目前定义的类型有

| type | org | unsigned | 说明 |
| --- | --- | --- | --- | 
| bool | Y | | | 
| char | Y | Y | |
| int | Y | Y | | 
| short | Y | Y | |
| long | Y | Y | |
| long long | Y | Y |  |
| wchar_t | Y | | |
| charN_t | Y | | N: 16/32 |
| intmax_t | Y | | | 
| uintmax_t | Y | | |
| int_leastN_t | Y | | N:8/16/32/64 |
| uint_leastN_t | Y | | N:8/16/32/64 |
| int_fastN_t | Y | | N:8/16/32/64 |
| uint_fastN_t | Y | | N:8/16/32/64 |
| intptr_t | Y | | |
| uintptr_t | Y | | |
| size_t | Y | | |
| ptrdiff_t | Y | | |

#### 4.2 操作函数

C++11的原子库提供了函数，一种是 `std::atomic_xxx`的 c类型函数，参数会有一个atomic对象或flag；一种是类的成员函数，命名不带 `atomic_`。<br>
我们以`atomic_fetch_add` 和 成员函数`fetch_add`比较，其函数声明分别如下

```
// atomic_fetch_add
template (integral) (1) template <class T> T atomic_fetch_add (volatile atomic<T>* obj, T val) noexcept;
                        template <class T> T atomic_fetch_add (atomic<T>* obj, T val) noexcept;
template (pointer) (2)  template <c lass U> U* atomic_fetch_add (volatile atomic<U*>* obj, ptrdiff_t val) noexcept;
                        template <class U> U* atomic_fetch_add (atomic<U*>* obj, ptrdiff_t val) noexcept;
overloads (3)           T atomic_fetch_add (volatile A* obj, M val)     noexcept;
                        T atomic_fetch_add (A* obj, M val) noexcept;

// fetch_add
if T is integral (1)    T fetch_add (T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
                        T fetch_add (T val, memory_order sync = memory_order_seq_cst) noexcept;
if T is pointer (2)     T fetch_add (ptrdiff_t val, memory_order sync = memory_order_seq_cst) volatile noexcept;
                        T fetch_add (ptrdiff_t val, memory_order sync = memory_order_seq_cst) noexcept;
```


. 对 atomic对象

| 功能 | C类型函数 | atomic成员函数 |
| --- | --- | --- |
| 初始化 | atomic_init | atomic 类构造函数 |
| 是否无锁 | atomic_is_lock_free | is_lock_free |
| 获取值 | atomic_load | load |
| 无条件设置值 | atomic_store | store |
| 读并交换(设置值) | atomic_exchange | exchange |
| 满足条件才替换 | atomic_compare_exchange_weak <br> atomic_compare_exchange_strong | compare_exchange_weak <br> compare_exchange_strong |
| 对原值的加、减、并、或、亦或 | atomic_fetch_and<br>atomic_fetch_sub<br>atomic_fetch_and<br>atomic_fetch_or<br>atomic_fetch_xor | fecch_add<br>fetch_sub<br>fetch_and<br>fetch_or<br>fetch_xor<br> operator++ <br> opreator-- |
| 访问元素 | | operator T |
| 操作符(integral或指针) | | operator += -= &= \|= ^= |
| 初始化原子对象的宏 | ATOMIC_VAR_INIT | |

**备注** 除 `atomic_init`(初始化)、`atomic_is_lock_free`(是否 lock-free方式)外，其他的几个函数 atomic_xxx，均有 `atomic_xxx_explicit`形式。


. 对 atomic flags

| 功能 | C类型函数 | atomic_flag 成员函数 |
| --- | --- | --- |
| 原子地将flag设置为true并返回其先前的值 | atomic_flag_test_and_set<br>atomic_flag_test_and_set_explicit | test_and_set |
| 原子地将flag设置成false | atomic_flag_clear<br> atomic_flag_clear_explicit | clear |
| 初始化 std::atomic_flag为 false 的宏 | ATOMIC_FLAG_INIT | |

#### 4.3 memory_order

4.2小节中`fetch_add`函数声明有 `memory_order`的参数的设置，其实在`atomic` 和 `atomic_flag`的主要成员函数中都可以设置这个参数。<br>
为了提高执行速度，现在编译器与cpu做了一些乱序的优化，导致一些执行语句的执行顺序与期望的有多出入，这个时候就需要设置内存执行顺序。

内容来自 [百度brpc的atomic_instructions.md](https://github.com/apache/incubator-brpc/blob/master/docs/cn/atomic_instructions.md).

| memory order | 作用 |
| --- | --- |
| memory_order_relaxed | 没有fencing作用 |
| memory_order_consume | 后面依赖此原子变量的访存指令勿重排至此条指令之前 |
| memory_order_acquire | 后面访存指令勿重排至此条指令之前 |
| memory_order_release | 前面访存指令勿重排至此条指令之后。当此条指令的结果对其他线程可见后，之前的所有指令都可见 |
| memory_order_acq_rel | acquire + release语意 |
| memory_order_seq_cst | acq_rel语意外加所有使用seq_cst的指令有严格地全序关系 |

#### 4.4 宏常量

直接来自 CPP 参考网站。

| macro | relative to types | 
| --- | --- |
| ATOMIC_BOOL_LOCK_FREE | bool |
| ATOMIC_CHAR_LOCK_FREE |char<br>signed char<br>unsigned char |
| ATOMIC_SHORT_LOCK_FREE | short<br>unsigned short |
| ATOMIC_INT_LOCK_FREE | int<br>unsigned int |
| ATOMIC_LONG_LOCK_FREE | long <br> unsigned long|
| ATOMIC_LLONG_LOCK_FREE | long long <br> unsigned long long |
| ATOMIC_WCHAR_T_LOCK_FREE | wchar_t |
| ATOMIC_CHAR16_T_LOCK_FREE | char16_t |
| ATOMIC_CHAR32_T_LOCK_FREE | char32_t |
| ATOMIC_POINTER_LOCK_FREE | U* <br>(for any type U)  |

```
0 if the types are never lock-free.
1 it the types are sometimes lock-free.
2 if the types are always lock-free.

Consistent with the value returned by atomic::is_lock_free.
```

### 5 Future

#### 5.1 基本内容

| 功能 | 名称 | 类型 |
| --- | --- | --- |
| 提供者 | promise<br>packaged_task<br>async| class<br>class<br>function |
| Futures | future <br> shared_future | class<br>class |

除上述5者外，还有几个其他信息

```
future_err		报告与futures或 promises有关的错误 (类) 
future_errc		识别future的错误代码(枚举) 
future_status	用于定时的操作的返回值(枚举) 
launch			指定std::async的启动策略(枚举)
future_category	识别future的错误类别 (函数)
```

提供promise与future使用的一个例子，基本了解在多线程间的使用方式

```
// promise example
#include <iostream>       // std::cout
#include <functional>     // std::ref
#include <thread>         // std::thread
#include <future>         // std::promise, std::future

void print_int (std::future<int>& fut) {
	int x = fut.get();
	std::cout << "value: " << x << '\n';
}

int main ()
{
	std::promise<int> prom;                      // create promise
	std::future<int> fut = prom.get_future();    // engagement with future
	std::thread th1 (print_int, std::ref(fut));  // send future to new thread
	prom.set_value (10);      // fulfill promise
                              // (synchronizes with getting the future)
	th1.join();
	return 0;
}
```

#### 5.2 Providers函数

| 功能 | promise | package_task | async |
| --- | --- | --- | --- |
| 创建 | 构造函数 | 构造函数 | 函数定义 |
| 销毁 | 析构函数 | 析构函数 | |
| 赋值 | operator= | operator= | |
| 获取future | get_future | get_future | 函数执行返回值 | 
| 设置(同步)信息 | set_value<br>set_exception | operator() | - |
| 退出时同步 | set_value_at_thread_exit<br>set_exception_at_thread_exit | make_ready_at_thread_exit | - |
| 交换 | swap(成员/非成员) | swap(成员/非成员) | - |
| 其他 |  | valid <br> reset | | 

**其他**

. async在定义时可自动选择启动策略，也可以自定选择，启动策略包括`launch::async` `launch::deferred`中的一个或两者的并。
. 库提供 `uses_allocator<promise>` 与 `uses_allocator<packaged_task>` 来设定分配器 

#### 5.3 Futures 函数

| 功能 | future | shared_future |
| --- | --- | --- |
| 获取内容 | get | get | 
| 是否与共享状态相关联 | valid | valid | 
| 等待 | wait<br>wait_for<br>wait_until | wait<br>wait_for<br>wait_until |
| 其他 | share | |

#### 5.4 枚举类

| 枚举类 | 枚举值 |
| --- | --- |
| future_errc | broken_promise<br>future_already_retrieved<br>promise_already_satisfied<br>no_state |
| future_status | ready<br>timeout<br>deferred |
| launch | async<br>deferred |


