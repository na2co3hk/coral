#ifndef REFLECTION_H
#define REFLECTION_H

#include<type_traits>
#include<string_view>
#include<string>
#include<optional>
#include<array>
#include<tuple>

namespace manjusaka {
#define GET_NTH_ARG(                                                                        \
    _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9,  _10, _11, _12, _13, _14, _15, _16,         \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32,         \
    _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48,         \
    _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, n, ...) n

#define GET_ARG_COUNT(...) GET_NTH_ARG(__VA_ARGS__,                     \
        64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
        48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, \
        32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
        16, 15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1)

    //直接拼接会导致错误，还需要包装一下
#define CONCAT(A, B) CONCAT1(A, B)
#define CONCAT1(A, B) A##_##B

#define STR(str) #str

#define REPEAT_0(func, i, arg)
#define REPEAT_1(func, i, arg)        func(i, arg)
#define REPEAT_2(func, i, arg, ...)   func(i, arg) REPEAT_1(func, i + 1, __VA_ARGS__)
#define REPEAT_3(func, i, arg, ...)   func(i, arg) REPEAT_2(func, i + 1, __VA_ARGS__)
#define REPEAT_4(func, i, arg, ...)   func(i, arg) REPEAT_3(func, i + 1, __VA_ARGS__)
#define REPEAT_5(func, i, arg, ...)   func(i, arg) REPEAT_4(func, i + 1, __VA_ARGS__)
#define REPEAT_6(func, i, arg, ...)   func(i, arg) REPEAT_5(func, i + 1, __VA_ARGS__)
#define REPEAT_7(func, i, arg, ...)   func(i, arg) REPEAT_6(func, i + 1, __VA_ARGS__)
#define REPEAT_8(func, i, arg, ...)   func(i, arg) REPEAT_7(func, i + 1, __VA_ARGS__)
#define REPEAT_9(func, i, arg, ...)   func(i, arg) REPEAT_8(func, i + 1, __VA_ARGS__)
#define REPEAT_10(func, i, arg, ...)  func(i, arg) REPEAT_9(func, i + 1, __VA_ARGS__)
#define REPEAT_11(func, i, arg, ...)  func(i, arg) REPEAT_10(func, i + 1, __VA_ARGS__)
#define REPEAT_12(func, i, arg, ...)  func(i, arg) REPEAT_11(func, i + 1, __VA_ARGS__)
#define REPEAT_13(func, i, arg, ...)  func(i, arg) REPEAT_12(func, i + 1, __VA_ARGS__)
#define REPEAT_14(func, i, arg, ...)  func(i, arg) REPEAT_13(func, i + 1, __VA_ARGS__)
#define REPEAT_15(func, i, arg, ...)  func(i, arg) REPEAT_14(func, i + 1, __VA_ARGS__)
#define REPEAT_16(func, i, arg, ...)  func(i, arg) REPEAT_15(func, i + 1, __VA_ARGS__)
#define REPEAT_17(func, i, arg, ...)  func(i, arg) REPEAT_16(func, i + 1, __VA_ARGS__)
#define REPEAT_18(func, i, arg, ...)  func(i, arg) REPEAT_17(func, i + 1, __VA_ARGS__)
#define REPEAT_19(func, i, arg, ...)  func(i, arg) REPEAT_18(func, i + 1, __VA_ARGS__)
#define REPEAT_20(func, i, arg, ...)  func(i, arg) REPEAT_19(func, i + 1, __VA_ARGS__)
#define REPEAT_21(func, i, arg, ...)  func(i, arg) REPEAT_20(func, i + 1, __VA_ARGS__)
#define REPEAT_22(func, i, arg, ...)  func(i, arg) REPEAT_21(func, i + 1, __VA_ARGS__)
#define REPEAT_23(func, i, arg, ...)  func(i, arg) REPEAT_22(func, i + 1, __VA_ARGS__)
#define REPEAT_24(func, i, arg, ...)  func(i, arg) REPEAT_23(func, i + 1, __VA_ARGS__)
#define REPEAT_25(func, i, arg, ...)  func(i, arg) REPEAT_24(func, i + 1, __VA_ARGS__)
#define REPEAT_26(func, i, arg, ...)  func(i, arg) REPEAT_25(func, i + 1, __VA_ARGS__)
#define REPEAT_27(func, i, arg, ...)  func(i, arg) REPEAT_26(func, i + 1, __VA_ARGS__)
#define REPEAT_28(func, i, arg, ...)  func(i, arg) REPEAT_27(func, i + 1, __VA_ARGS__)
#define REPEAT_29(func, i, arg, ...)  func(i, arg) REPEAT_28(func, i + 1, __VA_ARGS__)
#define REPEAT_30(func, i, arg, ...)  func(i, arg) REPEAT_29(func, i + 1, __VA_ARGS__)
#define REPEAT_31(func, i, arg, ...)  func(i, arg) REPEAT_30(func, i + 1, __VA_ARGS__)
#define REPEAT_32(func, i, arg, ...)  func(i, arg) REPEAT_31(func, i + 1, __VA_ARGS__)
#define REPEAT_33(func, i, arg, ...)  func(i, arg) REPEAT_32(func, i + 1, __VA_ARGS__)
#define REPEAT_34(func, i, arg, ...)  func(i, arg) REPEAT_33(func, i + 1, __VA_ARGS__)
#define REPEAT_35(func, i, arg, ...)  func(i, arg) REPEAT_34(func, i + 1, __VA_ARGS__)
#define REPEAT_36(func, i, arg, ...)  func(i, arg) REPEAT_35(func, i + 1, __VA_ARGS__)
#define REPEAT_37(func, i, arg, ...)  func(i, arg) REPEAT_36(func, i + 1, __VA_ARGS__)
#define REPEAT_38(func, i, arg, ...)  func(i, arg) REPEAT_37(func, i + 1, __VA_ARGS__)
#define REPEAT_39(func, i, arg, ...)  func(i, arg) REPEAT_38(func, i + 1, __VA_ARGS__)
#define REPEAT_40(func, i, arg, ...)  func(i, arg) REPEAT_39(func, i + 1, __VA_ARGS__)
#define REPEAT_41(func, i, arg, ...)  func(i, arg) REPEAT_40(func, i + 1, __VA_ARGS__)
#define REPEAT_42(func, i, arg, ...)  func(i, arg) REPEAT_41(func, i + 1, __VA_ARGS__)
#define REPEAT_43(func, i, arg, ...)  func(i, arg) REPEAT_42(func, i + 1, __VA_ARGS__)
#define REPEAT_44(func, i, arg, ...)  func(i, arg) REPEAT_43(func, i + 1, __VA_ARGS__)
#define REPEAT_45(func, i, arg, ...)  func(i, arg) REPEAT_44(func, i + 1, __VA_ARGS__)
#define REPEAT_46(func, i, arg, ...)  func(i, arg) REPEAT_45(func, i + 1, __VA_ARGS__)
#define REPEAT_47(func, i, arg, ...)  func(i, arg) REPEAT_46(func, i + 1, __VA_ARGS__)
#define REPEAT_48(func, i, arg, ...)  func(i, arg) REPEAT_47(func, i + 1, __VA_ARGS__)
#define REPEAT_49(func, i, arg, ...)  func(i, arg) REPEAT_48(func, i + 1, __VA_ARGS__)
#define REPEAT_50(func, i, arg, ...)  func(i, arg) REPEAT_49(func, i + 1, __VA_ARGS__)
#define REPEAT_51(func, i, arg, ...)  func(i, arg) REPEAT_50(func, i + 1, __VA_ARGS__)
#define REPEAT_52(func, i, arg, ...)  func(i, arg) REPEAT_51(func, i + 1, __VA_ARGS__)
#define REPEAT_53(func, i, arg, ...)  func(i, arg) REPEAT_52(func, i + 1, __VA_ARGS__)
#define REPEAT_54(func, i, arg, ...)  func(i, arg) REPEAT_53(func, i + 1, __VA_ARGS__)
#define REPEAT_55(func, i, arg, ...)  func(i, arg) REPEAT_54(func, i + 1, __VA_ARGS__)
#define REPEAT_56(func, i, arg, ...)  func(i, arg) REPEAT_55(func, i + 1, __VA_ARGS__)
#define REPEAT_57(func, i, arg, ...)  func(i, arg) REPEAT_56(func, i + 1, __VA_ARGS__)
#define REPEAT_58(func, i, arg, ...)  func(i, arg) REPEAT_57(func, i + 1, __VA_ARGS__)
#define REPEAT_59(func, i, arg, ...)  func(i, arg) REPEAT_58(func, i + 1, __VA_ARGS__)
#define REPEAT_60(func, i, arg, ...)  func(i, arg) REPEAT_59(func, i + 1, __VA_ARGS__)
#define REPEAT_61(func, i, arg, ...)  func(i, arg) REPEAT_60(func, i + 1, __VA_ARGS__)
#define REPEAT_62(func, i, arg, ...)  func(i, arg) REPEAT_61(func, i + 1, __VA_ARGS__)
#define REPEAT_63(func, i, arg, ...)  func(i, arg) REPEAT_62(func, i + 1, __VA_ARGS__)
#define REPEAT_64(func, i, arg, ...)  func(i, arg) REPEAT_63(func, i + 1, __VA_ARGS__)


