#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <climits>
#include <cfloat>
#include <chrono>
#include "jston.h"

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

// struct for testing edge cases
struct EdgeCaseStruct {
    char very_short_string[2];                  // very short string array
    int array_with_uninitialized_elements[10];  // array with uninitialized elements
    bool boolean_values[3];                     // boolean type array
};
register_json_struct(EdgeCaseStruct, very_short_string, array_with_uninitialized_elements, boolean_values);

// struct for testing special characters
struct SpecialCharsStruct {
    char special_string[64];
    char escaped_chars[64];
};
register_json_struct(SpecialCharsStruct, special_string, escaped_chars);

// struct for testing extreme values
struct ExtremeValuesStruct {
    int min_int;
    int max_int;
    double min_double;
    double max_double;
    float min_float;
    float max_float;
};
register_json_struct(ExtremeValuesStruct, min_int, max_int, min_double, max_double, min_float, max_float);

// recursive struct definition (this might have issues with current framework)
struct RecursiveStruct;

struct RecursiveStruct {
    int id;
    RecursiveStruct* child;
};
// note: the current framework may not support serialization of function pointers and pointer types
// register_json_struct(RecursiveStruct, id, child);

// struct with only one field for testing
struct SingleFieldStruct {
    int only_field;
};
register_json_struct(SingleFieldStruct, only_field);

// struct with very large char array for testing
struct LargeCharArrayStruct {
    char large_string[1024];
};
register_json_struct(LargeCharArrayStruct, large_string);

// struct for testing field names conflicting with c++ keywords
struct KeywordFieldStruct {
    int int_;  // trailing underscore to avoid conflict with keyword
    int struct_;
    int class_;
    int private_;
    int public_;
};
register_json_struct(KeywordFieldStruct, int_, struct_, class_, private_, public_);

// print separator function
void print_separator() {
    std::cout << "\n======================================================================\n" << std::endl;
}

// test edge cases
void test_edge_cases() {
    std::cout << "=== Testing Edge Cases ===" << std::endl;

    // test short string array
    EdgeCaseStruct edge;
    strcpy(edge.very_short_string, "A");  // Should copy only one character plus null terminator

    // initialize some array elements
    edge.array_with_uninitialized_elements[0] = 1;
    edge.array_with_uninitialized_elements[9] = 10;

    // initialize boolean array
    edge.boolean_values[0] = true;
    edge.boolean_values[1] = false;
    edge.boolean_values[2] = true;

    // serialization and deserialization
    try {
        nlohmann::json edge_json = jston::to_json(edge);
        std::cout << "EdgeCaseStruct serialization successful!\n" << edge_json.dump(2) << std::endl;
        
        EdgeCaseStruct edge_loaded;
        jston::from_json(edge_json, edge_loaded);
        std::cout << "EdgeCaseStruct deserialization successful!" << std::endl;
        std::cout << "very_short_string: " << edge_loaded.very_short_string << std::endl;
        std::cout << "array_with_uninitialized_elements[0]: " << edge_loaded.array_with_uninitialized_elements[0] << std::endl;
        std::cout << "array_with_uninitialized_elements[9]: " << edge_loaded.array_with_uninitialized_elements[9] << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "EdgeCaseStruct test failed: " << e.what() << std::endl;
    }
}

// test special characters
void test_special_characters() {
    std::cout << "=== Testing Special Characters ===" << std::endl;

    SpecialCharsStruct special;
    strcpy(special.special_string, "special chars test: 中文, にほん, 한국어");
    strcpy(special.escaped_chars, "Escaped chars: \\n \\t \\r \\\\ \\\" \\\\'");

    try {
        nlohmann::json special_json = jston::to_json(special);
        std::cout << "SpecialCharsStruct serialization successful!\n" << special_json.dump(2) << std::endl;
        
        SpecialCharsStruct special_loaded;
        jston::from_json(special_json, special_loaded);
        std::cout << "SpecialCharsStruct deserialization successful!" << std::endl;
        std::cout << "special_string: " << special_loaded.special_string << std::endl;
        std::cout << "escaped_chars: " << special_loaded.escaped_chars << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "SpecialCharsStruct test failed: " << e.what() << std::endl;
    }
}

// test extreme values
void test_extreme_values() {
    std::cout << "=== Testing Extreme Values ===" << std::endl;

    ExtremeValuesStruct extreme;
    extreme.min_int = INT_MIN;
    extreme.max_int = INT_MAX;
    extreme.min_double = -DBL_MAX;
    extreme.max_double = DBL_MAX;
    extreme.min_float = -FLT_MAX;
    extreme.max_float = FLT_MAX;

    try {
        nlohmann::json extreme_json = jston::to_json(extreme);
        std::cout << "ExtremeValuesStruct serialization successful!\n" << extreme_json.dump(2) << std::endl;

        ExtremeValuesStruct extreme_loaded;
        jston::from_json(extreme_json, extreme_loaded);
        std::cout << "ExtremeValuesStruct deserialization successful!" << std::endl;
        std::cout << "min_int: " << extreme_loaded.min_int << std::endl;
        std::cout << "max_int: " << extreme_loaded.max_int << std::endl;
        std::cout << "min_double: " << extreme_loaded.min_double << std::endl;
        std::cout << "max_double: " << extreme_loaded.max_double << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ExtremeValuesStruct test failed: " << e.what() << std::endl;
    }
}

