# jston - C++ Struct to JSON Conversion Framework

## Project Overview

This is an optimized C++ framework for converting between structs and JSON. Compared to the earlier version `struct_translator`, the new framework provides a more concise and user-friendly interface. Users only need to define their structs without writing additional configuration code to achieve automatic conversion between structs and JSON.

## platform
- **Linux**: Tested on Ubuntu 20.04

## Main Features

- **Zero Configuration**: No additional configuration code needed after creating structs
- **Automatic Registration**: The framework automatically registers struct metadata and conversion functions
- **Nested Struct Support**: Automatically handles multi-level nested structs
- **Array Support**: Supports arrays of basic types and structs
- **Multiple Data Types**: Supports basic data types, C-style strings, function pointers, etc.
- **Clean API**: Provides intuitive `to_json` and `from_json` functions

## Framework Structure

- **inc/jston.h**: Core header file of the framework, containing all necessary classes, functions, and macro definitions
- **test/test_basic.cpp**: Basic function test program
- **test/test_advanced.cpp**: Advanced function test program

## Usage

### 1. Define Structs

Use standard C++ struct definition syntax, then register the struct using the `register_json_struct` macro, which will automatically register the struct's metadata:

```cpp
#include "jston.h"

// Define a simple struct
struct Person {
    int age;
    char name[32];
    double salary;
};
register_json_struct(Person, age, name, salary);

// Define a nested struct
struct Company {
    char name[32];
    Person CEO;
    int employee_count;
};
register_json_struct(Company, name, CEO, employee_count);

// Define a struct containing arrays
struct Department {
    char name[32];
    Person employees[20];
    int count;
};
register_json_struct(Department, name, employees, count);
```

### 2. Convert Struct to JSON

Use the `jston::to_json` function to convert a struct to a JSON object:

```cpp
Person person;
person.age = 30;
strcpy(person.name, "John Doe");
person.salary = 50000.5;

// Convert struct to JSON
nlohmann::json json = jston::to_json(person);
std::cout << json.dump(4) << std::endl;
```

Output result:

```json
{
    "age": 30,
    "name": "John Doe",
    "salary": 50000.5
}
```

### 3. Convert JSON to Struct

Use the `jston::from_json` function to convert a JSON object to a struct:

```cpp
// Parse JSON from string
std::string json_str = R"({
    "age": 25,
    "name": "Jane Smith",
    "salary": 45000.75
})";
nlohmann::json json = nlohmann::json::parse(json_str);

// Convert JSON to struct
Person person;
jston::from_json(json, person);

// Use the converted struct
std::cout << "Name: " << person.name << ", Age: " << person.age << std::endl;
```

### 4. Handling Nested Structs and Arrays

The framework automatically handles conversion of nested structs and arrays:

```cpp
// Create a complex object containing nested structs and arrays
Company company;
strcpy(company.name, "Tech Corp");
company.CEO.age = 45;
strcpy(company.CEO.name, "Alice Johnson");
company.CEO.salary = 150000.0;
company.employee_count = 100;

// Convert to JSON
nlohmann::json company_json = jston::to_json(company);
std::cout << company_json.dump(4) << std::endl;
```

## Building the Example Programs

### Prerequisites

- CMake 3.10 or higher
- nlohmann_json library

### Building Steps

1. Ensure the nlohmann_json library is installed

2. Create a build directory in the project directory and enter it:

```bash
mkdir -p build && cd build
```

3. Run CMake and build commands:

```bash
cmake ..
make
```

4. Run the test programs:

```bash
./test_basic
./test_advanced
```

## Supported Data Types

- **Basic Types**: char, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long, float, double, bool
- **Strings**: C-style character arrays (char[])
- **Structs**: Supports nested structs
- **Arrays**: Arrays of basic types and struct arrays
- **Function Pointers**: Will be marked as `"[function_pointer]"` but not actually serialized

## Notes

- Function pointers are not actually serialized, they are only marked as `"[function_pointer]"` in JSON
- The framework only supports C-style character arrays (char[]), not directly supporting std::string
- For character arrays, the framework ensures proper handling of string terminators
- Processing of nested structs and arrays is automatic, no manual configuration needed
- Ensure struct definitions are completed before use to properly register metadata
- The framework supports structs with up to 30 fields

## Comparison with Original Framework

| Feature | Original Framework | New Framework |
|---------|-------------------|---------------|
| Configuration Requirements | Manual callback function writing | Zero configuration, automatic registration |
| API Simplicity | Multiple macros and functions | Simple `jston::to_json`/`jston::from_json` functions |
| Type Safety | Partial type safety | Higher level of type safety |
| Extensibility | Limited | Better extensibility |
| Usability | Moderate | High |

## License

Apache 2.0