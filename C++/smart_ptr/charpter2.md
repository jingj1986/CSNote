** 智能指针 **


## 实现

本小节按照这样思路进行，先实现智能指针的基本简单功能，然后逐渐完善和优化。
有时候觉得，当知道代码要实现什么功能之后，代码比文字有更好的表达力，所以通过几个不同版本来说明设计，配上必要文字说明改进的思路。

### unique_ptr

. 设计1

先列出unique_ptr 应该包含的基本的函数与成员。

```
template <class T>
class unique_ptr {

public:
	unique_ptr(T* ptr = nullptr) ;	
	~unique_ptr() ; 				

	unique_ptr(const unique_ptr&) = delete;
	unique_ptr& operator=(const unique_ptr&) = delete;

	unique_ptr(const unique_ptr&& );
	unique_ptr& operator=(unique_ptr&& );

	T& operator*();
	T* operator->();
	T* get();
	T* release() ;
	
	void reset(T* ptr);
	void swap(unique_ptr& rhs);

	bool operator bool();
	
private:
	T* ptr_;	// 指针, 不要定义为 _ptr 形式

};

```
不包含拷贝构造函数与拷贝赋值函数（采用delete而不是private，在继承xx等居中好处，具体见R4-x）；
为何才实现这两个函数，采用完美转发的移动构造函数与移动赋值函数可实现该功能，且能更好的适应C++11的转义语义。

. 实现1

在实现时，我们需要加上一些控制，如函数的`explicit` 、`noexcept` 、`const`等

	. 我们不希望指针可以静默转为unique_ptr，采用explicit 限制 (Rxx);
	. 	析构函数是不允许抛出异常的(RXX)，相反，构造函数没有这个显示；但是这里我们要实现的是智能指针，就是要避免资源泄露，保证在出现异常情况下仍然可以释放持有的资源，所以构造函数\赋值函数也必须是不抛异常。关于构造函数参考(RXX)
	. 同上，release 资源释放函数, reset 重置使用的资源，swap交换资源，也不应该抛出异常
    . R4-14, 只要函数不会发射异常，就为其加上noexcept声明
	. 成员函数可以设置为const的，优先考虑设置为const

R2-30 指出使用inline会有一系列的影响，比如代码展开造成代码膨胀等；而如果将成员函数实现都放在类的定义中，编译器默认会构造成inline函数。而我们也知道inline的好处，此处的每个函数的代码量都比较小，所以我们也就允许其展开为inline。

```
#ifndef _UNIQUE_H_
#define _UNIQUE_H_

#include <algorithm>	// for using std::swap

template <class T>
class unique_ptr {

public:
	explicit unique_ptr(T* ptr = nullptr) noexcept 
		: ptr_(ptr)	 {}

	~unique_ptr() {
		del();			// 增加成员函数，其他成员函数也会调用
	}	

	unique_ptr(const unique_ptr&) = delete;
	unique_ptr& operator=(const unique_ptr&) = delete;

	unique_ptr(const unique_ptr&& rhs) noexcept {	
		ptr_ = other.release();		// 
	}

	unique_ptr& operator=(unique_ptr&& rhs) noexcept {
		if (rhs.ptr_ != this->ptr_) {
		    del();
		    ptr_ = rhs.release();
        }
		return *this;
	}

	T& operator*() const {return *ptr_;}
	T* operator->() const {return ptr_;}
	T* get() const {return ptr_;}

	T* release() noexcept {
		T* ptr = ptr_;
		ptr_ = nullptr;
		return ptr;
	}
	
	void reset(T* ptr) noexcept {
        if (ptr_ != ptr) {
	    	del();
		    ptr_ = ptr;
        }
   	}

	void swap(unique_ptr& rhs) noexcept {
		using std::swap;	// 显式声明，RXX
		swap(ptr_, rhs.ptr_);
	}

	explicit operator bool() {
		return ptr_ != nullptr;
	}

private:
    void del() noexcept{
        if (ptr_) {
            delete ptr_;
        }
        ptr_ = nullptr;
    }	

private:
	T* ptr_;	// 指针, 不要定义为 _ptr 形式
};

#endif
```

