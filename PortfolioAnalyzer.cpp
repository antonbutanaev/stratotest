#include <numeric>
#include "Log.h"
#include "PortfolioAnalyzer.h"

using namespace std;

PortfolioAnalyzer::PortfolioAnalyzer() {
}

void PortfolioAnalyzer::sellAll(Date date, Price &cash) {
	for (auto &position: portfolio_) {
		const auto price = Quotes::get().getQuote(position.first, date);
		cash += position.second.pos * price;
		auto &stat = stat_[position.first];
		stat.numTrades += 1;
		stat.gaines.push_back(price / position.second.base - 1);
		stat.profits.push_back(position.second.pos * (price - position.second.base));
	}
	portfolio_.clear();
}

void PortfolioAnalyzer::buy(const Ticker &ticker, Price sum, Date date, Price &cash) {
	const Price price = Quotes::get().getQuote(ticker, date);
	const size_t num = sum / price;
	auto &portfolio = portfolio_[ticker];
	portfolio.base = (portfolio.base * portfolio.pos + num * price) / (portfolio.pos + num);
	portfolio.pos += num;
	cash -= num * price;
}

void PortfolioAnalyzer::result() {
	cout << "Portfolio stat:" << endl;
	for (auto &stat: stat_) {
		cout << stat.first << " num " << stat.second.numTrades;
		if (!stat.second.gaines.empty()) {
			std::sort(stat.second.gaines.begin(), stat.second.gaines.end());
			const auto avgGain = accumulate(stat.second.gaines.begin(), stat.second.gaines.end(), 0.) / stat.second.gaines.size();
			cout
				<< " avgGain " << 100 * avgGain
				<< "% median gain " << 100 * stat.second.gaines[stat.second.gaines.size()/2] << "%";

			std::sort(stat.second.profits.begin(), stat.second.profits.end());
			const auto totalProfit = accumulate(stat.second.profits.begin(), stat.second.profits.end(), 0.);
			cout
				<< " profit total " << totalProfit
				<< " profit avg " << totalProfit / stat.second.profits.size()
				<< " profit median " << stat.second.profits[stat.second.profits.size() / 2];

		}
		cout << endl;
	}
}
