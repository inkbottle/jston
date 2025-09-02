#include <iostream>
#include <string>
#include <cstring>
#include "jston.h"

// function pointer type definition
typedef void (*logger_func)(const char*);

// simple logger function
void simple_logger(const char* message) {
    std::cout << "[LOG] " << message << std::endl;
}

// use original struct definition statement instead of macro
struct Car {
    int id;
    double price;
    char brand[32];
    char model[32];
};
register_json_struct(Car, id, price, brand, model);

struct Person {
    int age;
    char name[32];
    Car car;
    int phone_numbers[5];
};
register_json_struct(Person, age, name, car, phone_numbers);

struct Company {
    char name[32];
    Person employees[10];
    int employee_count;
};
register_json_struct(Company, name, employees, employee_count);

struct SystemConfig {
    int log_level;
    bool enable_debug;
    logger_func logger;
};
register_json_struct(SystemConfig, log_level, enable_debug, logger);

// define a new struct for nested array testing
struct Address {
    char street[64];
    char city[32];
    char state[16];
    int zip_code;
};
register_json_struct(Address, street, city, state, zip_code);

// define a struct containing basic type arrays and nested struct arrays
struct ComplexPersonType {
    int id;
    char name[32];
    double scores[5];      // double array
    float salaries[3];     // float array
    long ids[4];           // long array
    short counts[6];       // short array
    Address addresses[2];  // nested struct array
};
register_json_struct(ComplexPersonType, id, name, scores, salaries, ids, counts, addresses);

// define 5-level nested struct for testing complex nested arrays
struct Level5 {
    int id;
    char name[16];
    double value;
};
register_json_struct(Level5, id, name, value);

struct Level4 {
    int id;
    char name[16];
    Level5 items[3];
};
register_json_struct(Level4, id, name, items);

struct Level3 {
    int id;
    char name[16];
    Level4 items[2];
};
register_json_struct(Level3, id, name, items);

struct Level2 {
    int id;
    char name[16];
    Level3 items[2];
};
register_json_struct(Level2, id, name, items);

struct Level1 {
    int id;
    char name[16];
    Level2 items[2];
};
register_json_struct(Level1, id, name, items);

// define a struct with 30 fields for testing
struct LargeStruct {
    int field1;
    double field2;
    char field3[32];
    bool field4;
    char field5[16];
    short field6;
    long field7;
    float field8;
    char field9[8];
    unsigned int field10;
    int field11;
    double field12;
    char field13[32];
    bool field14;
    char field15[16];
    short field16;
    long field17;
    float field18;
    char field19[8];
    unsigned int field20;
    int field21;
    double field22;
    char field23[32];
    bool field24;
    char field25[16];
    short field26;
    long field27;
    float field28;
    char field29[8];
    unsigned int field30;
};
register_json_struct(LargeStruct, field1, field2, field3, field4, field5, field6, field7, field8, field9, field10,
                     field11, field12, field13, field14, field15, field16, field17, field18, field19, field20, field21,
                     field22, field23, field24, field25, field26, field27, field28, field29, field30);

// Print separator function
void print_separator() {
    std::cout << "\n======================================================================\n" << std::endl;
}

// Test basic struct serialization and deserialization
void test_basic_struct_serialization() {
    std::cout << "=== Testing Basic Struct Serialization and Deserialization ===" << std::endl;

    // Initialize struct
    Person person_1;
    person_1.age = 30;
    strcpy(person_1.name, "John Doe");
    person_1.car.id = 1001;
    person_1.car.price = 35000.50;
    strcpy(person_1.car.brand, "Toyota");
    strcpy(person_1.car.model, "Camry");
    person_1.phone_numbers[0] = 123456789;
    person_1.phone_numbers[1] = 987654321;

    // Test struct to JSON conversion
    nlohmann::json json_str = jston::to_json(person_1);
    std::cout << "Person to JSON:\n" << json_str.dump(4) << std::endl;

    // Test JSON to struct conversion
    Person person_2;
    jston::from_json(json_str, person_2);
    std::cout << "\nJSON to Person:\nage: " << person_2.age << ", name: " << person_2.name;
    std::cout << ", car brand: " << person_2.car.brand << ", car model: " << person_2.car.model << std::endl;
}

// Test nested structs and arrays
void test_nested_struct_and_array() {
    std::cout << "=== Testing Nested Structs and Arrays ===" << std::endl;

    // initialize person struct
    Person person_1;
    person_1.age = 30;
    strcpy(person_1.name, "John Doe");
    person_1.car.id = 1001;
    person_1.car.price = 35000.50;
    strcpy(person_1.car.brand, "Toyota");
    strcpy(person_1.car.model, "Camry");
    person_1.phone_numbers[0] = 123456789;
    person_1.phone_numbers[1] = 987654321;

    // Test nested structs and arrays
    Company company;
    strcpy(company.name, "TechCorp");
    company.employee_count = 2;
    company.employees[0] = person_1;
    
    // Create another Person object for employee array
    Person person_2;
    nlohmann::json json_person = jston::to_json(person_1);
    jston::from_json(json_person, person_2);
    company.employees[1] = person_2;

    nlohmann::json company_json = jston::to_json(company);
    std::cout << "Company to JSON:\n" << company_json.dump(4) << std::endl;
}

