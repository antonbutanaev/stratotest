#ifndef STRATOTEST_LOG_H_
#define STRATOTEST_LOG_H_

#include <type_traits>
#include <iostream>
#include <vector>
#include <stdexcept>

#define LOG(a) std::cout << a << std::endl
#define LOG0(a)
#define THROW(a) {std::stringstream s; s << a; throw std::runtime_error(s.str());}

struct Printable {};

template <typename T>
void printWrap(
	const T &t,
	const typename std::enable_if_t<!std::is_base_of_v<Printable, T>> * = nullptr
) {
	std::cout << t;
}

template <typename T>
void printWrap(
	const T &t,
	const typename std::enable_if_t<std::is_base_of_v<Printable, T>> * = nullptr
) {
	t.print();
}

template <typename T>
void printWrap(const std::vector<T> &v) {
	bool first = true;
	for(const auto &t: v) {
		if (!first)
			std::cout << '\t';
		first = false;
		printWrap(t);
	}
}

enum NoEOL_ {NoEOL};

inline void print(NoEOL_) {}

template <typename T>
void print(const T &t) {
	printWrap(t);
	std::cout << std::endl;
}

template <typename T, typename... Rest>
void print(const T &t, const Rest &...rest) {
	printWrap(t);
	std::cout << '\t';
	print(rest...);
}

#endif
