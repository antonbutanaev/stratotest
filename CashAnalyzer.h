#ifndef STRATOTEST_CASHANALYZER_H_
#define STRATOTEST_CASHANALYZER_H_

#include "Quotes.h"

class CashAnalyzer {
public:
	void addBalance(Price sum);
	void result(Date startDate, Date endDate, Price cash, Price origCash);
private:
	Price max{0.};
	Price prev{0.};
	Price maxDrawDown{0.};
	size_t numInMinus{0};
	size_t maxNumInMinus{0};
	size_t numContLosses{0};
	size_t maxNumContLosses{0};
};

#endif
