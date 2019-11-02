#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <date/date.h>

#define LOG(a) std::cout << a << std::endl
#define LOG0(a)

using Time = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
using Price = double;

struct Quote {
	Time time;
	Price price;
};

using Quotes = std::vector<Quote>;

using namespace std;
using namespace date;

Quotes parseQuotes(const std::string &file) {
	Quotes quotes;
	ifstream in(file);
	string header;
	getline(in, header);
	if (header != "Date,Open,High,Low,Close,Adj Close,Volume")
		throw std::runtime_error("Wrong quote file format");

	for (;;) {
		int yi, mi, di;
		char dash;
		in >> yi >> dash >> mi >> dash >> di;
		if (!in)
			break;

		auto ymd = year{yi}/mi/di;

		LOG0(yi << ':' << mi << ':' << di << ' ');

		Quote quote;
		quote.time = sys_days{ymd};

		char comma;
		Price open, high, low, close, adjClose;
		long volume;
		in >> comma
			>> open >> comma
			>> high >> comma
			>> low >> comma
			>> close >> comma
			>> adjClose >> comma
			>> volume;

		LOG0(
			open << ':'
			<< high << ':'
			<< low << ':'
			<< close << ':'
			<< adjClose << ':'
			<< volume
		);

		quote.price = adjClose;
		quotes.push_back(quote);
	}

	return quotes;
}

int main() try {
	const auto bnd = parseQuotes("/home/anton/temp/quotes/BND.csv");
	cout << bnd.size() << endl;
	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
