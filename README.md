# jston - C++ 结构体与 JSON 转换框架

## 项目概述

这是一个优化后的 C++ 结构体与 JSON 转换框架，相比早期版本的 `struct_translator`，新框架提供了更加简洁易用的接口，用户只需定义结构体，无需编写额外的配置代码就能实现结构体与 JSON 之间的自动转换。

## 平台支持
- **Linux**: 已在 Ubuntu 20.04 上测试通过

## 主要特点

- **零配置**: 用户创建结构体后无需编写额外的配置代码
- **自动注册**: 框架自动注册结构体元数据和转换函数
- **支持嵌套结构体**: 自动处理多层嵌套的结构体
- **支持数组**: 支持基本类型数组和结构体数组
- **支持多种数据类型**: 支持基本数据类型、C风格字符串、函数指针等
- **简洁的 API**: 提供直观的 `to_json` 和 `from_json` 函数

## 框架结构

- **inc/jston.h**: 框架的核心头文件，包含所有必要的类、函数和宏定义
- **test/test_basic.cpp**: 基本功能测试程序
- **test/test_advanced.cpp**: 高级功能测试程序

## 使用方法

### 1. 定义结构体

使用标准的 C++ 结构体定义语法，然后使用 `register_json_struct` 宏注册结构体，这会自动注册结构体的元数据：

```cpp
#include "jston.h"

// 定义简单结构体
struct Person {
    int age;
    char name[32];
    double salary;
};
register_json_struct(Person, age, name, salary);

// 定义嵌套结构体
struct Company {
    char name[32];
    Person CEO;
    int employee_count;
};
register_json_struct(Company, name, CEO, employee_count);

// 定义包含数组的结构体
struct Department {
    char name[32];
    Person employees[20];
    int count;
};
register_json_struct(Department, name, employees, count);
```

### 2. 结构体转 JSON

使用 `jston::to_json` 函数将结构体转换为 JSON 对象：

```cpp
Person person;
person.age = 30;
strcpy(person.name, "John Doe");
person.salary = 50000.5;

// 结构体转 JSON
nlohmann::json json = jston::to_json(person);
std::cout << json.dump(4) << std::endl;
```

输出结果：

```json
{
    "age": 30,
    "name": "John Doe",
    "salary": 50000.5
}
```

### 3. JSON 转结构体

使用 `jston::from_json` 函数将 JSON 对象转换为结构体：

```cpp
// 从字符串解析 JSON
std::string json_str = R"({
    "age": 25,
    "name": "Jane Smith",
    "salary": 45000.75
})";
nlohmann::json json = nlohmann::json::parse(json_str);

// JSON 转结构体
Person person;
jston::from_json(json, person);

// 使用转换后的结构体
std::cout << "Name: " << person.name << ", Age: " << person.age << std::endl;
```

### 4. 处理嵌套结构体和数组

框架会自动处理嵌套结构体和数组的转换：

```cpp
// 创建包含嵌套结构体和数组的复杂对象
Company company;
strcpy(company.name, "Tech Corp");
company.CEO.age = 45;
strcpy(company.CEO.name, "Alice Johnson");
company.CEO.salary = 150000.0;
company.employee_count = 100;

// 转换为 JSON
nlohmann::json company_json = jston::to_json(company);
std::cout << company_json.dump(4) << std::endl;
```

## 构建示例程序

### 前提条件

- CMake 3.10 或更高版本
- nlohmann_json 库

### 构建步骤

1. 确保已安装 nlohmann_json 库

2. 在项目目录中创建 build 目录并进入：

```bash
mkdir -p build && cd build
```

3. 运行 CMake 和构建命令：

```bash
cmake ..
make
```

4. 运行测试程序：

```bash
./test_basic
./test_advanced
```

## 支持的数据类型

- **基本类型**: char, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long, float, double, bool
- **字符串**: C风格字符数组 (char[])
- **结构体**: 支持嵌套结构体
- **数组**: 基本类型数组和结构体数组
- **函数指针**: 会被标记为 `"[function_pointer]"`，但不会实际序列化

## 注意事项

- 函数指针不会被实际序列化，只会在 JSON 中标记为 `"[function_pointer]"`
- 框架只支持 C 风格字符数组 (char[])，不直接支持 std::string
- 对于字符数组，框架会确保正确处理字符串结束符
- 嵌套结构体和数组的处理是自动的，无需手动配置
- 确保结构体的定义在使用前完成，以便正确注册元数据
- 框架支持最多 30 个字段的结构体

## 与原有框架的对比

| 特性 | 原有框架 | 新框架 |
|------|---------|-------|
| 配置需求 | 需要手动编写回调函数 | 零配置，自动注册 |
| API 简洁度 | 多个宏和函数 | 简单的 `jston::to_json`/`jston::from_json` 函数 |
| 类型安全 | 部分类型安全 | 更高级的类型安全 |
| 扩展性 | 有限 | 更好的扩展性 |
| 易用性 | 中等 | 高 |

## License

Apache 2.0