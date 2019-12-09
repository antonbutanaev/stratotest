#include <cmath>
#include <numeric>

#include "CashAnalyzer.h"
#include "Log.h"

using namespace std;

void CashAnalyzer::addBalance(Price sum) {
	if (prev_ != 0.) {

		double gain = sum / prev_ - 1;
		gaines_.push_back(gain);

		if (max_ < sum)
			max_ = sum;

		if (sum < max_)
			++numInMinus_;
		else {
			if (maxNumInMinus_ < numInMinus_)
				maxNumInMinus_ = numInMinus_;
			numInMinus_ = 0;
		}

		if (gain < 0)
			++numContLosses_;
		else {
			if (maxNumContLosses_ < numContLosses_)
				maxNumContLosses_ = numContLosses_;
			numContLosses_ = 0;
		}

		const auto drawDown = 100. * (sum / max_ - 1);
		if (maxDrawDown_ > drawDown)
			maxDrawDown_ = drawDown;

		LOG("=== CASH "
			<< sum
			<< " change " << 100. * gain << "%"
			<< " DD " << drawDown  << "%"
		);

	}
	prev_ = sum;
}

void CashAnalyzer::result(Date startDate, Date endDate, Price cash, Price origCash) {
	if (gaines_.empty()) {
		cout << "NO RESULT" << endl;
		return;
	}

	std::sort(gaines_.begin(), gaines_.end());
	const auto avgGain = accumulate(gaines_.begin(), gaines_.end(), 0.) / gaines_.size();
	const auto rate = cash / origCash;
	const auto numMonths = (endDate.year()/endDate.month() - startDate.year()/startDate.month()).count();
	const auto annual = pow(rate, 12./numMonths);
	LOG("RESULT "
		<< startDate << " " << endDate
		<< " years " << numMonths/12.
		<< " result " << cash << " rate " << rate << " annual " << 100 * (annual - 1) << "%"
		<< " max in minus " << maxNumInMinus_
		<< " max cont losses " << maxNumContLosses_
		<< " max dd " << maxDrawDown_ << "%"
		<< " median gain " << 100 * gaines_[gaines_.size() / 2] << "%"
		<< " avg gain " << 100 * avgGain << "%"

	);

}
