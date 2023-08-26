#ifndef ANNOTATION_HPP
#define ANNOTATION_HPP

//自动生成get和set方法
#define AUTO_GET_SET(x, y)                                     \
auto get##y() const {                                          \
     return x;                                                 \
}                                                              \
template<typename T>                                           \
void set##y(const T& t){                                       \
     using U = decltype(x) ;                                   \
     static_assert(std::is_same_v<T, U>, "Invalid argument");  \
     x = t;                                                    \
}

//禁止拷贝构造
#define NON_COPYABLE(className)                                \
className(const className&) = delete;                          \
className& operator=(const className&) = delete;

//禁止移动构造
#define NON_MOVABLE(className)                                 \
className(className&&) = delete;                               \
className& operator=(className&&) = delete;


#endif //ANNOTATION_HPP