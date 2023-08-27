#ifndef MYSQL_H
#define MYSQL_H

#include<string>
#include<functional>
#include<tuple>
#include<map>
#include<array>
#include<string.h>
#include<chrono>
#include<mysql/mysql.h>

#include"operation.hpp"
#include"reflection.hpp"
#include"type_mapping.hpp"

using blob = manjusaka::blob;

class mysql {

public:
	template<typename... Args>
	bool connect(Args &&...args) {
		if (con_ != nullptr) {
			mysql_close(con_);
		}

		con_ = mysql_init(nullptr);
		if (con_ == nullptr) {
			return false;
		}

		int timeout = -1;
		auto tp = get_tuple(timeout , std::forward<Args>(args)...);

		if (timeout > 0) {
			if (mysql_options(con_ , MYSQL_OPT_CONNECT_TIMEOUT , &timeout) != 0) {
				return false;
			}
		}

		char value = 1;
		mysql_options(con_ , MYSQL_OPT_RECONNECT , &value);
		mysql_options(con_ , MYSQL_SET_CHARSET_NAME , "utf8");

		if (std::apply(&mysql_real_connect , tp) == nullptr) {
			return false;
		}

		return true;
	}

	template<typename... Args>
	bool disconnect(Args &&...args) {
		if (con_ != nullptr) {
			mysql_close(con_);
			con_ = nullptr;
		}
		return true;
	}

	bool ping() { return mysql_ping(con_) == 0; }

	//返回插入成功数据的个数
	template<typename T>
	int insert(const T& t) {
		std::string sql = manjusaka::generate_insert_sql<T>();
		/*manjusaka::forEach(t, [&](auto&& fieldname, auto&& value) {
			manjusaka::get_str(sql, manjusaka::to_str(value));
		});*/

		stmt_ = mysql_stmt_init(con_);
		if (!stmt_) {
			return -1;
		}

		if (mysql_stmt_prepare(stmt_ , sql.c_str() , (int) sql.size())) {
			return -1;
		}

		auto guard = guard_statement(stmt_);

		if (stmt_execute(t) < 0) {
			return -1;
		}

		return 1;
	}

	//批量插入
	template<typename T>
	int insert(const std::vector<T>& t) {
		std::string sql = manjusaka::generate_insert_sql<T>();

		stmt_ = mysql_stmt_init(con_);
		if (!stmt_) {
			return -1;
		}

		if (mysql_stmt_prepare(stmt_ , sql.c_str() , (int) sql.size())) {
			return -1;
		}

		auto guard = guard_statement(stmt_);

		//原子操作，保证批量操作都完成
		bool b = begin();
		if (!b) {
			return -1;
		}

		for (auto& item : t) {
			int r = stmt_execute(item);
			if (r == -1) {
				rollback();
				return -1;
			}
		}
		b = commit();

		return b ? (int) t.size() : -1;
	}

	//映射对象版本
	//返回对象的数组
	template<typename T , typename... Args>
	std::enable_if_t<manjusaka::is_reflection_v<T> , std::vector<T>> query(Args &&...args) {
		std::string condition = "";
		manjusaka::append(condition , std::forward<Args>(args)...);
		std::string sql = manjusaka::generate_select_sql<T>(condition);
		constexpr size_t size = T::field_count;

		stmt_ = mysql_stmt_init(con_);
		if (!stmt_) {
			return {};
		}

		auto guard = guard_statement(stmt_);
		/*test
		std::cout << sql << std::endl;*/

		if (mysql_stmt_prepare(stmt_ , sql.c_str() , (unsigned long) sql.size())) {
			return {};
		}

		std::array<MYSQL_BIND , size> param_binds = {};
		std::map<size_t , std::vector<char>> mp;

		std::vector<T>v; //返回对象
		T t{};
		int index = 0;
		manjusaka::forEach(t , [ & ] (auto&& fieldName , auto&& value) {
			set_param_bind(param_binds [ index ] , value , index , mp);
			index++;
		});

		if (index == 0) {
			return {};
		}

		if (mysql_stmt_bind_result(stmt_ , &param_binds [ 0 ])) {
			return {};
		}

		if (mysql_stmt_execute(stmt_)) {
			return {};
		}

		//匹配结果
		while (mysql_stmt_fetch(stmt_) == 0) {
			index = 0;
			manjusaka::forEach(t , [ & ] (auto&& fieldName , auto&& value) {
				set_value(param_binds [ index ] , value , index , mp);
				index++;

			});

			for (auto& p : mp) {
				p.second.assign(p.second.size() , 0);
			}

			v.push_back(std::move(t));
			manjusaka::forEach(t , [ & ] (auto&& fieldName , auto&& value) {
				using U = std::remove_reference_t<decltype(value)>;
				if constexpr (std::is_arithmetic_v<U>) {
					memset(&value , 0 , sizeof(U));
				}
			});
		}
		return v;
	}

