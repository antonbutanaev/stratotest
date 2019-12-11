#ifndef STRATOTEST_PORTFOLIOANALYZER_H_
#define STRATOTEST_PORTFOLIOANALYZER_H_

#include "Quotes.h"

class PortfolioAnalyzer {
public:
	PortfolioAnalyzer();
	void sellAll(Date date, Price &cash);
	void sell(const Ticker &ticker, Date date, Price &cash);
	void buy(const Ticker &ticker, Price sum, Date date, Price &cash);
	Price value(Date date) const;
	size_t numPositions() const;

	std::vector<Ticker> portfolio() const;

	void result();

private:
	struct Position {
		size_t pos{0};
		Price base{0};
	};
	using Portfolio = std::map<Ticker, Position>;
	Portfolio portfolio_;

	struct Stat {
		size_t numTrades{0};
		std::vector<double> gaines;
		std::vector<Price> profits;
	};
	std::map<Ticker, Stat> stat_;
};

#endif
