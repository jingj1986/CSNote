
#ifndef _UNIQUE_H_
#define _UNIQYE_H_

//#include <iostream>
#include <algorithm>

template <class T> 
class unique_ptr {
public:
    explicit unique_ptr(T* ptr = nullptr) noexcept
        :ptr_(ptr) {}

    ~unique_ptr() {
        del();
    }

    unique_ptr(const unique_ptr &) = delete;
    unique_ptr& operator=(const unique_ptr &) = delete;

    unique_ptr(const unique_ptr &&other) noexcept {
        ptr_ = other.release();
    }

    unique_ptr &operator=(unique_ptr &&rhs) {
        if (rhs.ptr_ != this->ptr_ ) {    // 比较*ptr_
            //release();    // 本地应销毁，而不是释放所有权
            del();
            ptr_ =  rhs.release();
        }
        return *this;
    }

    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T* get() const { return ptr_; }

    void reset(T* ptr) noexcept {
        if (ptr_ != ptr) {
            del();
            ptr_ = ptr; 
        }
    }

    void swap(unique_ptr &rhs) noexcept{
        using std::swap;
        swap(ptr_, rhs.ptr_);
    }


    // 释放所有权
    T* release() noexcept {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

private:
    void del() noexcept {
        if (ptr_) {
            delete ptr_;
        }
        ptr_ = nullptr;
    }

private:
    T* ptr_;

};

template<class T>
void swap(unique_ptr<T> &lhs, unique_ptr<T> &rhs) {
    lhs.swap(rhs);
}

#endif
