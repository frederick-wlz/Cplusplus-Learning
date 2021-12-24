# RVO & NRVO & Copy-Elision
现代编译器缺省会使用`RVO`（return value optimization，返回值优化）、`NRVO`（named return value optimization、命名返回值优化）和`复制省略`（Copy elision）技术，来减少拷贝次数来提升代码的运行效率

示例代码片段如下：
```cpp
class Object {
public:
    int val;
public:
    Object(int value) : val(value) {
        std::cout << "[C] General constructor." << std::endl;
    }

    Object(const Object& obj) {
        val = obj.val;
        std::cout  << "[C] Copying constructor." << std::endl;
    }

    Object& operator=(const Object& obj) {
        if (this != &obj) {
            val = obj.val;
        }
        std::cout << "[C] = constructor." << std::endl;
        return *this;
    }

    ~Object() {
        std::cout << "[D] Destructor." << std::endl;
    }
};

Object get_object() {
    Object obj(1);
    return obj;
}

int main() {
    Object p_obj = get_object();
    return 0;
}

```

## 不做任何返回值优化
在g++中有一个编译选项 `-fno-elide-constructors` 可以去掉任何返回值优化，运行后打印如下：
```cpp
[C] General constructor.    // get_object()函数构造obj对象
[C] Copying constructor.    // get_object()函数使用obj对象拷贝构造临时对象_temp
[D] Destructor.             // get_object()函数返回，obj对象析构
[C] Copying constructor.    // main函数使用_temp对象拷贝构造p_obj对象
[D] Destructor.             // _temp对象析构  
[D] Destructor.             // p_obj对象析构
```

## RVO 
RVO(return value optimization)，返回值优化，是编译器的一项优化技术
VS在debug模式使用RVO，运行后打印如下：
```cpp
[C] General constructor.    // get_object()函数构造obj对象
[C] Copying constructor.    // 
[D] Destructor.             // obj对象析构  
[D] Destructor.             // p_obj对象析构
```
可以看出少了一次拷贝构造和析构函数，是因为编译器将get_object函数改写成了如下形式：
```cpp
void get_object(const Object& _result) {
    Object obj;
    obj.Object::Object(1);
    _result.Object::Object(obj);
    obj.Object::~Object();
    return;
}
```
在main函数中改写为：
```cpp
Object p_obj;
get_object(p_obj);
```
这样将p_obj作为参数，在get_obj函数中进行构造，就避免了产生临时对象，省去了一次构造和析构

## NRVO
NRVO（named return value optimization、命名返回值优化），相比于RVO，NRVO进一步优化了返回值

如上例所示，RVO优化避免了临时对象的产生，但仍产生了局部对象obj，那能不能直接产生p_obj对象呢，NRVO实现了这样的优化

g++默认支持NRVO，打印信息如下：
```cpp
[C] General constructor.    // get_object()函数构造p_obj对象
[D] Destructor.             // p_obj对象析构
```
可以看出少了两次拷贝构造和析构函数，是因为编译器将get_object函数改写成了如下形式：
```cpp
void get_object(const Object& _result) {
    _result.Object::Object(1);
    return;
}
```
在main函数中改写为：
```cpp
Object p_obj;
get_object(p_obj);
```

## Copy-Elision技术
C++11以后，g++编译器默认开启复制省略（copy elision）选项，可以在以值语义传递对象时避免触发复制、移动构造函数。copy elision 主要发生在两个场景：
- 函数返回的是值语义时
- 函数参数是值语义时

在示例中添加移动构造函数，代码如下：
```cpp
class Object {
public:
    int val;
public:
    Object(int value) : val(value) {
        std::cout << "[C] General constructor." << std::endl;
    }

    Object(const Object& obj) {
        val = obj.val;
        std::cout  << "[C] Copying constructor." << std::endl;
    }
    
    Object(Object&& obj) {
        std::cout << "[C] Moving constructor." << std::endl;
    }

    Object& operator=(const Object& obj) {
        if (this != &obj) {
            val = obj.val;
        }
        std::cout << "[C] = constructor." << std::endl;
        return *this;
    }

    ~Object() {
        std::cout << "[D] Destructor." << std::endl;
    }
};

Object get_object() {
    Object obj(1);
    return obj;
}

int main() {
    Object p_obj = get_object();
    return 0;
}
```
同时关闭返回值优化，打印信息如下：
```c++
[C] General constructor.
[C] Moving constructor.
[D] Destructor.
[C] Moving constructor.
[D] Destructor.
[D] Destructor.
```
可以看到拷贝构造变成了移动构造函数
这不能完全体现出copy-elision的作用，换一个示例，代码如下：
```c++
std::vector<std::string> get_vec() {
    std::vector<std::string>tmp = {"123", "456"};
    std::cout << "obj_ptr: " << &tmp << std::endl;
    std::cout << "data_ptr: " << tmp.data() << std::endl;
    return tmp;
}

int main() {
    std::vector<std::string>vec = get_vec();
    std::cout << "obj_ptr: " << &vec << std::endl;
    std::cout << "data_ptr: " << vec.data() << std::endl;
    return 0;
}
```
关闭返回值优化，打印信息如下：
```c++
obj_ptr: 0x7ffee88227e0
data_ptr: 0x7f9d454059e0
obj_ptr: 0x7ffee8822880
data_ptr: 0x7f9d454059e0
```
可以看出数据指针地址相同，说明移动构造函数只进行了浅拷贝，将对象指针改变了，并没有移动数据

## 结论
RVO、NRVO在C++17前是非标的，由编译器自行实现(g++8支持NRVO），在C++17后变成了标准，编译器必须实现，移动构造函数是C++11后出现，在无返回值优化的情况下，将拷贝构造函数替换成移动构造函数，
C++17同时对Copy-Elision进一步支持，在C++17前虽然NRVO消除了拷贝构造，但仍然要求有拷贝构造函数，在C++17后，无拷贝构造函数也可以实现Copy-Elision

