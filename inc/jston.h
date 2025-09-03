#ifndef __JSTON_H__
#define __JSTON_H__

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

/**
 * jston - a simple and easy-to-use C++ struct to JSON conversion framework
 * features:
 * 1. no additional configuration code required after creating structs
 * 2. automatically handles complex types like nested structs and arrays
 * 3. provides clean API interface
 */

namespace jston {

// type encoding system
enum class TYPE_CODE {
    UNKNOWN = 0x00,
    CHAR = 0x01,
    SHORT = 0x02,
    INT = 0x03,
    LONG = 0x04,
    LONG_LONG = 0x05,
    U_SHORT = 0x06,
    U_INT = 0x07,
    U_LONG = 0x08,
    U_LONG_LONG = 0x09,
    FLOAT = 0x10,
    DOUBLE = 0x11,
    BOOL = 0x12,      // boolean type
    STRING = 0x13,    // char array
    FUNCTION = 0x14,  // function pointer
    STRUCT = 0x15,    // nested struct
    ARRAY = 0x16,     // array
    POINTER = 0x17    // pointer type
};

// field metadata struct
struct field_metadata {
    const char* name;              // field name
    TYPE_CODE type_code;           // type code
    size_t offset;                 // field offset
    size_t size;                   // field size
    const char* struct_type_name;  // struct type name (if nested struct)
    TYPE_CODE
    sub_type_code;        // When type_code is array, use this for basic element types; for custom structs, use
                          // struct_type_name
    size_t element_size;  // Array element size, valid when type_code is ARRAY
    size_t array_length;  // Array length, valid when type_code is ARRAY
};

// struct metadata manager class
class MetadataManager {
private:
    // store metadata for all structs
    inline static std::unordered_map<std::string, std::vector<field_metadata>> metadata_map;

public:
    // register struct metadata
    static void register_metadata(const std::string& type_id, const std::vector<field_metadata>& fields) {
        metadata_map[type_id] = fields;
    }

