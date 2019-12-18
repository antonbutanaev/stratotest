#include <cmath>
#include <numeric>
#include "CashAnalyzer.h"
#include "Log.h"
#include "Settings.h"
#include "Utils.h"

using namespace std;

void CashAnalyzer::addBalance(Price sum) {
	if (prev_ != 0.) {

		double gain = sum / prev_ - 1;
		gaines_.push_back(gain);

		if (max_ < sum)
			max_ = sum;

		if (sum < max_)
			++numInLoss_;
		else {
			if (maxNumInLoss_ < numInLoss_)
				maxNumInLoss_ = numInLoss_;
			numInLoss_ = 0;
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

		printIf(
			Settings::get().cashAnalyzer.log,
			"CASH", "Prev", "Gain%", "DD%"
		);
		printIf(
			Settings::get().cashAnalyzer.log,
			sum, prev_, 100. * gain, drawDown
		);
	}
	prev_ = sum;
}

void CashAnalyzer::result(Date startDate, Date endDate, Price cash, Price origCash) {
	if (gaines_.empty()) {
		return;
	}

	if (maxNumInLoss_ < numInLoss_)
		maxNumInLoss_ = numInLoss_;

	if (maxNumContLosses_ < numContLosses_)
		maxNumContLosses_ = numContLosses_;

	std::sort(gaines_.begin(), gaines_.end());
	const auto avgGain = accumulate(gaines_.begin(), gaines_.end(), 0.) / gaines_.size();
	const auto rate = cash / origCash;
	const auto numMonths = (endDate.year()/endDate.month() - startDate.year()/startDate.month()).count();
	const auto annual = pow(rate, 12./numMonths);
	size_t pos0 = 0;
	while (pos0 < gaines_.size() && gaines_[pos0] < .0)
		++pos0;

	static bool titlePrinted = false;
	printIf(
		!titlePrinted,
		"Start", "End", "Years", "Rate", "Annual", "MaxInLoss", "MaxContLoss", "MaxDD", "AvgGain", "MedGain", "Gain0+", "Sharpe"
	);
	titlePrinted = true;

	print(
		startDate,
		endDate,
		numMonths/12.,
		rate,
		100. * (annual - 1),
		maxNumInLoss_,
		maxNumContLosses_,
		maxDrawDown_,
		100. * avgGain,
		100. * gaines_[gaines_.size() / 2],
  		100. * (gaines_.size() - pos0) / gaines_.size(),
  		sharpe(gaines_)
	);
}
