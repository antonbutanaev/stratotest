#include <iostream>
#include "AtHighs.h"
#include "CashAnalyzer.h"
#include "PortfolioAnalyzer.h"

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
	cout << "Quotes period " << quotesStart_ << ' ' << quotesEnd_ << endl;
}

Tickers AtHighs::findAtHighs(const Tickers &tickers, const Date &onDate) {
	const size_t numBack = 12;

	struct S {
		Ticker ticker;
		double gain;
	};
	vector<S> atHighs;

	for (const auto &ticker: tickers) {
		const auto priceOnDate = Quotes::get().getQuote(ticker, onDate);
		bool atHigh = true;
		const auto firstDate = Quotes::get().getFisrtDate(ticker);
		auto date = onDate - months{numBack};
		if (date < firstDate)
			date = firstDate;
		for (; date < onDate; date += months{1})
			if (Quotes::get().getQuote(ticker, date) > priceOnDate) {
				atHigh = false;
				break;
			}
		if (atHigh) {
			const auto gain =
				Quotes::get().getQuote(ticker, onDate) /
				Quotes::get().getQuote(ticker, onDate - months{1});
			atHighs.push_back(S{ticker, gain});
		}
	}
	std::sort(atHighs.begin(), atHighs.end(), [](auto a, auto b) {
		return a.gain < b.gain;
	});
	Tickers res;
	int n = 0;
	for (const auto &it: atHighs) {
		if (++n > 3)
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
		cout << "Stocks at highs on " << date;
		for (const auto &ticker: stocksAtHighs)
			cout << ' ' << ticker;
		cout << endl;

		if (!stocksAtHighs.empty())
			for (const auto &ticker: stocksAtHighs)
				portfolioAnalyzer.buy(ticker, cash / stocksAtHighs.size(), date, cash);
		else {
			const auto bondsAtHighs = findAtHighs(bonds_, date);
			cout << "Bonds  at highs on " << date;
			for (const auto &ticker: bondsAtHighs)
				cout << ' ' << ticker;
			cout << endl;

			if (!bondsAtHighs.empty())
				for (const auto &ticker: bondsAtHighs)
					portfolioAnalyzer.buy(ticker, cash / bondsAtHighs.size(), date, cash);
			else
				portfolioAnalyzer.buy(moneyEquiv_[0], cash, date, cash);
		}

	}

	portfolioAnalyzer.sellAll(end, cash);
	cashAnalyzer.addBalance(cash);

	cashAnalyzer.result(begin, end, cash, origCash);
	portfolioAnalyzer.result();
}

