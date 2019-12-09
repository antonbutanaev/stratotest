#include "Log.h"
#include "PortfolioAnalyzer.h"

using namespace std;

PortfolioAnalyzer::PortfolioAnalyzer() {
}

void PortfolioAnalyzer::sellAll(Date date, Price &cash) {
	for (auto &position: portfolio_) {
		const auto price = Quotes::get().getQuote(position.first, date);
		cash += position.second.pos * price;
		stat_[position.first].numTrades += 1;
	}
	portfolio_.clear();
}

void PortfolioAnalyzer::buy(const Ticker &ticker, Price sum, Date date, Price &cash) {
	Price price = Quotes::get().getQuote(ticker, date);
	size_t num = sum / price;
	portfolio_[ticker].pos += num;
	cash -= num * price;
	stat_[ticker].numTrades += 1;
}

void PortfolioAnalyzer::result() const {
	cout << "Portfolio stat:" << endl;
	for (const auto &stat: stat_)
		cout << stat.first << " " << stat.second.numTrades << endl;
}
