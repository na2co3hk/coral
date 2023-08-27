#ifndef HORIZON_H
#define HORIZON_H

#include"mysql.hpp"
#include"operation.hpp"
#include"reflection.hpp"
#include"type_mapping.hpp"
#include"connection_pool.hpp"

template<typename DB>
using ormcpp = manjusaka::connection_pool<DB>;

#endif //HORIZON_H