	//指定字段版本
	//返回tuple
	template<typename T>
	std::enable_if_t<!manjusaka::is_reflection_v<T> , std::vector<T>> query(const std::string& sql) {
		static_assert(manjusaka::is_tuple_v<T>);
		const auto size = std::tuple_size_v<T>;

		stmt_ = mysql_stmt_init(con_);
		if (!stmt_) {
			return {};
		}

		auto guard = guard_statement(stmt_);

		if (mysql_stmt_prepare(stmt_ , sql.c_str() , (unsigned long) sql.size())) {
			return {};
		}

		std::array<MYSQL_BIND , size> param_binds = {};
		std::map<size_t , std::vector<char>> mp;

		std::vector<T> v;
		T tp{};
		int index = 0;
		manjusaka::forEach(tp , [ & ] (auto&& value) {
			set_param_bind(param_binds [ index ] , value , index , mp);
			index++;
		});


		if (index == 0) {
			return {};
		}

		if (mysql_stmt_bind_result(stmt_ , &param_binds [ 0 ])) {
			return {};
		}

		if (mysql_stmt_execute(stmt_)) {
			return {};
		}

		while (mysql_stmt_fetch(stmt_) == 0) {
			index = 0;
			manjusaka::forEach(tp , [ & ] (auto&& value) {
				set_value(param_binds [ index ] , value , index , mp);
				index++;
			});

			for (auto& p : mp) {
				p.second.assign(p.second.size() , 0);
			}

			v.push_back(std::move(tp));
		}
		return v;
	}

	template<typename T , typename... Args>
	bool delete_records(Args &&... args) {
		std::string condition = "";
		manjusaka::append(condition , std::forward<Args>(args)...);
		std::string sql = manjusaka::generate_delete_sql<T>(condition);

		if (mysql_query(con_ , sql.data())) {
			//record
			return false;
		}
		return true;
	}

	bool execute(const std::string& sql) {
		if (mysql_query(con_ , sql.data()) != 0) {
			return false;
		}
		return true;
	}

	// transaction
	bool begin() {
		if (mysql_query(con_ , "BEGIN")) {
			return false;
		}

		return true;
	}

	bool commit() {
		if (mysql_query(con_ , "COMMIT")) {
			return false;
		}

		return true;
	}

	bool rollback() {
		if (mysql_query(con_ , "ROLLBACK")) {
			return false;
		}

		return true;
	}

	void refreshAliveTime() {
		aliveTime_ = std::chrono::system_clock::now();
	}

	long long getAliveTime() {
		auto n = std::chrono::system_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::milliseconds>(n - aliveTime_);
		return d.count();
	}

private:
	/*
	* 用于查询语句时绑定参数
	* param_binds ： 传出参数
	* value : 字段的值
	* i : 绑定参数在param_binds中的索引
	* mp : 用于将索引i和param_binds中的缓冲区绑定,同时也是传出参数，可以用于绑定结果
	*/
	template<typename T>
	void set_param_bind(MYSQL_BIND& param_bind , T&& value , int i ,
		std::map<size_t , std::vector<char>>& mp) {
		using U = std::remove_const_t<std::remove_reference_t<T>>;
		if constexpr (manjusaka::is_optional_v<U>) {
			return set_bind_bind(param_bind , *value , i , mp);
		}
		else if constexpr (std::is_arithmetic_v<U>) {
			param_bind.buffer_type = (enum_field_types) manjusaka::type_to_id(manjusaka::identity<U>{});
			param_bind.buffer = const_cast<void*>(static_cast<const void*>(&value));
		}
		else if constexpr (std::is_same_v<std::string , U>) {
			param_bind.buffer_type = MYSQL_TYPE_STRING;
			std::vector<char> tmp(65536 , 0);
			mp.emplace(i , std::move(tmp));
			param_bind.buffer = &(mp.rbegin()->second [ 0 ]);
			param_bind.buffer_length = 65536;
		}
		else if constexpr (manjusaka::is_char_array_v<U>) {
			param_bind.buffer_type = MYSQL_TYPE_VAR_STRING;
			std::vector<char> tmp(sizeof(U) , 0);
			mp.emplace(i , std::move(tmp));
			param_bind.buffer = &(mp.rbegin()->second [ 0 ]);
			param_bind.buffer_length = (unsigned long) sizeof(U);
		}
		else if constexpr (std::is_same_v<manjusaka::blob , U>) {
			param_bind.buffer_type = MYSQL_TYPE_BLOB;
			std::vector<char> tmp(65536 , 0);
			mp.emplace(i , std::move(tmp));
			param_bind.buffer = &(mp.rbegin()->second [ 0 ]);
			param_bind.buffer_length = 65536;
		}
	}



	/*
	* 设置查询结果的参数
	* 传入参数和上面一样
	*/