// Test struct with function pointer
void test_struct_with_function_pointer() {
    std::cout << "=== Testing Struct with Function Pointer ===" << std::endl;

    SystemConfig config;
    config.log_level = 2;
    config.enable_debug = true;
    config.logger = simple_logger;

    nlohmann::json config_json = jston::to_json(config);
    std::cout << "SystemConfig to JSON:\n" << config_json.dump(4) << std::endl;

    // Test function call
    config.logger("Testing logger function pointer");
}

// Test basic type and array serialization
void test_basic_type_and_array_serialization() {
    std::cout << "=== Testing Basic Type and Array Serialization ===" << std::endl;

    Person person_3;
    person_3.age = 25;
    strcpy(person_3.name, "Jane Smith");
    person_3.car.id = 1002;
    person_3.car.price = 45000.75;
    strcpy(person_3.car.brand, "Honda");
    strcpy(person_3.car.model, "Accord");
    person_3.phone_numbers[0] = 555123456;

    std::cout << "Person3 data:\nage: " << person_3.age << ", name: " << person_3.name << std::endl;
}

// Test struct with 30 fields
void test_large_struct() {
    std::cout << "=== Testing Struct with 30 Fields ===" << std::endl;

    LargeStruct large;
    large.field1 = 1;
    large.field2 = 2.2;
    strcpy(large.field3, "field3");
    large.field4 = true;
    strcpy(large.field5, "field5");
    large.field6 = 6;
    large.field7 = 77777777;
    large.field8 = 8.8f;
    strcpy(large.field9, "field9");
    large.field10 = 10;
    large.field11 = 11;
    large.field12 = 12.12;
    strcpy(large.field13, "field13");
    large.field14 = false;
    strcpy(large.field15, "field15");
    large.field16 = 16;
    large.field17 = 17171717;
    large.field18 = 18.18f;
    strcpy(large.field19, "field19");
    large.field20 = 20;
    large.field21 = 21;
    large.field22 = 22.22;
    strcpy(large.field23, "field23");
    large.field24 = true;
    strcpy(large.field25, "field25");
    large.field26 = 26;
    large.field27 = 27272727;
    large.field28 = 28.28f;
    strcpy(large.field29, "field29");
    large.field30 = 30;

    // serialize 30-field struct
    nlohmann::json large_json = jston::to_json(large);
    std::cout << "LargeStruct to JSON:\n" << large_json.dump(4) << std::endl;

    // deserialize 30-field struct
    LargeStruct large_loaded;
    jston::from_json(large_json, large_loaded);
    std::cout << "\nLargeStruct deserialized successfully!" << std::endl;
    std::cout << "Sample fields: field1=" << large_loaded.field1 << ", field2=" << large_loaded.field2
              << ", field3=" << large_loaded.field3 << ", field30=" << large_loaded.field30 << std::endl;
}

