#ifndef STRATOTEST_PORTFOLIOANALYZER_H_
#define STRATOTEST_PORTFOLIOANALYZER_H_

#include "Quotes.h"

class PortfolioAnalyzer {
public:
	PortfolioAnalyzer();
	void sellAll(Date date, Price &cash);
	void buy(const Ticker &ticker, Price sum, Date date, Price &cash);
	void result() const;

private:
	struct Position {
		size_t pos{0};
		Price base{0};
	};
	using Portfolio = std::map<Ticker, Position>;
	Portfolio portfolio_;
};

#endif
