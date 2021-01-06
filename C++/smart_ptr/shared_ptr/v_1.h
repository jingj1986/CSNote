#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

#include <algorithm>

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
