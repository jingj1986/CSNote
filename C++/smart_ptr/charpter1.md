** 智能指针 **

 使用C++，肯定是需要了解智能指针了，在C++11之后，推荐使用的有 `unique_ptr`、`share_ptr`、`weak_ptr` 三类；在11之前，采用的是`auto_ptr`，该类型现在不推荐使用（原因后续说明）。
 本文先列出智能指针的函数，然后说明一些应用场景，最后尝试实现简易版本的智能指针。

## 函数

参考cpp 手册，列出这几个指针的函数

|  | unique_ptr | share_ptr | weak_ptr |
| --- | --- | --- | --- |
| Member types | - | - | - |
| element_type | Y | Y | Y |
| deleter_type | Y | | |
| pointer | Y  | | |
| Member Functions | - | - | - |
| (constructor) | Y | Y | Y |
| (destructor) | Y | Y | Y |
| operator= | Y | Y | Y |
| swap | Y | Y | Y |
| reset | Y | Y | Y |
| get | Y | Y |  |
| operator* | Y- | Y | |
| operator-> | Y- | Y | |
| use_count |  | Y | Y |
| unique | | Y | |
| release | Y | | |
| expired | | | Y |
| lock | | | Y |
| operator bool | Y | Y | |
| operator[] | Y | | |
| owner_before | | Y | Y |
| Non Member functions | - | - | - |
| swap | Y | Y | Y |
| ==, !=, <, <=, >, >= | Y | Y | |
| ostream operator<< | | Y | |

`share_ptr` 还有几个特殊函数:
```
make_shared
allocate_shared
static_pointer_cast
dynamic_pointer_cast
const_pointer_cast
get_deleter
```

## 使用

### auto_ptr

auto_ptr 是C++最早的智能指针，在此之前的一系列C++经典书籍中，有对该类使用的广泛讨论，但是存在一定的问题，比如

```
auto_ptr<string> strs[4] =
 {
  auto_ptr<string> (new string("hello")),
  auto_ptr<string> (new string("world")),
  auto_ptr<string> (new string("C++")),
  auto_ptr<string> (new string("java")),
 };
 auto_ptr<string> p_str;
 p_str = strs[2]; // strs[2] loses ownership. 将所有权从strs[2]转让给p_str，此时strs[2]不再引用该字符串从而变成空指针
 for (int i = 0; i < 4; i++) {
	cout << strs[i] << endl;
 }
```

后面在使用 films[2] 的时候，已经是空指针了，程序在执行时就会崩溃。
- 换成 `unique_ptr` ,编译时会出错；
- 换成 `shared_ptr`则运行正常，因为p_str 与 strs[2] 指向同一块内存。[1]

总结来说 <br>
. `auto_ptr` 与 `unique_ptr` 对内存地址都是独占的，而 `unique_ptr`的策略更加严格、更加安全; <br>
.  `auto_ptr`的拷贝构造函数(constructor)与拷贝赋值函数(operator=)仅采用copy语音转义指针资源，将原指针设置为NULL；而unique_ptr支持转移构造函数，采用move语义形式

### 示例

在早起C++经典书籍中对智能指针的使用与讨论主要使用 `auto_ptr`，这里我们换用 `unique_ptr` 或 `shared_ptr` 

####  1. 类设计 

```

```

#### 2. 智能指针包含在多语句内

```
```

#### 3. 容器内使用
```
```

#### 4. 简单工厂模式

R4-18
```
//BaseClass

template<typename... Ts>
unique_ptr<BaseClass> makeFact(Ts&&... param);

auto pFact = makeFact(arguments);
```
unique还可以设置对象的删除函数，在C++11中
```
auto delFac = []() {} // lambda 表达式
template<typename... Ts>
unique_ptr<BaseClass, decltype(delFac)>
makeFact(Ts&&... params) { ... } 
```
而C++14以后，可以用auto来设置函数的返回值

```
template<typename... Ts>
auto makeFact(Ts&&... params) {
    auto delFac = []() {} // lambda 表达式
    unique_ptr<BaseClass, decltype(delFac)> pFact(nullptr, delFac);
    if () ... //
    return pFact;
}
```

#### 5. 不同

unique_ptr 与 shared_ptr 定义不同
```
template <class T, class D = default_delete<T>> class unique_ptr;
template <class T> class shared_ptr;
```

这样shared_ptr具有更大灵活性,

```
shared_ptr<A> pw1(new A, del1);
shared_ptr<A> pw2(new A, del2);  //使用不同的析构器

vector<shared_ptr<A>> vpw{pw1, pw2};
```
这样定义的不同，在Pimpl习惯用法上就有明显的差异。

**Pimpl习惯用法**

使用Pimpl习惯用法时，如果pImpl为unique_ptr形式，那么就需要在Impl的定义之后明确析构。内容来自 R4-22.

```
// A.h
class A {
 public:
    A();
 private:
    struct X;	
    std::unique_ptr<X> x;   // unique_ptr方式的pImpl
};

// A.cpp
#include "A.h"
struct A::X {
  int i;
  std::string s;
  std::vector<double> v;
};

A::A() : x(std::make_unique<X>()) {}

// main.cpp
#include "A.h"
int main()
{
  A a; // 错误：A::X 是不完整类型
}
```

原因是在`A a;` 语句处，编译器会在此处创建隐式的析构函数(inline)的，析构函数执行unique_ptr的析构器，采用的是default_delete，在 delete 语句之前会用 static_assert 断言指针指向的不是非完整类型；而此时没有看到A::X的定义，所以其为非完整类型，断言失败，出现错误。

```
// 删除器的实现
template<class T>
struct default_delete // default deleter for unique_ptr
{
  constexpr default_delete() noexcept = default;
  
  template<class U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
  default_delete(const default_delete<U>&) noexcept
  { // construct from another default_delete
  }

  void operator()(T* p) const noexcept
  {
    static_assert(0 < sizeof(T), "can't delete an incomplete type");	// 断言
    delete p;
  }
};
```

解决方法就是让析构 std::unique_ptr 的代码看见完整类型，即让析构函数的定义位于要析构的类型的定义之后

```
// A.h
class A {
 public:
 	A();
 	~A();
 private:
 	struct X;
 	std::unique_ptr<X> x;
};

// A.cpp
#include "A.h"
struct A::X {
  int i;
  std::string s;
  std::vector<double> v;
};

A::A() : x(std::make_unique<X>()) {}
A::~A() = default; // 必须位于 A::X 的定义之后
```
有了析构函数后，若要支持移动构造也许显示声明；在移动构造时会有原对象的析构，所以也需要将A::X的定义之后，显式定义移动构造函数、移动赋值函数。

而在Pimpl中使用 shared_ptr 则不会有该问题，这就是`unique_ptr` 与 `shared_ptr`定义的不同。但是Pimpl中一般是用到 unique_ptr的类型。
更多详细的描述与讨论见 R4-22章节。


## 参考
1. [C++智能指针简单剖析](https://www.cnblogs.com/lanxuezaipiao/p/4132096.html)
2. R2: effective C++
3. R3: more effective C++ 
4. R4: modern effective C++
5. R5: Exceptional C++
6. R6: More Exceptional C++


