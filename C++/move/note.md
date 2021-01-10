## 类型推断

```
template <typename T>
void f(ParamType x);    //ParamType 即为x类型
```

`ParamType` 会有不同的表达形式，具体为

```
template <typename T> 
void f1(T x);    // 形式1

template <typename T>
void f2(T& x);   // 形式2

template <typename T>
void f3(const T& x);   // 形式3

template <typename T>
void f4(T* x);   // 形式4

template <typename T>
void f5(const T* x);   // 形式5

template <typename T>
void f6(T* const x);   // 形式6

template <typename T>
void f7(const T* const x);   // 形式7

template <typename T>
void f8(T&&  x);   // 形式8
```
一般，引用到的参数是固定，不会在更换，所以没有 `T& const`形式；`T&&`为实现完美转发，也没有const等形式。

当传不同参数给 `fx` 时，类型`T`、`ParamType`会得到不同的类型，我们选一些典型类型来看

```
int a;          //为方便，均只指定类型不初始化
const int b;
const int& c;

int* p1;
const int* p2;
int* const p3; 
const int* const p4; 

char s1[2] = "a";
const char s2[2] = "b";
```

其类型推导如下:(说明，表格中上下两项分别代表`T`与`ParamType`的类型)

| | f1 | f2 | f3 | f4 | f5 | f6 | f7 |f8 |
| --- |  --- | --- | --- | --- | --- | --- | --- | --- |
|  | T x | T& x | const T& x| T* x | cont T* x| T* const x | const T* const x | T&& x | 
| int a |int<br>int|int<br>int&|int<br>const int&|int<br>int*|int<br>const int*|int<br>int* const|int<br>const int* const|int&<br>int&|
| const int b |int<br>int|const int<br>const int&|int<br>const int&|const int<br>const int*|int<br>const int*|const int<br>const int* const|int<br>const int* const|const int&<br>const int&|
| int& c |int<br>int|int<br>int&|int<br>const int&|-|-|-|-|int&<br>int&|
| const int& d |int<br>int|const int<br>const int&|int<br>const int&|-|-|-|-|const int&<br>const int&|
| int&& e |int<br>int|int<br>int&|int<br>const int&|-|-|-|-|const int&<br>const int& ?|
| 1 |-|-|-|-|-|-|-| int<br>int&& |
| int* p1 |int* <br>int*|int*<br>int*&|int*<br>int* const&|int<br>int*|int<br>const int*|int<br>int* const|int<br>const int* const|-|
| const int* p2 |const int* <br>const int*|const int*<br>const int* &|const int*<br>const int* const&|const int<br>const int*|int<br>const int*|int<br>const int* const|int<br>const int* const|-|
| int* const p3 |int* <br>int*|int* const<br>int* const&|int*<br>int* const&|int*<br>int*|int<br>const int*|int<br>int* const|int<br>const int* const|-|
| const int* const p4 |const int* <br>const int*|const int* const<br>const int* const&|const int*<br>const int* const&|const int<br>const int*|int<br>const int*|const int<br>const int* const|int<br>const int* const|-|
| char s1[2] |char*<br>char*|char[2]<br>char(&)[2]|char[2]<br>const char(&)[2]|char<br>char*|char*<br>const char*|char<br>char* const| char<br>const char* ?|char(&)[2]<br>char(&)[2]|
| const char s2 [2] |const char*<br>const char*|const char[2]<br>const char(&)[2]|char[2]<br>const char(&)[2]|char<br>char*|char*<br>const char*|char*<br>const char* const|char*<br>const char*|char(&)[2]<br>char(&)[2]|

-`volatile`的声明与`const`的效果相同。

基本思想：

- 我们可以认为是一个函数调用，如果我们想使用cv(const volatile)、引用，那就要的在函数声明定义时，明确的标注出来；如果没有标注，基本认为是不使用这些修饰。<br>
- 函数参数使用了引用，我们认为是保留该参数原有信息，所以cv等修饰，就要保留；而且因为函数声明为引用，所以该参数也应用是应用形式。<br>
- 对数组类型也是类似，如果没有引用类型声明，那数组的一些信息就要退化，编程指针类型；如果声明为引用类型，那就保留数组的信息。
- 如果在函数声明中使用了`cv`的修饰词，在得到ParamType类型后，去掉 `cv`的修饰词，也就得到了 `T`的类型。

auto 的类型推断与模板的类型推断几乎一致，例如：

```
auto x = 1; // int x
const auto cx = x; // const int cx
const auto& rx = x; // const int& rx
auto&& uref1 = x; // int& uref1
auto&& uref2 = cx; // const int& uref2
auto&& uref3 = 1; // int&& uref3
```

### 其他情况

- 模板还支持 参数是函数名的情况，具体如下：

```
template<typename T> void f1(T x);
template<typename T> void f2(T& x);
template<typename T> void f3(T&& x);

void g(int);

f1(g); // T 和 ParamType 都是 void(*)(int)
f2(g); // ParamType 是 void(&)(int)，T 是 void()(int)
f3(g); // T 和 ParamType 都是 void(&)(int)
```

- 对初始化列表`std::initializer_list`的识别与使用上，auto与模板有不同

```
auto x1 = 1;        // int x1
auto x2(1);         // int x2
auto x3 = { 1  };   // std:::initializer_list<int> x3
auto x4 = { 1,2 };  // std:::initializer_list<int> x4, C++14 中必须用 =，否则报错
auto x5{ 1  };      // C++11 为 std::initializer_list<int> x5，C++14 为 int x5
auto x6{ 1,2 };     // C++11 为 std::initializer_list<int> x6, C++14报错
auto x7 = { 1, 2, 3.0  }; // 错误：不能为 std::initializer_list<T> 推断 T

// 模板 
template<typename T> // 等价于 x 声明的模板
void f1(T x);

f1({ 1, 2, 3  });   // 错误：不能推断 T 的类型

template<typename T>
void f2(std::initializer_list<T> initList);

f2({ 1, 2, 3 });    // OK, T 被推断为 int，initList 类型是 std::initializer_list<int>
```

- 补充

```
// C++ 14, auto 可以作为函数返回类型
auto f() { return 1;  }
auto g = [](auto x) { return x;  }; 
// auto 仍然使用的是模板实参推断的机制，因此不能为 auto 返回类型返回一个初始化列表
// auto newInitList() { return { 1  };  } // 错误

// C++17 auto 可以作为非类型模板参数
template<auto N>
struct X {
  void f() { std::cout << N;  }  
};

X<1> x;
x.f(); // 1
```

-- 
TODO char s[] 用 const T* const 推断
int& 用 T&& 推断，会成为const么? int &&呢

