#include <numeric>
#include "Log.h"
#include "PortfolioAnalyzer.h"

using namespace std;

const Price comission = .4;

PortfolioAnalyzer::PortfolioAnalyzer() {
}

void PortfolioAnalyzer::sellAll(Date date, Price &cash) {
	for (auto &position: portfolio_) {
		const auto price = Quotes::get().getQuote(position.first, date);
		cash += position.second.pos * price;
		cash -= comission;
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
	cash -= comission;
}

void PortfolioAnalyzer::result() {
	cout << "Portfolio stat:" << endl;
	for (auto &stat: stat_) {
		cout << stat.first << " num " << stat.second.numTrades;
		auto &gaines = stat.second.gaines;
		auto &profits = stat.second.profits;

		if (!gaines.empty()) {
			std::sort(gaines.begin(), gaines.end());
			const auto avgGain = accumulate(gaines.begin(), gaines.end(), 0.) / gaines.size();
			cout
				<< " avgGain " << 100 * avgGain
				<< "% median gain " << 100 * gaines[gaines.size() / 2] << "%";

			std::sort(profits.begin(), profits.end());
			const auto totalProfit = accumulate(profits.begin(), profits.end(), 0.);
			cout
				<< " profit total " << totalProfit
				<< " profit avg " << totalProfit / profits.size()
				<< " profit median " << profits[profits.size() / 2];

		}
		cout << endl;
	}
}
