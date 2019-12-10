#include <iostream>
#include "AtHighs.h"
#include "CashAnalyzer.h"
#include "PortfolioAnalyzer.h"
#include "Settings.h"
#include "Log.h"

using namespace std;
using namespace date;

AtHighs::AtHighs() {
	const auto loadQuotes = [&](const Tickers &tickers) {
		for (const auto &ticker: tickers)
			Quotes::loadQuotes(ticker, quotesStart_, quotesEnd_);
	};

	loadQuotes(stocks_);
	loadQuotes(bonds_);
	loadQuotes(moneyEquiv_);

	quotesStart_ += months{1};
	if (Settings::get().atHighs.logStrategy)
		print("Quotes period", quotesStart_, quotesEnd_);
}

Tickers AtHighs::findAtHighs(const Tickers &tickers, const Date &onDate) {
	const size_t numBack = Settings::get().atHighs.numLookBack;

	struct S: public Printable {
		Ticker ticker;
		double gain;

		void print() const {
			::print(ticker, gain, NoEOL);
		}
	};
	vector<S> atHighs;

	for (const auto &ticker: tickers) {
		const auto priceOnDate = Quotes::get().getQuote(ticker, onDate);
		bool atHigh = true;
		const auto firstDate = Quotes::get().getFisrtDate(ticker);
		auto date = onDate - months{numBack};
		if (date < firstDate)
			date = firstDate;

		if (Settings::get().atHighs.logStrategy)
			print("Analyze prices", ticker, date, onDate);

		for (; date < onDate; date += months{1}) {
			const auto price = Quotes::get().getQuote(ticker, date);
			if (Settings::get().atHighs.logStrategy)
				print(date, onDate, price, priceOnDate);

			if (price > priceOnDate) {
				atHigh = false;
				break;
			}

		}
		if (atHigh) {
			const auto gain = 100 * (priceOnDate / Quotes::get().getQuote(ticker, onDate - months{1}) - 1);
			if (Settings::get().atHighs.logStrategy)
				print("At high, prev month gain", gain);

			atHighs.push_back(S{{}, ticker, gain});
		}
	}
	std::sort(atHighs.begin(), atHighs.end(), [](auto a, auto b) {
		return a.gain < b.gain;
	});
	if (Settings::get().atHighs.logStrategy)
		print("At highs, sorted by prev month gain", atHighs);
	Tickers res;
	size_t num = 0;
	for (const auto &it: atHighs) {
		if (++num > Settings::get().atHighs.numToBuy)
			break;
		res.push_back(it.ticker);
	}
	return res;
}

void AtHighs::run(Price cash, const Date &begin, const Date &end) {
	const auto origCash = cash;
	CashAnalyzer cashAnalyzer;
	PortfolioAnalyzer portfolioAnalyzer;

	for (auto date = begin; date < end; date += months{1}) {
		portfolioAnalyzer.sellAll(date, cash);
		cashAnalyzer.addBalance(cash);

		const auto stocksAtHighs = findAtHighs(stocks_, date);
		if (Settings::get().atHighs.logStrategy)
			print("Stocks at highs on ", date, stocksAtHighs);

		if (!stocksAtHighs.empty()) {
			const auto sum = cash / stocksAtHighs.size();
			for (const auto &ticker: stocksAtHighs)
				portfolioAnalyzer.buy(ticker, sum, date, cash);
		} else {
			const auto bondsAtHighs = findAtHighs(bonds_, date);
			if (Settings::get().atHighs.logStrategy)
				print("Stocks at highs on ", date, bondsAtHighs);

			if (!bondsAtHighs.empty()) {
				const auto sum = cash / bondsAtHighs.size();
				for (const auto &ticker: bondsAtHighs)
					portfolioAnalyzer.buy(ticker, sum, date, cash);
			} else
				portfolioAnalyzer.buy(moneyEquiv_[0], cash, date, cash);
		}

	}

	portfolioAnalyzer.sellAll(end, cash);
	cashAnalyzer.addBalance(cash);
	cashAnalyzer.result(begin, end, cash, origCash);
	portfolioAnalyzer.result();
}