// Test basic type arrays and nested struct arrays
void test_basic_type_arrays_and_nested_struct_arrays() {
    std::cout << "=== Testing Basic Type Arrays and Nested Struct Arrays ===" << std::endl;

    ComplexPersonType complex_person;
    complex_person.id = 101;
    strcpy(complex_person.name, "Complex Person");

    // Initialize basic type arrays
    complex_person.scores[0] = 95.5;
    complex_person.scores[1] = 88.5;
    complex_person.scores[2] = 92.0;
    complex_person.scores[3] = 87.5;
    complex_person.scores[4] = 90.0;

    complex_person.salaries[0] = 50000.5f;
    complex_person.salaries[1] = 60000.75f;
    complex_person.salaries[2] = 70000.25f;

    complex_person.ids[0] = 10000000;
    complex_person.ids[1] = 20000000;
    complex_person.ids[2] = 30000000;
    complex_person.ids[3] = 40000000;

    complex_person.counts[0] = 10;
    complex_person.counts[1] = 20;
    complex_person.counts[2] = 30;
    complex_person.counts[3] = 40;
    complex_person.counts[4] = 50;
    complex_person.counts[5] = 60;

    // Initialize nested struct arrays
    strcpy(complex_person.addresses[0].street, "123 Main St");
    strcpy(complex_person.addresses[0].city, "New York");
    strcpy(complex_person.addresses[0].state, "NY");
    complex_person.addresses[0].zip_code = 10001;

    strcpy(complex_person.addresses[1].street, "456 Elm St");
    strcpy(complex_person.addresses[1].city, "Boston");
    strcpy(complex_person.addresses[1].state, "MA");
    complex_person.addresses[1].zip_code = 2108;

    // Serialize ComplexPersonType
    nlohmann::json complex_json = jston::to_json(complex_person);
    std::cout << "ComplexPersonType to JSON:\n" << complex_json.dump(4) << std::endl;

    // Deserialize ComplexPersonType
    ComplexPersonType complex_loaded;
    jston::from_json(complex_json, complex_loaded);

    // Verify basic type arrays
    std::cout << "\nVerifying basic type arrays:" << std::endl;
    std::cout << "scores: [";
    for (int i = 0; i < 5; i++) {
        std::cout << complex_loaded.scores[i];
        if (i < 4) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    // Verify nested struct arrays
    std::cout << "\nVerifying nested struct arrays:" << std::endl;
    for (int i = 0; i < 2; i++) {
        std::cout << "Address " << i + 1 << ": " << complex_loaded.addresses[i].street << ", "
                  << complex_loaded.addresses[i].city << ", " << complex_loaded.addresses[i].state << " "
                  << complex_loaded.addresses[i].zip_code << std::endl;
    }
}

// Test 5-level nested struct array
void test_deep_nested_struct_array()
{
    std::cout << "=== Testing 5-Level Nested Struct Array ===" << std::endl;

    Level1 nested_struct;
    nested_struct.id = 1;
    strcpy(nested_struct.name, "Level1");

    // initialize level 2
    for (int i = 0; i < 2; i++) {
        nested_struct.items[i].id = i + 100;
        sprintf(nested_struct.items[i].name, "Level2_%d", i);

        // initialize level 3
        for (int j = 0; j < 2; j++) {
            nested_struct.items[i].items[j].id = i * 1000 + j + 1000;
            sprintf(nested_struct.items[i].items[j].name, "Level3_%d_%d", i, j);

            // initialize level 4
            for (int k = 0; k < 2; k++) {
                nested_struct.items[i].items[j].items[k].id = i * 10000 + j * 1000 + k + 10000;
                sprintf(nested_struct.items[i].items[j].items[k].name, "Level4_%d_%d_%d", i, j, k);

                // initialize level 5
                for (int l = 0; l < 3; l++) {
                    nested_struct.items[i].items[j].items[k].items[l].id =
                        i * 100000 + j * 10000 + k * 1000 + l + 100000;
                    sprintf(nested_struct.items[i].items[j].items[k].items[l].name, "Level5_%d_%d_%d_%d", i, j, k, l);
                    nested_struct.items[i].items[j].items[k].items[l].value = (i * 100 + j * 10 + k) * 1.1 + l * 0.5;
                }
            }
        }
    }

    // Serialize 5-level nested struct
    try {
        nlohmann::json nested_json = jston::to_json(nested_struct);
        std::cout << "5-level nested struct serialization successful!" << std::endl;

        // output only partial content as full output would be very long
        std::string json_str = nested_json.dump(2);
        std::cout << "JSON:\n" << json_str << std::endl;

        // deserialize 5-level nested struct
        Level1 nested_loaded;
        jston::from_json(nested_json, nested_loaded);

        // Verify deserialization results
        std::cout << "\nVerifying deserialization results:" << std::endl;
        std::cout << "Level1: id=" << nested_loaded.id << ", name=" << nested_loaded.name << std::endl;

        // check some deep fields to verify nested structure correctness
        std::cout << "Deep field check: " << nested_loaded.items[1].name << " -> "
                  << nested_loaded.items[1].items[1].name << " -> " << nested_loaded.items[1].items[1].items[1].name
                  << " -> " << nested_loaded.items[1].items[1].items[1].items[2].name
                  << " (value: " << nested_loaded.items[1].items[1].items[1].items[2].value << ")" << std::endl;

        // Verify serialization and deserialization consistency
        nlohmann::json nested_json2 = jston::to_json(nested_loaded);
        if (nested_json == nested_json2) {
            std::cout << "\nSerialization and deserialization consistency verification passed!" << std::endl;
        } else {
            std::cout << "\nWarning: Serialization and deserialization results are inconsistent!" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Serialization/deserialization failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== JSON Translator Framework Example Program ===" << std::endl;
    
    // Test basic struct serialization and deserialization
    test_basic_struct_serialization();
    print_separator();
    
    // Test nested structs and arrays
    test_nested_struct_and_array();
    print_separator();
    
    // Test struct with function pointer
    test_struct_with_function_pointer();
    print_separator();
    
    // Test basic type and array serialization
    test_basic_type_and_array_serialization();
    print_separator();
    
    // Test struct with 30 fields
    test_large_struct();
    print_separator();
    
    // Test basic type arrays and nested struct arrays
    test_basic_type_arrays_and_nested_struct_arrays();
    print_separator();
    
    // Test 5-level nested struct array
    test_deep_nested_struct_array();
    
    std::cout << "\n=== Example Program Completed ===" << std::endl;
    return 0;
}