	template<typename T>
	void set_value(MYSQL_BIND& param_bind , T&& value , int i ,
		std::map<size_t , std::vector<char>>& mp) {
		using U = std::remove_const_t<std::remove_reference_t<T>>;
		if constexpr (manjusaka::is_optional_v<U>) {
			using value_type = typename U::value_type;
			if constexpr (std::is_arithmetic_v<value_type>) {
				value_type item;
				memcpy(&item , param_bind.buffer , sizeof(value_type));
				value = std::move(item);
			}
			else {
				value_type item;
				value = std::move(item);
				return set_value(param_bind , *value , i , mp);
			}
		}
		else if constexpr (std::is_same_v<std::string , U>) {
			auto& vec = mp [ i ]; //缓冲区
			value = std::string(&vec [ 0 ] , strlen(vec.data()));
		}
		else if constexpr (manjusaka::is_char_array_v<U>) {
			auto& vec = mp [ i ];
			memcpy(value , vec.data() , vec.size());
		}
		else if constexpr (std::is_same_v<manjusaka::blob , U>) {
			auto& vec = mp [ i ];
			value = blob(vec.data() , vec.data() + get_blob_len(i));
		}
	}

	/*
	* 用于插入语句时绑定参数
	* param_binds : 传出参数
	* value ：字段的值
	*/
	template<typename T>
	constexpr void set_param_bind(std::vector<MYSQL_BIND>& param_binds , T&& value) {
		MYSQL_BIND param = {};
		using U = std::remove_const_t<std::remove_reference_t<T>>;

		if constexpr (manjusaka::is_optional_v<U>) { //判断是否是optional
			if (value.has_value()) {
				return set_param_bind(param_binds , std::move(value.value()));
			}
			else {
				param.buffer_type = MYSQL_TYPE_NULL;
			}
		}
		else if constexpr (std::is_arithmetic_v<U>) { //判断是否是算数类型，即整型那些
			param.buffer_type = (enum_field_types) manjusaka::type_to_id(manjusaka::identity<U>{});
			param.buffer = const_cast<void*>(static_cast<const void*>(&value));
		}
		else if constexpr (std::is_same_v<std::string , U>) { //是否为字符串类型
			param.buffer_type = MYSQL_TYPE_STRING;
			param.buffer = (void*) (value.c_str());
			param.buffer_length = (unsigned long) value.size();
		}
		else if constexpr (std::is_same_v<const char* , U> or manjusaka::is_char_array_v<U>) { //是否为array
			param.buffer_type = MYSQL_TYPE_VARCHAR; //改
			param.buffer = (void*) (value);
			param.buffer_length = (unsigned long) strlen(value);
		}
		else if constexpr (std::is_same_v<manjusaka::blob , U>) { //是否为vector<char>
			param.buffer_type = MYSQL_TYPE_BLOB;
			param.buffer = (void*) (value.data());
			param.buffer_length = (unsigned long) value.size();
		}
		else {
			//类型还未支持
		}

		param_binds.push_back(param);
	}

	//执行插入
	template<typename T>
	int stmt_execute(const T& t) {
		std::vector<MYSQL_BIND>param_binds;

		manjusaka::forEach(t , [ & ] (auto&& fieldName , auto&& value) {
			set_param_bind(param_binds , value);
		});

		if (mysql_stmt_bind_param(stmt_ , &param_binds [ 0 ])) {
			return -1;
		}

		if (mysql_stmt_execute(stmt_)) {
			return -1;
		}

		int count = (int) mysql_stmt_affected_rows(stmt_);
		if (count == 0) {
			return -1;
		}

		return count;
	}

	struct guard_statement {
		guard_statement(MYSQL_STMT* stmt) :stmt_(stmt) {};
		~guard_statement() {
			if (stmt_ != nullptr) {
				status_ = mysql_stmt_close(stmt_); //status有值代表出错了
			}

			//TODO:在日志记录错误信息	if (status_) LOG_ERROR << ...
		}

		int status_{ 0 };
		MYSQL_STMT* stmt_{ nullptr };
	};


	int get_blob_len(int col) {
		unsigned long data_len = 0;

		MYSQL_BIND param;
		memset(&param , 0 , sizeof(MYSQL_BIND));
		param.length = &data_len;
		param.buffer_type = MYSQL_TYPE_BLOB;

		auto retcode = mysql_stmt_fetch_column(stmt_ , &param , col , 0);
		if (retcode != 0) {
			//error
			return 0;
		}
		return static_cast<int>(data_len);
	}

	//将可变参数展开为tuple
	template<typename... Args>
	auto get_tuple(int& timeout , Args &&...args) {
		auto tp = std::make_tuple(con_ , std::forward<Args>(args)...);
		if constexpr (sizeof...(Args) == 5) {
			auto [con , ip , usr , pwd , dbn , to] = tp;
			timeout = to;
			return std::make_tuple(con , ip , usr , pwd , dbn , 0 , nullptr , 0);
		}
		else if constexpr (sizeof...(Args) == 6) {
			auto [con , ip , usr , pwd , dbn , to , port] = tp;
			timeout = to;
			return std::make_tuple(con , ip , usr , pwd , dbn , port , nullptr , 0);
		}
		else if constexpr (sizeof...(Args) == 4) {
			return std::tuple_cat(tp , std::make_tuple(0 , nullptr , 0));
		}
	}

	MYSQL* con_{ nullptr };
	MYSQL_STMT* stmt_{ nullptr }; //使用预处理加快速度
	std::chrono::system_clock::time_point aliveTime_{ std::chrono::system_clock::now() };
	bool has_error_{ false };
};

#endif //MYSQL_H