    // get struct metadata
    static const std::vector<field_metadata>* get_metadata(const std::string& type_id) {
        auto it = metadata_map.find(type_id);
        if (it != metadata_map.end()) {
            return &(it->second);
        }
        return nullptr;
    }
};

// type traits utility - used to determine type characteristics
template <typename T>
struct type_traits {
    static constexpr bool is_array = false;
    static constexpr bool is_function = false;
    static constexpr bool is_string = false;
    static constexpr bool is_char_array = false;
};

// array type specialization template
template <typename T, size_t N>
struct type_traits<T[N]> {
    static constexpr bool is_array = true;
    static constexpr bool is_function = false;
    static constexpr bool is_string = std::is_same<T, char>::value;
    static constexpr bool is_char_array = std::is_same<T, char>::value;
    using ELEMENT_TYPE = T;
    static constexpr size_t size = N;
};

// function pointer type specialization template
template <typename Ret, typename... Args>
struct type_traits<Ret (*)(Args...)> {
    static constexpr bool is_array = false;
    static constexpr bool is_function = true;
    static constexpr bool is_string = false;
    static constexpr bool is_char_array = false;
    static constexpr TYPE_CODE type_code = TYPE_CODE::FUNCTION;
};

// pointer type specialization template
template <typename T>
struct type_traits<T*> {
    static constexpr bool is_array = false;
    static constexpr bool is_function = false;
    static constexpr bool is_string = false;
    static constexpr bool is_char_array = false;
    static constexpr TYPE_CODE type_code = TYPE_CODE::POINTER;
    using POINTER_TYPE = T;
};

// get type code general template function
template <typename T>
TYPE_CODE get_type_code() {
    if (std::is_same<T, char>::value) {
        return TYPE_CODE::CHAR;
    }
    if (std::is_same<T, short>::value) {
        return TYPE_CODE::SHORT;
    }
    if (std::is_same<T, int>::value) {
        return TYPE_CODE::INT;
    }
    if (std::is_same<T, long>::value) {
        return TYPE_CODE::LONG;
    }
    if (std::is_same<T, long long>::value) {
        return TYPE_CODE::LONG_LONG;
    }
    if (std::is_same<T, unsigned short>::value) {
        return TYPE_CODE::U_SHORT;
    }
    if (std::is_same<T, unsigned int>::value) {
        return TYPE_CODE::U_INT;
    }
    if (std::is_same<T, unsigned long>::value) {
        return TYPE_CODE::U_LONG;
    }
    if (std::is_same<T, unsigned long long>::value) {
        return TYPE_CODE::U_LONG_LONG;
    }
    if (std::is_same<T, float>::value) {
        return TYPE_CODE::FLOAT;
    }
    if (std::is_same<T, double>::value) {
        return TYPE_CODE::DOUBLE;
    }
    if (std::is_same<T, bool>::value) {
        return TYPE_CODE::BOOL;
    }
    // Only C-style char arrays are recognized as string type
    if (type_traits<T>::is_char_array) {
        return TYPE_CODE::STRING;
    }
    if (type_traits<T>::is_function) {
        return TYPE_CODE::FUNCTION;
    }
    if (std::is_pointer<T>::value) {
        return TYPE_CODE::POINTER;
    }
    if (type_traits<T>::is_array && !type_traits<T>::is_char_array) {
        return TYPE_CODE::ARRAY;
    }
    // Default to struct type
    return TYPE_CODE::STRUCT;
}

// forward declare template function for struct registration
template <typename T>
void register_struct_metadata();

// forward declaration of three-parameter from_json function
void from_json(const std::vector<field_metadata>& metadata, const nlohmann::json& j, void* obj);

// forward declaration of three-parameter to_json function
nlohmann::json to_json(const std::vector<field_metadata>& metadata, const void* obj);

// helper template function for registering metadata
template <typename T>
void register_metadata_helper(const std::vector<field_metadata>& fields) {
    MetadataManager::register_metadata(typeid(T).name(), fields);
}

// automatic registration mechanism - automatically call the registration function at program startup
template <typename T>
class AutoRegistrar {
public:
    AutoRegistrar() {
        register_struct_metadata<T>();
    }
};

// trigger automatic registration template variable
template <typename T>
AutoRegistrar<T> g_auto_registrar;

// struct to JSON conversion function
template <typename T>
nlohmann::json to_json(const T& obj) {
    const std::string type_id = typeid(T).name();
    const auto* metadata = MetadataManager::get_metadata(type_id);

    if (!metadata) {
        throw std::runtime_error("No metadata found for type: " + type_id);
    }
    return to_json(*metadata, &obj);
}

// JSON to struct conversion function
template <typename T>
void from_json(const nlohmann::json& j, T& obj) {
    // check if JSON is an object type
    if (!j.is_object()) {
        throw std::runtime_error("JSON value is not an object, cannot convert to struct");
    }

    // get type ID and metadata
    const std::string type_id = typeid(T).name();
    const auto* metadata = MetadataManager::get_metadata(type_id);

    if (!metadata) {
        throw std::runtime_error("No metadata found for type: " + type_id);
    }
    from_json(*metadata, j, &obj);
}

// struct to JSON string conversion function
template <typename T>
std::string to_json_string(const T& obj) {
    return to_json(obj).dump();
}

// JSON string to struct conversion function
template <typename T>
void from_json_string(const std::string& j, T& obj) {
    if (j.empty()) {
        throw std::runtime_error("empty json string provided");
    }
    
    try {
        from_json(nlohmann::json::parse(j), obj);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("json parsing error: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("from_json exception: ") + e.what());
    }
}

// overloaded to_json function, accepts metadata and object pointer as parameters
inline nlohmann::json to_json(const std::vector<field_metadata>& metadata, const void* obj) {
    nlohmann::json result;

    // iterate through all fields and convert
    for (const auto& field : metadata) {
        try {
            // handle differently based on field type
            switch (field.type_code) {
                case TYPE_CODE::CHAR: {
                    const char& value =
                        *reinterpret_cast<const char*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = static_cast<uint8_t>(value);
                    break;
                }
                case TYPE_CODE::SHORT: {
                    const short& value =
                        *reinterpret_cast<const short*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::INT: {
                    const int& value = *reinterpret_cast<const int*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::LONG: {
                    const long& value =
                        *reinterpret_cast<const long*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::LONG_LONG: {
                    const long long& value =
                        *reinterpret_cast<const long long*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::U_SHORT: {
                    const unsigned short& value =
                        *reinterpret_cast<const unsigned short*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::U_INT: {
                    const unsigned int& value =
                        *reinterpret_cast<const unsigned int*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::U_LONG: {
                    const unsigned long& value =
                        *reinterpret_cast<const unsigned long*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::U_LONG_LONG: {
                    const unsigned long long& value =
                        *reinterpret_cast<const unsigned long long*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::FLOAT: {
                    const float& value =
                        *reinterpret_cast<const float*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::DOUBLE: {
                    const double& value =
                        *reinterpret_cast<const double*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::BOOL: {
                    const bool& value =
                        *reinterpret_cast<const bool*>(reinterpret_cast<const char*>(obj) + field.offset);
                    result[field.name] = value;
                    break;
                }
                case TYPE_CODE::STRING: {
                    // char array
                    const char* value =
                        reinterpret_cast<const char*>(reinterpret_cast<const char*>(obj) + field.offset);
                    // create a safe std::string, handle only ascii characters
                    std::string safe_string;
                    size_t max_chars = field.size > 0 ? field.size : 256;  // use field size or default value
                    for (size_t i = 0; i < max_chars; ++i) {
                        unsigned char c = static_cast<unsigned char>(value[i]);
                        if (c == '\0') {
                            break;  // end of string
                        }
                        if (c < 128) {
                            safe_string += c;  // add only ASCII characters
                        }
                    }
                    result[field.name] = safe_string;
                    break;
                }
                case TYPE_CODE::FUNCTION: {
                    // simplified handling for function pointers
                    result[field.name] = "[function_pointer]";
                    break;
                }
                case TYPE_CODE::POINTER: {
                    // get the pointer value is not necessary
                    result[field.name] = "[pointer]";
                    break;
                }
                case TYPE_CODE::STRUCT: {
                    // handle nested struct
                    const void* struct_ptr =
                        reinterpret_cast<const void*>(reinterpret_cast<const char*>(obj) + field.offset);

                    // get struct type name and convert
                    if (field.struct_type_name && *field.struct_type_name) {
                        const auto* struct_metadata = MetadataManager::get_metadata(field.struct_type_name);
                        if (struct_metadata) {
                            result[field.name] = jston::to_json(*struct_metadata, struct_ptr);
                        } else {
                            result[field.name] = "[struct]";
                        }
                    } else {
                        result[field.name] = "[struct]";
                    }
                    break;
                }
                case TYPE_CODE::ARRAY: {  // handle array
                    const void* array_ptr =
                        reinterpret_cast<const void*>(reinterpret_cast<const char*>(obj) + field.offset);
                    nlohmann::json array = nlohmann::json::array();

                    // prefer to use precomputed array element size and length
                    if (field.element_size > 0 && field.array_length > 0) {
                        // handle struct array
                        if (field.struct_type_name && *field.struct_type_name) {
                            // try to find corresponding struct metadata
                            const auto* struct_metadata = MetadataManager::get_metadata(field.struct_type_name);
                            if (struct_metadata) {
                                // iterate through each element in array
                                for (size_t i = 0; i < field.array_length; ++i) {
                                    const void* element_ptr =
                                        static_cast<const char*>(array_ptr) + i * field.element_size;
                                    nlohmann::json element_json = jston::to_json(*struct_metadata, element_ptr);
                                    array.push_back(element_json);
                                }
                            }
                        } else {
                            // handle basic type array
                            // note: even if sub_type_code is UNKNOWN, we still try to handle to avoid random values
                            // from uninitialized arrays for unrecognized types, we mark them as [unknown_array_type]
                            if (field.sub_type_code == TYPE_CODE::UNKNOWN) {
                                array.push_back("[unknown_array_type]");
                            } else {
                                switch (field.sub_type_code) {
                                    case TYPE_CODE::DOUBLE: {
                                        const double* double_array = static_cast<const double*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(double_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::FLOAT: {
                                        const float* float_array = static_cast<const float*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(float_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::LONG_LONG: {
                                        const long long* longlong_array = static_cast<const long long*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(longlong_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::LONG: {
                                        const long* long_array = static_cast<const long*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(long_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::INT: {
                                        const int* int_array = static_cast<const int*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(int_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::SHORT: {
                                        const short* short_array = static_cast<const short*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(short_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::U_INT: {
                                        const unsigned int* uint_array = static_cast<const unsigned int*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(uint_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::U_SHORT: {
                                        const unsigned short* ushort_array =
                                            static_cast<const unsigned short*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(ushort_array[i]);
                                        }
                                        break;
                                    }
                                    case TYPE_CODE::BOOL: {
                                        const bool* bool_array = static_cast<const bool*>(array_ptr);
                                        for (size_t i = 0; i < field.array_length; ++i) {
                                            array.push_back(bool_array[i]);
                                        }
                                        break;
                                    }
                                    default:
                                        // unrecognized array type
                                        array.push_back("[unknown_array]");
                                        break;
                                }
                            }
                        }
                    } else {
                        // use traditional method to handle arrays as fallback solution
                        // handling priority: 1. prefer struct_type_name for precise matching
                        // first check if struct_type_name exists and is valid
                        if (field.struct_type_name && *field.struct_type_name) {
                            // try to find corresponding struct metadata
                            const auto* struct_metadata = MetadataManager::get_metadata(field.struct_type_name);
                            if (struct_metadata) {
                                // calculate array element size
                                size_t element_size = 0;
                                for (const auto& f : *struct_metadata) {
                                    size_t field_end = f.offset + (f.size > 0 ? f.size : sizeof(void*));
                                    if (field_end > element_size) {
                                        element_size = field_end;
                                    }
                                }

                                // ensure alignment
                                element_size = (element_size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

                                // calculate number of elements in array
                                int array_size = field.size / element_size;

                                // iterate through each element in array
                                for (int i = 0; i < array_size; ++i) {
                                    const void* element_ptr = static_cast<const char*>(array_ptr) + i * element_size;
                                    nlohmann::json element_json = jston::to_json(*struct_metadata, element_ptr);
                                    array.push_back(element_json);
                                }
                            } else {
                                // not a struct array, use sub_type_code to precisely match basic type array
                                // this is the main matching method to ensure type safety and precise conversion
                                if (field.sub_type_code == TYPE_CODE::DOUBLE) {
                                    const double* double_array = static_cast<const double*>(array_ptr);
                                    int array_size = field.size / sizeof(double);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(double_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::FLOAT) {
                                    const float* float_array = static_cast<const float*>(array_ptr);
                                    int array_size = field.size / sizeof(float);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(float_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::LONG_LONG) {
                                    const long long* longlong_array = static_cast<const long long*>(array_ptr);
                                    int array_size = field.size / sizeof(long long);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(longlong_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::LONG) {
                                    const long* long_array = static_cast<const long*>(array_ptr);
                                    int array_size = field.size / sizeof(long);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(long_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::INT) {
                                    const int* int_array = static_cast<const int*>(array_ptr);
                                    int array_size = field.size / sizeof(int);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(int_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::SHORT) {
                                    const short* short_array = static_cast<const short*>(array_ptr);
                                    int array_size = field.size / sizeof(short);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(short_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::U_INT) {
                                    const unsigned int* uint_array = static_cast<const unsigned int*>(array_ptr);
                                    int array_size = field.size / sizeof(unsigned int);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(uint_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::U_SHORT) {
                                    const unsigned short* ushort_array = static_cast<const unsigned short*>(array_ptr);
                                    int array_size = field.size / sizeof(unsigned short);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(ushort_array[i]);
                                    }
                                } else if (field.sub_type_code == TYPE_CODE::BOOL) {
                                    const bool* bool_array = static_cast<const bool*>(array_ptr);
                                    int array_size = field.size / sizeof(bool);
                                    for (int i = 0; i < array_size; ++i) {
                                        array.push_back(bool_array[i]);
                                    }
                                } else {
                                    // unrecognized array type
                                    array.push_back("[unknown_array]");
                                }
                            }
                        }
                    }
                    // only add this field if it doesn't exist in result yet
                    if (!result.contains(field.name)) {
                        result[field.name] = array;
                    }
                    break;
                }
                default:
                    result[field.name] = "[unknown_type]";
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error converting field '" << field.name << "': " << e.what() << std::endl;
            result[field.name] = "[error]";
        }
    }

    return result;
}

// three-parameter from_json function implementation
inline void from_json(const std::vector<field_metadata>& metadata, const nlohmann::json& j, void* obj) {
    // iterate through all fields and convert
    for (const auto& field : metadata) {
        try {
            // check if field exists and is not null
            if (j.find(field.name) == j.end() || j[field.name].is_null()) {
                continue;
            }

            // handle differently based on field type
            switch (field.type_code) {
                case TYPE_CODE::CHAR: {
                    char& value = *reinterpret_cast<char*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = static_cast<char>(j[field.name].get<uint8_t>());
                    break;
                }
                case TYPE_CODE::SHORT: {
                    short& value = *reinterpret_cast<short*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<short>();
                    break;
                }
                case TYPE_CODE::INT: {
                    int& value = *reinterpret_cast<int*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<int>();
                    break;
                }
                case TYPE_CODE::LONG: {
                    long& value = *reinterpret_cast<long*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<long>();
                    break;
                }
                case TYPE_CODE::LONG_LONG: {
                    long long& value = *reinterpret_cast<long long*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<long long>();
                    break;
                }
                case TYPE_CODE::U_SHORT: {
                    unsigned short& value =
                        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<unsigned short>();
                    break;
                }
                case TYPE_CODE::U_INT: {
                    unsigned int& value = *reinterpret_cast<unsigned int*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<unsigned int>();
                    break;
                }
                case TYPE_CODE::U_LONG: {
                    unsigned long& value =
                        *reinterpret_cast<unsigned long*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<unsigned long>();
                    break;
                }
                case TYPE_CODE::U_LONG_LONG: {
                    unsigned long long& value =
                        *reinterpret_cast<unsigned long long*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<unsigned long long>();
                    break;
                }
                case TYPE_CODE::FLOAT: {
                    float& value = *reinterpret_cast<float*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<float>();
                    break;
                }
                case TYPE_CODE::DOUBLE: {
                    double& value = *reinterpret_cast<double*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<double>();
                    break;
                }
                case TYPE_CODE::BOOL: {
                    bool& value = *reinterpret_cast<bool*>(reinterpret_cast<char*>(obj) + field.offset);
                    value = j[field.name].get<bool>();
                    break;
                }
                case TYPE_CODE::STRING: {
                    // check if this is a char pointer (char* or const char*)
                    if (std::is_pointer<decltype(*reinterpret_cast<void**>(reinterpret_cast<char*>(obj) + field.offset))>::value) {
                        // for char pointers, we don't perform deserialization as we can't know where to allocate memory
                        // just leave the pointer as it is (null or existing)
                        break;
                    }
                    // regular char array (c-style string) handling
                    char* value = reinterpret_cast<char*>(reinterpret_cast<char*>(obj) + field.offset);
                    try {
                        std::string str_value = j[field.name].get<std::string>();
                        // use strncpy to prevent overflow and manually add string terminator
                        if (field.size > 0) {
                            strncpy(value, str_value.c_str(), field.size - 1);
                            value[field.size - 1] = '\0';  // ensure string termination
                        }
                    } catch (const std::exception& e) {
                        // handle potential string conversion errors
                        throw std::runtime_error(
                            std::string("Error converting field '" + std::string(field.name) + "': ") + e.what());
                    }
                    break;
                }
                case TYPE_CODE::FUNCTION: {
                    // do not deserialize function pointers
                    break;
                }
                case TYPE_CODE::POINTER: {
                    // explicitly set pointer types to null during deserialization
                    void** ptr = reinterpret_cast<void**>(reinterpret_cast<char*>(obj) + field.offset);
                    *ptr = nullptr;
                    break;
                }
                case TYPE_CODE::STRUCT: {
                    // handle nested struct
                    void* struct_ptr = reinterpret_cast<void*>(reinterpret_cast<char*>(obj) + field.offset);

                    // dynamically call from_json based on struct type name
                    if (field.struct_type_name && *field.struct_type_name) {
                        // get metadata for struct type
                        const auto* struct_metadata = MetadataManager::get_metadata(field.struct_type_name);
                        if (struct_metadata) {
                            // check if field exists in JSON and is not null
                            if (j.find(field.name) != j.end() && !j[field.name].is_null()) {
                                ::jston::from_json(*struct_metadata, j[field.name], struct_ptr);
                            }
                        }
                    }
                    break;
                }
                case TYPE_CODE::ARRAY: {
                    // handle array
                    if (!j.contains(field.name) || !j[field.name].is_array()) {
                        continue;
                    }

                    void* array_ptr = reinterpret_cast<void*>(reinterpret_cast<char*>(obj) + field.offset);

                    const auto& json_array = j[field.name];

                    // first try as struct array
                    if (field.struct_type_name && *field.struct_type_name) {
                        // get metadata for struct type
                        const auto* struct_metadata = MetadataManager::get_metadata(field.struct_type_name);
                        if (struct_metadata) {
                            // prefer to use precomputed element_size and array_length
                            size_t element_size = field.element_size > 0 ? field.element_size : 0;

                            // iterate through each element in array
                            for (size_t i = 0; i < json_array.size(); ++i) {
                                void* element_ptr = static_cast<char*>(array_ptr) + i * element_size;
                                if (i < static_cast<size_t>(json_array.size())) {
                                    ::jston::from_json(*struct_metadata, json_array[i], element_ptr);
                                }
                            }
                        }
                    } else {
                        if (field.sub_type_code == TYPE_CODE::DOUBLE) {
                            double* double_array = static_cast<double*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(double)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number()) {
                                    double_array[i] = json_array[i].get<double>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::FLOAT) {
                            float* float_array = static_cast<float*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(float)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number()) {
                                    float_array[i] = json_array[i].get<float>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::LONG_LONG) {
                            long long* longlong_array = static_cast<long long*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(long long)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number_integer()) {
                                    longlong_array[i] = json_array[i].get<long long>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::LONG) {
                            long* long_array = static_cast<long*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(long)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number_integer()) {
                                    long_array[i] = json_array[i].get<long>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::INT) {
                            int* int_array = static_cast<int*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(int)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number_integer()) {
                                    int_array[i] = json_array[i].get<int>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::SHORT) {
                            short* short_array = static_cast<short*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(short)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number_integer()) {
                                    short_array[i] = json_array[i].get<short>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::U_INT) {
                            unsigned int* uint_array = static_cast<unsigned int*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(unsigned int)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number_unsigned()) {
                                    uint_array[i] = json_array[i].get<unsigned int>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::U_SHORT) {
                            unsigned short* ushort_array = static_cast<unsigned short*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(unsigned short)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_number_unsigned()) {
                                    ushort_array[i] = json_array[i].get<unsigned short>();
                                }
                            }
                        } else if (field.sub_type_code == TYPE_CODE::BOOL) {
                            bool* bool_array = static_cast<bool*>(array_ptr);
                            int array_size;
                            if (field.array_length > 0) {
                                array_size =
                                    std::min(static_cast<int>(json_array.size()), static_cast<int>(field.array_length));
                            } else {
                                array_size = std::min(static_cast<int>(json_array.size()),
                                                      static_cast<int>(field.size / sizeof(bool)));
                            }
                            for (int i = 0; i < array_size; ++i) {
                                if (i < static_cast<int>(json_array.size()) && json_array[i].is_boolean()) {
                                    bool_array[i] = json_array[i].get<bool>();
                                }
                            }
                        } else {
                            // unrecognized basic type array
                            std::cerr << "Error: Unknown basic type array for field '" << field.name << "'"
                                      << std::endl;
                        }
                    }
                } break;
                default:
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing field '" << field.name << "': " << e.what() << std::endl;
        }
    }
}

// macro for adding basic type field metadata
#define STRUCT_TRANSLATOR_ADD_FIELD(field_list, struct_name, type, name)                                               \
    do {                                                                                                               \
        jston::field_metadata field_metadata;                                                                          \
        field_metadata.name = #name;                                                                                   \
        field_metadata.type_code = jston::get_type_code<type>();                                                       \
        field_metadata.offset = offsetof(struct_name, name);                                                           \
        field_metadata.size = sizeof(((struct_name*)0)->name);                                                         \
        field_metadata.struct_type_name = nullptr;                                                                     \
        field_list.push_back(field_metadata);                                                                          \
    } while (0)

// macro for adding struct type field metadata (preserve struct type information)
#define STRUCT_TRANSLATOR_ADD_STRUCT_FIELD(field_list, struct_name, nested_type, name)                                 \
    do {                                                                                                               \
        jston::field_metadata field_metadata;                                                                          \
        field_metadata.name = #name;                                                                                   \
        field_metadata.type_code = jston::TYPE_CODE::STRUCT;                                                           \
        field_metadata.offset = offsetof(struct_name, name);                                                           \
        field_metadata.size = sizeof(((struct_name*)0)->name);                                                         \
        field_metadata.struct_type_name = typeid(nested_type).name();                                                  \
        field_list.push_back(field_metadata);                                                                          \
    } while (0)

// macro for adding array type field metadata (preserve array element type information)
#define STRUCT_TRANSLATOR_ADD_ARRAY_FIELD(field_list, struct_name, array_type, name)                                   \
    do {                                                                                                               \
        jston::field_metadata field_metadata;                                                                          \
        field_metadata.name = #name;                                                                                   \
        field_metadata.type_code = jston::TYPE_CODE::ARRAY;                                                            \
        field_metadata.offset = offsetof(struct_name, name);                                                           \
        field_metadata.size = sizeof(((struct_name*)0)->name);                                                         \
        field_metadata.struct_type_name = nullptr;                                                                     \
        /* calculate array element size and array length */                                                            \
        field_metadata.element_size = sizeof(typename std::remove_extent<array_type>::type);                           \
        field_metadata.array_length = std::extent<array_type>::value;                                                  \
        /* set explicit type name for basic type arrays */                                                             \
        using ARRAY_ELEMENT_TYPE = typename std::remove_all_extents<array_type>::type;                                 \
        if (std::is_same<ARRAY_ELEMENT_TYPE, int>::value) {                                                            \
            field_metadata.sub_type_code = jston::TYPE_CODE::INT;                                                      \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, double>::value) {                                                  \
            field_metadata.sub_type_code = jston::TYPE_CODE::DOUBLE;                                                   \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, float>::value) {                                                   \
            field_metadata.sub_type_code = jston::TYPE_CODE::FLOAT;                                                    \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, long>::value) {                                                    \
            field_metadata.sub_type_code = jston::TYPE_CODE::LONG;                                                     \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, long long>::value) {                                               \
            field_metadata.sub_type_code = jston::TYPE_CODE::LONG_LONG;                                                \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, short>::value) {                                                   \
            field_metadata.sub_type_code = jston::TYPE_CODE::SHORT;                                                    \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, unsigned int>::value) {                                            \
            field_metadata.sub_type_code = jston::TYPE_CODE::U_INT;                                                    \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, unsigned short>::value) {                                          \
            field_metadata.sub_type_code = jston::TYPE_CODE::U_SHORT;                                                  \
        } else if (std::is_same<ARRAY_ELEMENT_TYPE, bool>::value) {                                                    \
            field_metadata.sub_type_code = jston::TYPE_CODE::BOOL;                                                     \
        } else {                                                                                                       \
            /* check if it is a custom struct type */                                                                  \
            if (jston::get_type_code<ARRAY_ELEMENT_TYPE>() == jston::TYPE_CODE::STRUCT) {                              \
                /* for custom struct array, use typeid */                                                              \
                field_metadata.struct_type_name = typeid(ARRAY_ELEMENT_TYPE).name();                                   \
            } else {                                                                                                   \
                /* non-custom struct type, set struct_type_name to null */                                             \
                field_metadata.struct_type_name = nullptr;                                                             \
            }                                                                                                          \
            field_metadata.sub_type_code = jston::TYPE_CODE::UNKNOWN;                                                  \
        }                                                                                                              \
        field_list.push_back(field_metadata);                                                                          \
    } while (0)

// auxiliary macro: handle fields in variable parameter list
#define _REGISTER_FIELD_IMPL(field_list, struct_name, field_name)                                                      \
    do {                                                                                                               \
        jston::field_metadata field_metadata;                                                                          \
        field_metadata.name = #field_name;                                                                             \
        field_metadata.type_code = jston::get_type_code<decltype(struct_name::field_name)>();                          \
        field_metadata.offset = offsetof(struct_name, field_name);                                                     \
        field_metadata.size = sizeof(struct_name::field_name);                                                         \
        field_metadata.sub_type_code = jston::TYPE_CODE::UNKNOWN;                                                      \
        field_metadata.struct_type_name = nullptr;                                                                     \
                                                                                                                       \
        if (std::is_array<decltype(struct_name::field_name)>::value) {                                                 \
            if (!std::is_same<typename std::remove_extent<decltype(struct_name::field_name)>::type, char>::value) {    \
                field_metadata.type_code = jston::TYPE_CODE::ARRAY;                                                    \
                /* calculate array element size and array length */                                                    \
                field_metadata.element_size =                                                                          \
                    sizeof(typename std::remove_extent<decltype(struct_name::field_name)>::type);                      \
                field_metadata.array_length = std::extent<decltype(struct_name::field_name)>::value;                   \
            }                                                                                                          \
            using ARRAY_ELEMENT_TYPE = typename std::remove_extent<decltype(struct_name::field_name)>::type;           \
            if (std::is_same<ARRAY_ELEMENT_TYPE, int>::value) {                                                        \
                field_metadata.sub_type_code = jston::TYPE_CODE::INT;                                                  \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, double>::value) {                                              \
                field_metadata.sub_type_code = jston::TYPE_CODE::DOUBLE;                                               \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, float>::value) {                                               \
                field_metadata.sub_type_code = jston::TYPE_CODE::FLOAT;                                                \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, long>::value) {                                                \
                field_metadata.sub_type_code = jston::TYPE_CODE::LONG;                                                 \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, long long>::value) {                                           \
                field_metadata.sub_type_code = jston::TYPE_CODE::LONG_LONG;                                            \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, short>::value) {                                               \
                field_metadata.sub_type_code = jston::TYPE_CODE::SHORT;                                                \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, unsigned int>::value) {                                        \
                field_metadata.sub_type_code = jston::TYPE_CODE::U_INT;                                                \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, unsigned short>::value) {                                      \
                field_metadata.sub_type_code = jston::TYPE_CODE::U_SHORT;                                              \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else if (std::is_same<ARRAY_ELEMENT_TYPE, bool>::value) {                                                \
                field_metadata.sub_type_code = jston::TYPE_CODE::BOOL;                                                 \
                field_metadata.struct_type_name = nullptr;                                                             \
            } else {                                                                                                   \
                field_metadata.sub_type_code = jston::TYPE_CODE::UNKNOWN;                                              \
                if (jston::get_type_code<ARRAY_ELEMENT_TYPE>() == jston::TYPE_CODE::STRUCT) {                          \
                    field_metadata.struct_type_name = typeid(ARRAY_ELEMENT_TYPE).name();                               \
                } else {                                                                                               \
                    field_metadata.struct_type_name = nullptr; /* Unknown type array */                                \
                }                                                                                                      \
            }                                                                                                          \
        } else if (field_metadata.type_code == jston::TYPE_CODE::STRUCT) {                                             \
            field_metadata.struct_type_name = typeid(decltype(struct_name::field_name)).name();                        \
        } else {                                                                                                       \
            field_metadata.struct_type_name = nullptr;                                                                 \
        }                                                                                                              \
        field_list.push_back(field_metadata);                                                                          \
    } while (0)

// field registration macros (multiple versions for different field counts)
#define _REGISTER_FIELDS_1(field_list, struct_name, field1) _REGISTER_FIELD_IMPL(field_list, struct_name, field1)

#define _REGISTER_FIELDS_2(field_list, struct_name, field1, field2)                                                    \
    _REGISTER_FIELDS_1(field_list, struct_name, field1);                                                               \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field2)

#define _REGISTER_FIELDS_3(field_list, struct_name, field1, field2, field3)                                            \
    _REGISTER_FIELDS_2(field_list, struct_name, field1, field2);                                                       \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field3);

#define _REGISTER_FIELDS_4(field_list, struct_name, field1, field2, field3, field4)                                    \
    _REGISTER_FIELDS_3(field_list, struct_name, field1, field2, field3);                                               \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field4)

#define _REGISTER_FIELDS_5(field_list, struct_name, field1, field2, field3, field4, field5)                            \
    _REGISTER_FIELDS_4(field_list, struct_name, field1, field2, field3, field4);                                       \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field5)

#define _REGISTER_FIELDS_6(field_list, struct_name, field1, field2, field3, field4, field5, field6)                    \
    _REGISTER_FIELDS_5(field_list, struct_name, field1, field2, field3, field4, field5);                               \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field6)

#define _REGISTER_FIELDS_7(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7)            \
    _REGISTER_FIELDS_6(field_list, struct_name, field1, field2, field3, field4, field5, field6);                       \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field7)

#define _REGISTER_FIELDS_8(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8)    \
    _REGISTER_FIELDS_7(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7);               \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field8)

#define _REGISTER_FIELDS_9(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,    \
                           field9)                                                                                     \
    _REGISTER_FIELDS_8(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8);       \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field9)

#define _REGISTER_FIELDS_10(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10)                                                                           \
    _REGISTER_FIELDS_9(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,        \
                       field9);                                                                                        \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field10)

#define _REGISTER_FIELDS_11(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11)                                                                  \
    _REGISTER_FIELDS_10(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10);                                                                              \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field11)

#define _REGISTER_FIELDS_12(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12)                                                         \
    _REGISTER_FIELDS_11(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11);                                                                     \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field12)

#define _REGISTER_FIELDS_13(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13)                                                \
    _REGISTER_FIELDS_12(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12);                                                            \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field13)

#define _REGISTER_FIELDS_14(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14)                                       \
    _REGISTER_FIELDS_13(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13);                                                   \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field14)

#define _REGISTER_FIELDS_15(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15)                              \
    _REGISTER_FIELDS_14(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14);                                          \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field15)

#define _REGISTER_FIELDS_16(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16)                     \
    _REGISTER_FIELDS_15(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15);                                 \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field16)

#define _REGISTER_FIELDS_17(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17)            \
    _REGISTER_FIELDS_16(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16);                        \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field17)

#define _REGISTER_FIELDS_18(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18)   \
    _REGISTER_FIELDS_17(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17);               \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field18)

#define _REGISTER_FIELDS_19(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19)                                                                                   \
    _REGISTER_FIELDS_18(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18);      \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field19)

#define _REGISTER_FIELDS_20(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20)                                                                          \
    _REGISTER_FIELDS_19(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19);                                                                                      \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field20)

#define _REGISTER_FIELDS_21(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21)                                                                 \
    _REGISTER_FIELDS_20(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20);                                                                             \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field21)

#define _REGISTER_FIELDS_22(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22)                                                        \
    _REGISTER_FIELDS_21(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21);                                                                    \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field22)

#define _REGISTER_FIELDS_23(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23)                                               \
    _REGISTER_FIELDS_22(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22);                                                           \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field23)

#define _REGISTER_FIELDS_24(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24)                                      \
    _REGISTER_FIELDS_23(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23);                                                  \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field24)

#define _REGISTER_FIELDS_25(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24, field25)                             \
    _REGISTER_FIELDS_24(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23, field24);                                         \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field25)

#define _REGISTER_FIELDS_26(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24, field25, field26)                    \
    _REGISTER_FIELDS_25(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23, field24, field25);                                \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field26)

#define _REGISTER_FIELDS_27(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24, field25, field26, field27)           \
    _REGISTER_FIELDS_26(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23, field24, field25, field26);                       \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field27)

#define _REGISTER_FIELDS_28(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24, field25, field26, field27, field28)  \
    _REGISTER_FIELDS_27(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23, field24, field25, field26, field27);              \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field28)

#define _REGISTER_FIELDS_29(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24, field25, field26, field27, field28,  \
                            field29)                                                                                   \
    _REGISTER_FIELDS_28(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23, field24, field25, field26, field27, field28);     \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field29)

#define _REGISTER_FIELDS_30(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,   \
                            field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,   \
                            field19, field20, field21, field22, field23, field24, field25, field26, field27, field28,  \
                            field29, field30)                                                                          \
    _REGISTER_FIELDS_29(field_list, struct_name, field1, field2, field3, field4, field5, field6, field7, field8,       \
                        field9, field10, field11, field12, field13, field14, field15, field16, field17, field18,       \
                        field19, field20, field21, field22, field23, field24, field25, field26, field27, field28,      \
                        field29);                                                                                      \
    _REGISTER_FIELD_IMPL(field_list, struct_name, field30)

// parameter counting macros and intermediate macros
#define _GET_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21,     \
                   _22, _23, _24, _25, _26, _27, _28, _29, _30, N, ...)                                                \
    N
#define _COUNT_ARGS(...)                                                                                               \
    _GET_COUNT(__VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8,  \
               7, 6, 5, 4, 3, 2, 1, 0)

// intermediate macro for connecting macro name and parameter count
#define _REG_FIELDS_IMPL(N, field_list, struct_name, ...) _REGISTER_FIELDS_##N(field_list, struct_name, __VA_ARGS__)
#define _REG_FIELDS(N, field_list, struct_name, ...)      _REG_FIELDS_IMPL(N, field_list, struct_name, __VA_ARGS__)

// main field registration macro
#define REGISTER_FIELDS(field_list, struct_name, ...)                                                                  \
    _REG_FIELDS(_COUNT_ARGS(__VA_ARGS__), field_list, struct_name, __VA_ARGS__)

// define an auxiliary macro for properly handling TypeName
#define _REGISTER_STRUCT_IMPL(TypeName, ...)                                                                           \
    namespace jston {                                                                                                  \
    template <>                                                                                                        \
    void register_struct_metadata<TypeName>() {                                                                        \
        std::vector<field_metadata> fields;                                                                            \
        REGISTER_FIELDS(fields, TypeName, __VA_ARGS__);                                                                \
        register_metadata_helper<TypeName>(fields);                                                                    \
    }                                                                                                                  \
    static AutoRegistrar<TypeName> g_auto_registrar_##TypeName;                                                        \
    }  // namespace jston

#define register_json_struct(TypeName, ...) _REGISTER_STRUCT_IMPL(TypeName, __VA_ARGS__)

}  // namespace jston

#endif