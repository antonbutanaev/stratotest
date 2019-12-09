#ifndef STRATOTEST_CASHANALYZER_H_
#define STRATOTEST_CASHANALYZER_H_

#include "Quotes.h"

class CashAnalyzer {
public:
	void addBalance(Price sum);
	void result(Date startDate, Date endDate, Price cash, Price origCash);
private:
	Price max_{0.};
	Price prev_{0.};
	Price maxDrawDown_{0.};
	size_t numInMinus_{0};
	size_t maxNumInMinus_{0};
	size_t numContLosses_{0};
	size_t maxNumContLosses_{0};
	std::vector<double> gaines_;
};

#endif