. 设计2 

R4-15 只要有可能使用constexpr，就使用它
增加对析构时删除函数的支持，
增加 ==, !=, <, <=, >, >=
参考C++手册上 定义的接口形式

```
template <class T, class D = default_delete<T>>
class unique_ptr {

public:
    using element_type = T;
    using deleter_type = D;
    using pointer = T*;

	constexpr unique_ptr() noexcept;
    constexpr unique_ptr(nullptr_t) noexcept : unique_ptr {}
    explicit unique_ptr(pointer p) noexcept;
    unique_ptr(pointer p, typename conditional<is_reference<D>::value,D,const D&> del) noexcept;
    unique_ptr(pointer p, typename remove_reference<D>::type&& del) noexcept;
	unique_ptr(const unique_ptr&& ) noexcept;
    template<class U, class E> unique_ptr(unique_ptr<U,E>&& x) noexcept;
    template<class U> unique_ptr(unique_ptr<U>&& x) noexcept;
	unique_ptr(const unique_ptr&) = delete;

	~unique_ptr() ; 				

	unique_ptr& operator=(const unique_ptr&) = delete;
	unique_ptr& operator=(unique_ptr&& x) noexcept;
    unique_ptr& operator=(nullptr_t) noexcept;
    template<class U, class E> unique_ptr& operator=(unique_ptr<U,E>&& x) noexcept;

	//T& operator*() const;
    typename add_lvalue_reference<element_type>::type operator*() const;
	pointer operator->() const noexcept;
	pointer get() const noexcept;
	pointer release() noexcept;
	
	void reset(T* ptr) noexcept;
	void swap(unique_ptr& rhs) noexcept;

    explicit operator bool() const noexcept;
	
private:
	pointer ptr_;	// 指针, 不要定义为 _ptr 形式
    deleter_type delFun;    //如函数指针、函数对象、lambda表达式
};

template <class T1, class D1, class T2, class D2>
  bool operator== (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator== (const unique_ptr<T,D>& lhs, nullptr_t) noexcept;
template <class T, class D> bool operator== (nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator!= (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator!= (const unique_ptr<T,D>& lhs, nullptr_t) noexcept;
template <class T, class D> bool operator!= (nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator<  (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator<  (const unique_ptr<T,D>& lhs, nullptr_t) noexcept;
template <class T, class D> bool operator<  (nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator<= (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator<= (const unique_ptr<T,D>& lhs, nullptr_t) noexcept;
template <class T, class D> bool operator<= (nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator>  (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator>  (const unique_ptr<T,D>& lhs, nullptr_t) noexcept;
template <class T, class D> bool operator>  (nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator>= (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator>= (const unique_ptr<T,D>& lhs, nullptr_t) noexcept;
template <class T, class D> bool operator>= (nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

```

设置析构器的影响，若析构器是函数指针，尺寸一般会增加一到两个字长；若是函数对象，则尺寸变化取决于函数对象中农存储了多少状态。无状态的函数对象（如lambda表达式）不会增加尺寸。-R4-18 <br>
我们可以看到设计1与设计2存在一定的差别，在Rxx和R5-x有关于类设计的讨论。要知道一旦向外提供了api，就要保持其稳定，所以在设计上需要多花些时间。

unique_ptr 支持通过 auto_ptr的构造函数，本接口未提供。

. 实现

基于上面的设计，几乎就会STL中的unique_ptr的版本，这里提供GCC与MicroSoft的unique_ptr的源文件，感兴趣可以查看

c++_unique_ptr.h : 来自 /usr/include/c++/9/bits/unique_ptr.h
Ms-memory.h: [MicroSoft-STL](https://github.com/github-sr/STL)

## 参考
1. R4: modern effective C++
