#ifndef STRATOTEST_LOG_H_
#define STRATOTEST_LOG_H_

#include <iostream>
#include <stdexcept>

#define LOG(a) std::cout << a << std::endl
#define LOG0(a)
#define THROW(a) {std::stringstream s; s << a; throw std::runtime_error(s.str());}

#endif