//生成参数列表
#define MAKE_LIST(...) #__VA_ARGS__,

//PS: obj.arg一定要加括号
#define DEFINE_FIELD(i, arg)                 \
template<typename T>                         \
struct FIELD<T, i>{                          \
    T& obj;                                  \
    FIELD(T& obj): obj(obj) {}               \
    auto value() -> decltype(auto) {         \
        return (obj.arg);                    \
    }                                        \
    static constexpr const char* name(){     \
        return STR(arg);                     \
    }                                        \
};

#define DEFINE_TABLE(table_name, ...)                                          \
static constexpr const char* TABLE_NAME(){ return STR(table_name); }           \
template<typename, size_t>                                                     \
struct FIELD;                                                                  \
static constexpr size_t field_count = GET_ARG_COUNT(__VA_ARGS__);              \
CONCAT(REPEAT, GET_ARG_COUNT(__VA_ARGS__))(DEFINE_FIELD, 0, __VA_ARGS__)       \
static constexpr std::string_view field_list = { MAKE_LIST(__VA_ARGS__) };

    template<typename T, typename = void>
    struct is_reflection : std::false_type {};

    template<typename T>
    struct is_reflection<T, std::void_t<decltype(&T::field_count)>> : std::true_type {};

    template<typename T>
    static constexpr bool is_reflection_v = is_reflection<T>::value;

    template<typename T>
    struct is_optional : std::false_type {};

    template<typename T>
    struct is_optional<std::optional<T>> : std::true_type {};

    template<typename T>
    static constexpr bool is_optional_v = is_optional<T>::value;

    template <class T>
    constexpr bool is_char_array_v = std::is_array_v<T>
        && std::is_same_v<char, std::remove_pointer_t<std::decay_t<T>>>;

    //U是一个泛型
    template<template <typename...> class U, typename T>
    struct is_template_instant : std::false_type {};

    //可用于判断某个泛型是否是某个类型，例如：tuple<int, short> 是否是tuple， vector<int> 是否是vector
    template<template <typename...> class U, typename... args>
    struct is_template_instant<U, U<args...>> : std::true_type {};

    template<typename T>
    struct is_tuple : is_template_instant<std::tuple, T> {};

    template<typename T>
    static constexpr bool is_tuple_v = is_tuple<T>::value;

    /*
     * f的参数：
     * 1.const char* 字段名
     * 2.任意类型 字段的具体值
     * 将类中的每个字段都带入到f中
     * */
    /*template<typename T, typename F, size_t... Is>
    inline constexpr void forEach(T&& obj, F&& f, std::index_sequence<Is...>) {
        using TDECAY = std::decay_t<T>;
        (void(f(typename TDECAY::template FIELD<T, Is>(std::forward<T>(obj)).name(),
            typename TDECAY::template FIELD<T, Is>(std::forward<T>(obj)).value())), ...);
    }

    template<typename T, typename F>
    inline constexpr void forEach(T&& obj, F&& f) {
        forEach(std::forward<T>(obj),
            std::forward<F>(f),
            std::make_index_sequence<std::decay_t<T>::field_count>{});
    }*/
    template<typename T, typename F, size_t... Is>
    inline constexpr void forEach(T&& obj, F&& f, std::index_sequence<Is...>) {
        using TDECAY = std::decay_t<T>;
        using U = std::remove_reference_t<T>;
        if constexpr (is_tuple_v<U>) {
            (f(std::get<Is>(std::forward<T>(obj))), ...);
        }
        else {
            (void(f(typename TDECAY::template FIELD<T, Is>(std::forward<T>(obj)).name(),
                typename TDECAY::template FIELD<T, Is>(std::forward<T>(obj)).value())), ...);
        }
    }

    template<typename T, typename F>
    inline constexpr void forEach(T&& obj, F&& f) {
        using U = std::remove_reference_t<T>;
        if constexpr (is_tuple_v<U>) {
            forEach(std::forward<T>(obj),
                std::forward<F>(f),
                std::make_index_sequence<std::tuple_size_v<U>>{});
        }
        else {
            forEach(std::forward<T>(obj),
                std::forward<F>(f),
                std::make_index_sequence<std::decay_t<T>::field_count>{});
        }
    }

    template<typename T>
    void serializeObj(std::ostream& out, const T& obj,
        const char* fieldName = "", int depth = 0) {
        auto indent = [&] {
            for (int i = 0; i < depth; ++i)
            {
                out << "    ";
            }
        };

        if constexpr (is_reflection_v<T>) {
            indent();
            out << fieldName << (*fieldName ? ": {" : "{") << std::endl;
            forEach(obj,
                [&](auto&& fieldName, auto&& value)
                { serializeObj(out, value, fieldName, depth + 1); });
            indent();
            out << "}" << std::endl;
        }
        else {
            indent();
            out << fieldName << ": " << obj << std::endl;
        }
    }

    template<typename T>
    void deserializeObj(std::istream& in, T& obj,
        const char* fieldName = "") {
        if constexpr (is_reflection_v<T>) {
            std::string token;
            in >> token; // eat '{'
            if (*fieldName) {
                in >> token; // WARNING: needs check fieldName valid
            }

            forEach(obj,
                [&](auto&& fieldName, auto&& value)
                { deserializeObj(in, value, fieldName); });

            in >> token; // eat '}'
        }
        else {
            if (*fieldName) {
                std::string token;
                in >> token; // WARNING: needs check fieldName valid
            }
            in >> obj; // dump value
        }
    }
}


#endif //REFLECTION_H