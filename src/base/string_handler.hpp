#ifndef STRING_HANDLER_HPP
#define STRING_HANDLER_HPP

#include<type_traits>
#include<string>
#include<vector>
#include<string_view>

namespace coral {

template<typename T>
inline void append_impl(std::string& sql, const T& str) {
    if constexpr (std::is_same_v<std::string, T> or
        std::is_same_v<std::string_view, T>) {
        if (str.empty())return;
    }
    else {
        if constexpr (sizeof(str) == 0)return;
    }

    sql += str;
    //sql += " ";
}

//�ɽ���ͬ���͵��ַ���ƴ����һ��
template<typename... Args>
inline void append(std::string& sql, Args &&... args) {
    (append_impl(sql, std::forward<Args>(args)), ...);
}

//�ַ����ָ�
//����ֵ:�ַ����� split(s:���ָ��ַ�, c:�ָ��־)
std::vector<std::string> split(const std::string& s, const std::string& c) {
	size_t pos1, pos2;
	std::vector<std::string>v;
	pos2 = s.find(c); //���Ը���KMP��д
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())v.push_back(s.substr(pos1));
	return v;
}

} //namespace coral

#endif STRING_HANDLER_HPP