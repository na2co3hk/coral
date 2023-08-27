#ifndef OPERATION_H
#define OPERATION_H

#include<optional>
#include<string>
#include "reflection.hpp"

namespace manjusaka {

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
        sql += " ";
    }

    //�ɽ���ͬ���͵��ַ���ƴ����һ��
    template<typename... Args>
    inline void append(std::string& sql, Args &&... args) {
        (append_impl(sql, std::forward<Args>(args)), ...);
    }

    //������+���ţ��������ж�һ���Ƿ��Ƿ�������
    //��mysql�мӲ��Ӷ�ûӰ��
    template<typename T,typename = std::enable_if_t<manjusaka::is_reflection_v<T>>>
    inline std::string get_name() {
        std::string quota_name = "`" + std::string(T::TABLE_NAME()) + "`";
        return quota_name;
    }

    template<typename T>
    inline std::string generate_insert_sql() {
        std::string sql = "insert into ";
        //insert into Person ( id, name, age ) values( ?, ?, ?);
        constexpr size_t size = T::field_count;
        std::string table_name = get_name<T>();
        constexpr std::string_view fields = T::field_list;
        append(sql, table_name.data(), "(", fields.data(), ")", "values(");

        for (size_t i = 0; i < size; i++) {
            sql += "?";
            if (i < size - 1) {
                sql += ", ";
            }
            else {
                sql += ");";
            }
        }
        return sql;
    }

    template<typename T>
    inline std::string generate_delete_sql(const std::string& where_condition = "") {
        std::string sql = "delete from ";
        std::string table_name = get_name<T>();
        append(sql, table_name.data());
        if (!where_condition.empty()) {
            append(sql, "where", where_condition);
        }
        return sql;
    }

    /*
     * ��ֹ��where, ����order by������Ҫ������condition����һ��
     * ע�⴫��˳���ȴ��������ٴ���ѯ�ֶ�
     * */
    template<typename T>
    inline std::string generate_select_sql(const std::string& select_condition, const std::string& select_fields = "*") {
        std::string sql = "select ";
        std::string table_name = get_name<T>();
        append(sql, select_fields, "from", table_name);
        if (!select_condition.empty()) {
            append(sql, select_condition);
        }
        return sql;
    }

    template<typename T>
    inline std::string generate_update_sql(const std::string& set_field, const std::string& where_condition = "") {
        std::string sql = "update ";
        std::string table_name = get_name<T>();
        append(sql, table_name, "set", set_field);
        if (!where_condition.empty()) {
            append(sql, "where", where_condition);
        }
        return sql;
    }

    template<typename T>
    inline constexpr auto to_str(T&& t) {
        if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
            return std::to_string(std::forward<T>(t));
        }
        else {
            return std::string("'") + t + std::string("'");
        }
    }

    inline void get_str(std::string& sql, const std::string& s) {
        auto pos = sql.find_first_of('?');
        sql.replace(pos, 1, " ");
        sql.insert(pos, s);
    }

    /*template<typename... Args>
    inline std::string get_sql(const std::string& s, Args &&... args) {
        auto sql = s;
        (get_str(sql, to_str(args)), ...);
        return sql;
    }*/
}

#endif //OPERATION_H
