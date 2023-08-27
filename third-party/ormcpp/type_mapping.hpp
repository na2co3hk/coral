#ifndef TYPE_MAPPING_H
#define TYPE_MAPPING_H

#include<type_traits>
#include<string>
#include<string_view>
#include<vector>
#include<array>

#include"mysql/mysql.h"

namespace manjusaka {
    template<typename T>
    struct identity {};

//C++类型到mysql参数类型标签的映射
#define REGISTER_TYPE(Type, Index)                                           \
  inline constexpr int type_to_id(identity<Type>) noexcept { return Index; } \
  inline constexpr auto id_to_type(                                          \
      std::integral_constant<std::size_t, Index>) noexcept {                 \
    Type res{};                                                              \
    return res;                                                              \
  }

REGISTER_TYPE(char, MYSQL_TYPE_TINY)
REGISTER_TYPE(short, MYSQL_TYPE_SHORT)
REGISTER_TYPE(int, MYSQL_TYPE_LONG)
REGISTER_TYPE(float, MYSQL_TYPE_FLOAT)
REGISTER_TYPE(double, MYSQL_TYPE_DOUBLE)
REGISTER_TYPE(int64_t, MYSQL_TYPE_LONGLONG)
inline int type_to_id(identity<std::string>) noexcept {
    return MYSQL_TYPE_VAR_STRING;
}
inline std::string id_to_type(
    std::integral_constant<std::size_t, MYSQL_TYPE_VAR_STRING>) noexcept {
    std::string res{};
    return res;
}

#define TINYINT char
#define SMALLINT short
#define INTEGER int
#define FLOAT float
#define DOUBLE double
#define BIGINT int64_t
#define BLOB std::vector<char>
#define TEXT std::string
#define VARCHAR(size) std::array<char, size>  //TODO:以后替换成自定义缓冲区类

    using blob = std::vector<char>;

    //类型到SQL类型的映射
    inline constexpr std::string_view type_to_name(identity<char>) noexcept {
        return "TINYINT";
    }

    inline constexpr std::string_view type_to_name(identity<short>) noexcept {
        return "SMALLINT";
    }

    inline constexpr std::string_view type_to_name(identity<int>) noexcept {
        return "INTEGER";
    }

    inline constexpr std::string_view type_to_name(identity<float>) noexcept {
        return "FLOAT";
    }

    inline constexpr std::string_view type_to_name(identity<double>) noexcept {
        return "DOUBLE";
    }

    inline constexpr std::string_view type_to_name(identity<int64_t>) noexcept {
        return "BIGINT";
    }

    inline constexpr std::string_view type_to_name(identity<blob>) noexcept { return "BLOB"; }

    inline std::string_view type_to_name(identity<std::string>) noexcept { return "TEXT"; }

    template<size_t N>
    inline auto type_to_name(identity<std::array<char, N>>) noexcept {
        std::string s = "varchar(" + std::to_string(N) + ")";
        return s;
    }
}
#endif //TYPE_MAPPING_H
