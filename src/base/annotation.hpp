#ifndef ANNOTATION_HPP
#define ANNOTATION_HPP

//�Զ�����get��set����
#define AUTO_GET_SET(x, y)                                     \
auto get##y() const {                                          \
     return x;                                                 \
}                                                              \
template<typename T>                                           \
void set##y(const T& t){                                       \
     x = t;                                                    \
}

//     using U = decltype(x) ;                                   \
//     static_assert(std::is_same_v<T, U>, "Invalid argument");  \

//��ֹ��������
#define NON_COPYABLE(className)                                \
className(const className&) = delete;                          \
className& operator=(const className&) = delete;

//��ֹ�ƶ�����
#define NON_MOVABLE(className)                                 \
className(className&&) = delete;                               \
className& operator=(className&&) = delete;


#endif //ANNOTATION_HPP