//
// Created by wanglizhi04 on 2021/11/26.
//
#include <iostream>
#include <vector>
#include <string>

class Object {
public:
    int val;
public:
    Object(int value) : val(value) {
        std::cout << "[C] General constructor." << std::endl;
    }
    // Object(const Object&) = delete;
    // Object(const Object&&) = delete;

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


const std::vector<std::string> get_vec() {
    std::vector<std::string>tmp = {"123", "456"};
    std::cout << "obj_ptr: " << &tmp << std::endl;
    std::cout << "data_ptr: " << tmp.data() << std::endl;
    return tmp;
}

int main() {
    // Object p_obj = get_object();
    const std::vector<std::string>vec = get_vec();
    std::cout << "obj_ptr: " << &vec << std::endl;
    std::cout << "data_ptr: " << vec.data() << std::endl;
    return 0;
}
