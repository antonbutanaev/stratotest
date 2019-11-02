#include <iostream>
#include <chrono>
#include <vector>
#include <date/date.h>

using Time = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
using Price = double;

struct Quote {
	Time time;
	Price price;
};

using Quotes = std::vector<Quote>;

int main() {
}
