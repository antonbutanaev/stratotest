#include "Log.h"
#include "PortfolioAnalyzer.h"

PortfolioAnalyzer::PortfolioAnalyzer() {
}

void PortfolioAnalyzer::sellAll(Date date, Price &cash) {
	for (auto &position: portfolio_) {
		const auto price = Quotes::get().getQuote(position.first, date);
		cash += position.second.pos * price;
	}
	portfolio_.clear();
}

void PortfolioAnalyzer::buy(const Ticker &ticker, Price sum, Date date, Price &cash) {
	Price price = Quotes::get().getQuote(ticker, date);
	size_t num = sum / price;
	portfolio_[ticker].pos += num;
	cash -= num * price;
}

void PortfolioAnalyzer::result() const {

}