// test single field struct
void test_single_field_struct() {
    std::cout << "=== Testing Single Field Struct ===" << std::endl;

    SingleFieldStruct single;
    single.only_field = 42;

    try {
        nlohmann::json single_json = jston::to_json(single);
        std::cout << "SingleFieldStruct serialization successful!\n" << single_json.dump(2) << std::endl;
        
        SingleFieldStruct single_loaded;
        jston::from_json(single_json, single_loaded);
        std::cout << "SingleFieldStruct deserialization successful!" << std::endl;
        std::cout << "only_field: " << single_loaded.only_field << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "SingleFieldStruct test failed: " << e.what() << std::endl;
    }
}

// test large char array
void test_large_char_array() {
    std::cout << "=== Testing Large Character Array ===" << std::endl;

    LargeCharArrayStruct large;
    memset(large.large_string, 'A', 1023);  // fill most characters
    large.large_string[1023] = '\0';        // ensure string termination
    large.large_string[100] = 'B';          // set a different character in the middle as marker

    try {
        nlohmann::json large_json = jston::to_json(large);
        std::cout << "LargeCharArrayStruct serialization successful!" << std::endl;
        
        // only print partial content to avoid excessive output
        std::string json_str = large_json.dump(2);
        if (json_str.length() > 200) {
            std::cout << "JSON (truncated): " << json_str.substr(0, 200) << "..." << std::endl;
        } else {
            std::cout << "JSON: " << json_str << std::endl;
        }
        
        LargeCharArrayStruct large_loaded;
        jston::from_json(large_json, large_loaded);
        std::cout << "LargeCharArrayStruct deserialization successful!" << std::endl;
        std::cout << "large_string[0]: " << large_loaded.large_string[0] << std::endl;
        std::cout << "large_string[100]: " << large_loaded.large_string[100] << std::endl;
        std::cout << "large_string[1022]: " << large_loaded.large_string[1022] << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "LargeCharArrayStruct test failed: " << e.what() << std::endl;
    }
}

// test keyword field names
void test_keyword_field_names() {
    std::cout << "=== Testing Keyword Field Names ===" << std::endl;

    KeywordFieldStruct keyword;
    keyword.int_ = 1;
    keyword.struct_ = 2;
    keyword.class_ = 3;
    keyword.private_ = 4;
    keyword.public_ = 5;

    try {
        nlohmann::json keyword_json = jston::to_json(keyword);
        std::cout << "KeywordFieldStruct serialization successful!\n" << keyword_json.dump(2) << std::endl;
        
        KeywordFieldStruct keyword_loaded;
        jston::from_json(keyword_json, keyword_loaded);
        std::cout << "KeywordFieldStruct deserialization successful!" << std::endl;
        std::cout << "int_: " << keyword_loaded.int_ << std::endl;
        std::cout << "struct_: " << keyword_loaded.struct_ << std::endl;
        std::cout << "class_: " << keyword_loaded.class_ << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "KeywordFieldStruct test failed: " << e.what() << std::endl;
    }
}

// define struct for performance testing (must be defined outside function because register_json_struct cannot be used inside functions)
struct PerformanceTestStruct {
    int array[1000];
    double double_array[500];
};
register_json_struct(PerformanceTestStruct, array, double_array);

// test serialization performance (for large structs)
void test_performance() {
    std::cout << "=== Testing Serialization Performance ===" << std::endl;

    PerformanceTestStruct perf;
    for (int i = 0; i < 1000; i++) {
        perf.array[i] = i;
    }
    for (int i = 0; i < 500; i++) {
        perf.double_array[i] = i * 1.1;
    }

    try {
        auto start = std::chrono::high_resolution_clock::now();
        nlohmann::json perf_json = jston::to_json(perf);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Large struct serialization completed in: " << duration.count() << " ms" << std::endl;
        
        start = std::chrono::high_resolution_clock::now();
        PerformanceTestStruct perf_loaded;
        jston::from_json(perf_json, perf_loaded);
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Large struct deserialization completed in: " << duration.count() << " ms" << std::endl;
        
        // verify some values
        std::cout << "Verification: array[0]=" << perf_loaded.array[0] 
                  << ", array[999]=" << perf_loaded.array[999]
                  << std::endl;
        std::cout << "Verification: double_array[0]=" << perf_loaded.double_array[0] 
                  << ", double_array[499]=" << perf_loaded.double_array[499] << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Performance test failed: " << e.what() << std::endl;
    }
}

// test error handling
void test_error_handling() {
    std::cout << "=== Testing Error Handling ===" << std::endl;

    // test invalid json deserialization
    try {
        std::string invalid_json = "{invalid json}";
        Person person;
        nlohmann::json j = nlohmann::json::parse(invalid_json);
        jston::from_json(j, person);
        std::cout << "This line should not be executed!" << std::endl;
    } catch (const nlohmann::json::exception& e) {
        std::cout << "Successfully caught JSON parsing error: " << e.what() << std::endl;
    }

    // test type mismatch
    try {
        nlohmann::json j = "string instead of object";
        Person person;
        jston::from_json(j, person);
        std::cout << "This line should not be executed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Successfully caught type mismatch error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== JSON Translator Advanced Test Program ===" << std::endl;

    // test edge cases
    test_edge_cases();
    print_separator();

    // test special characters
    test_special_characters();
    print_separator();

    // test extreme values
    test_extreme_values();
    print_separator();

    // test single field struct
    test_single_field_struct();
    print_separator();

    // test large char array
    test_large_char_array();
    print_separator();

    // test keyword field names
    test_keyword_field_names();
    print_separator();

    // test serialization performance
    test_performance();
    print_separator();

    // test error handling
    test_error_handling();

    std::cout << "\n=== Advanced Test Program Completed ===" << std::endl;
    return 0;
}