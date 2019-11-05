#ifndef STRATOTEST_QUOTES_H_
#define STRATOTEST_QUOTES_H_

#include <map>
#include <date/date.h>

using Price = double;
using Ticker = std::string;
const Price NoPrice = 0.;
using Date = date::year_month_day;

class Quotes {
public:
	void parseQuotes(const Ticker &ticker);

	Price getQuote(const Ticker &, const Date &);

	static Quotes &get() {
		static Quotes quotes;
		return quotes;
	}
private:
	Quotes();
	std::map<Ticker, std::map<Date, Price>> m_quotes;
};

#endif
