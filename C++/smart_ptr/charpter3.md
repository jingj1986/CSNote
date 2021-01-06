** 智能指针 **


## 实现

本小节按照这样思路进行，先实现智能指针的基本简单功能，然后逐渐完善和优化。
有时候觉得，当知道代码要实现什么功能之后，代码比文字有更好的表达力，所以通过几个不同版本来说明设计，配上必要文字说明改进的思路。

### shared_ptr

#### 设计1

`shared_ptr`最基本思路，提供一个额外的引用计数器，使得多个指针可动态的共享所指向的对象，通过计数器的加减，记录当前对象的引用次数，当引用数为0时，释放该对象在堆上的内存。

延续`unique_ptr` 的套路，我们先看看shared_ptr中应提供的基本接口，并在设计时，加上必要的 `explicit`、`const`、`noexcept`等限制，作为对外的接口。

```
template <class T>
class sm_ptr {
public:
    explicit sm_ptr(T* ptr);  // why not noexcept ?
    sm_ptr(const sm_ptr& other) noexcept;
    sm_ptr(sm_ptr&& other) noexcept;
    sm_ptr& operator=(const sm_ptr& other) noexcept;
    sm_ptr& operator=(sm_ptr&& other) noexcept;
    
    ~sm_ptr();

    T& operator*() const noexcept;
    T* operator->() const noexcept;
    T* get() const noexcept;
    explicit operator bool() const noexcept;

    void swap(sm_ptr& other) noexcept;

    unsigned int use_count() const noexcept;


private:
    T* ptr_;
    unsigned int* count_;
};
```

为什么使用指针的构造函数不设置为 `noexcept`呢？因为在构造时，需要创建一个计数器对象，而这个是不能保证noexcept的；
当已经有计数器之后，在进行拷贝构造、拷贝赋值时，没有new相关的操作，我们就可以保证是noexcept的。

设计中暂时没有reset接口。

#### 实现1

```
#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

#include <algorithm> // for std::swap

template <class T>
class sm_ptr {
public:
    explicit sm_ptr(T* ptr)
        : ptr_(ptr) {
        count_ = new unsigned int();
        *count_ = 1;
    }

    sm_ptr(const sm_ptr& p) noexcept 
        : ptr_(p.ptr_)
        , count_(p.count_) {
        increase();
    }

    sm_ptr(sm_ptr&& p) noexcept {
        swap(p);
    }

    sm_ptr& operator=(const sm_ptr& rp) noexcept {
        // 先自增参数的引用计数是为了防止参数就是自身的情况
        rp.increase();
        // 自身执行删减
        decrease();

        ptr_ = rp.ptr_;
        count_ = rp.count_;
        return *this;
    }

    sm_ptr& operator=(const sm_ptr&& p) noexcept {
        swap(p);
        return *this;
    }
    
    ~sm_ptr() noexcept {
        decrease();
    }

    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }
    T* get() const noexcept { return ptr_; }
    explicit operator bool() const noexcept { return ptr_; }

    void swap(sm_ptr& p) noexcept {
        using std::swap;
        swap(ptr_, p.ptr);
        swap(count_, p.count_);
    }

    unsigned int use_count() const noexcept {
        return count_ ? *count_ : 0;
    }

private:

    void increase() {
        if (count_) {
            ++(*count_);
        }
    }

    void decrease() {
        if (count_ && (--(*count_)) == 0) {
            if (ptr_) {
                delete ptr_;
            }
            ptr_ = nullptr;

            delete count_;
            count_ = nullptr;
        }
    }

private:
    T* ptr_;
    unsigned int* count_;    // or long*
};

#endif
```

#### 讨论

上述方案存在几个问题：

a. 引用计数的递增和递减应该是原子操作

b. 通过裸指针构造shared_ptr时，没有对裸指针显示，如果对同一个指针，构造多个shared_ptr，就会出现问题
```
    class A a = new A();
    shared_ptr<A> p_a1(a);
    shared_ptr<A> p_a2(a);  
    //问题： 一个对象会被两个shared_ptr释放
```

除了前面提到的reset接口外，上述方案还缺少shared_ptr的几个功能：

1. 使用一个控制块(包括引用计数，弱计数，其他数据如自定义删除器、分配器等)
2. 通过unique_ptr构造，新建控制块
3. 通过shared_ptr,weak_ptr构造，不新建控制块
4. make_shared等函数

所以我们看编译器(G++/MicroSoft-STL)中，都是先定义一个`shared_ptr`与`weak_ptr`的基类。
**TODO**: 如何解决上面问题。

#### 补充

设计一个计数器类，
ptr的基础类 - R3

使用的计数模式可替换

我们一开始的思路是采用计数器来方式实现对象的共享，在 R7-x智能指针一节，对实现对象共享，提供了更多的思路，包括：

- 引用者以双向链表形式存放，当增加共享者时，在链表中增加；当析构时，在链表中删除，并判断是否为最后一个，若是则删除共享对象。
-

## 参考
1. R3: more effective C++ 
2. R7: Modern C++ Design

