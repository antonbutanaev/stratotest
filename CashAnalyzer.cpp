#include <cmath>
#include "CashAnalyzer.h"
#include "Log.h"

void CashAnalyzer::addBalance(Price sum) {
	if (prev != 0.) {

		double res = sum / prev - 1;
		if (max < sum)
			max = sum;

		if (sum < max)
			++numInMinus;
		else {
			if (maxNumInMinus < numInMinus)
				maxNumInMinus = numInMinus;
			numInMinus = 0;
		}

		if (res < 0)
			++numContLosses;
		else {
			if (maxNumContLosses < numContLosses)
				maxNumContLosses = numContLosses;
			numContLosses = 0;
		}

		const auto dd = 100. * (sum / max - 1);
		if (maxDrawDown > dd)
			maxDrawDown = dd;

		LOG("=== CASH "
			<< sum
			<< " change " << 100. * res << "%"
			<< " DD " << dd  << "%"
		);

	}
	prev = sum;
}

void CashAnalyzer::result(Date startDate, Date endDate, Price cash, Price origCash) {
	const auto rate = cash / origCash;
	const auto numMonths = (endDate.year()/endDate.month() - startDate.year()/startDate.month()).count();
	const auto annual = pow(rate, 12./numMonths);
	LOG("RESULT "
		<< startDate << " " << endDate
		<< " years " << numMonths/12.
		<< " result " << cash << " rate " << rate << " annual " << 100 * (annual-1) << "%"
		<< " max in minus " << maxNumInMinus
		<< " max cont losses " << maxNumContLosses
		<< " max dd " << maxDrawDown
	);

}
