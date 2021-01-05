#ifndef _UNIQUE_PTR_H_V2_
#define _UNIQUE_PTR_H_V2_

#include <memory>   // default_delete
#include <type_traits>


template <class T, class D = std::default_delete<T>>
class unique_ptr {

public:
    using element_type = T;
    using deleter_type = D;
    using pointer = T*;

	constexpr unique_ptr() noexcept;
    constexpr unique_ptr(std::nullptr_t) noexcept : unique_ptr() {}
    explicit unique_ptr(pointer p) noexcept;
    unique_ptr(pointer p, typename std::conditional<std::is_reference<D>::value,D,const D&> del) noexcept;
    unique_ptr(pointer p, typename std::remove_reference<D>::type&& del) noexcept;
	unique_ptr(const unique_ptr&& ) noexcept;
    template<class U, class E> unique_ptr(unique_ptr<U,E>&& x) noexcept;
    template<class U> unique_ptr(unique_ptr<U>&& x) noexcept;
	unique_ptr(const unique_ptr&) = delete;

	~unique_ptr() ; 				

	unique_ptr& operator=(const unique_ptr&) = delete;
	unique_ptr& operator=(unique_ptr&& x) noexcept;
    unique_ptr& operator=(std::nullptr_t) noexcept;
    template<class U, class E> unique_ptr& operator=(unique_ptr<U,E>&& x) noexcept;

	//T& operator*() const;
    typename std::add_lvalue_reference<element_type>::type operator*() const;
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
template <class T, class D> bool operator== (const unique_ptr<T,D>& lhs, std::nullptr_t) noexcept;
template <class T, class D> bool operator== (std::nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator!= (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator!= (const unique_ptr<T,D>& lhs, std::nullptr_t) noexcept;
template <class T, class D> bool operator!= (std::nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator<  (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator<  (const unique_ptr<T,D>& lhs, std::nullptr_t) noexcept;
template <class T, class D> bool operator<  (std::nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator<= (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator<= (const unique_ptr<T,D>& lhs, std::nullptr_t) noexcept;
template <class T, class D> bool operator<= (std::nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator>  (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator>  (const unique_ptr<T,D>& lhs, std::nullptr_t) noexcept;
template <class T, class D> bool operator>  (std::nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

template <class T1, class D1, class T2, class D2>
  bool operator>= (const unique_ptr<T1,D1>& lhs, const unique_ptr<T2,D2>& rhs);
template <class T, class D> bool operator>= (const unique_ptr<T,D>& lhs, std::nullptr_t) noexcept;
template <class T, class D> bool operator>= (std::nullptr_t, const unique_ptr<T,D>& rhs) noexcept;

#endif
