#include <numeric>
#include <cmath>
#include "Log.h"
#include "Settings.h"
#include "PortfolioAnalyzer.h"
#include "Utils.h"

using namespace std;

PortfolioAnalyzer::PortfolioAnalyzer() {
}

void PortfolioAnalyzer::sell(const Ticker &ticker, Date date, Price &cash) {
	const auto portfolioIt = portfolio_.find(ticker);
	if (portfolioIt == portfolio_.end())
		THROW("Could not sell " << ticker << " on " << date);

	auto &position = *portfolioIt;
	const auto price = Quotes::get().getQuote(ticker, date);
	const auto pos = position.second.pos;
	position.second.pos = 0;
	cash += pos * price;
	cash -= Settings::get().comission;
	auto &stat = stat_[position.first];
	stat.numTrades += 1;
	stat.gaines.push_back(price / position.second.base - 1);
	stat.profits.push_back(pos * (price - position.second.base));
	printIf(
		Settings::get().positionAnalyzer.logSell,
		"Sell",
		position.first,
		date,
		pos,
		price,
		pos * price,
		cash,
		100 * stat.gaines.back(),
		stat.profits.back()
	);
}

Price PortfolioAnalyzer::value(Date date) const {
	Price res = 0;
	for (auto &position: portfolio_)
		res += position.second.pos * Quotes::get().getQuote(position.first, date);
	return res;
}

size_t PortfolioAnalyzer::numPositions() const {
	size_t res = 0;
	for (auto &position: portfolio_)
		if (position.second.pos != 0)
			++res;
	return res;
}

std::vector<Ticker> PortfolioAnalyzer::portfolio() const {
	std::vector<Ticker> result;
	for (auto &position: portfolio_)
		if (position.second.pos != 0)
			result.push_back(position.first);
	return result;
}

void PortfolioAnalyzer::sellAll(Date date, Price &cash) {
	for (auto &position: portfolio_)
		if (position.second.pos != 0)
			sell(position.first, date, cash);
	portfolio_.clear();
}

void PortfolioAnalyzer::buy(const Ticker &ticker, Price sum, Date date, Price &cash) {
	const Price price = Quotes::get().getQuote(ticker, date);
	const size_t num = sum / price;
	if (num == 0)
		return;
	auto &portfolio = portfolio_[ticker];
	portfolio.base = (portfolio.base * portfolio.pos + num * price) / (portfolio.pos + num);
	portfolio.pos += num;
	cash -= num * price;
	cash -= Settings::get().comission;
	printIf(
		Settings::get().positionAnalyzer.logBuy,
		"Buy",
		ticker,
		date,
		num,
		price,
		num * price,
		cash
	);
}

void PortfolioAnalyzer::result() {
	if (!Settings::get().positionAnalyzer.analyze)
		return;
	print("Ticker", "Num", "AvgGain", "MedGain", "Profit", "AvgProfit", "MedProfit", "Sharpe");
	for (auto &stat: stat_) {
		auto &gaines = stat.second.gaines;
		auto &profits = stat.second.profits;
		if (!gaines.empty()) {
			std::sort(gaines.begin(), gaines.end());
			const auto avgGain = accumulate(gaines.begin(), gaines.end(), 0.) / gaines.size();
			std::sort(profits.begin(), profits.end());
			const auto profit = accumulate(profits.begin(), profits.end(), 0.);
			print(
				stat.first,
				stat.second.numTrades,
				100 * avgGain,
				100 * gaines[gaines.size() / 2],
				profit,
				profit / profits.size(),
				profits[profits.size() / 2],
				sharpe(gaines)
			);
		}
	}
}
