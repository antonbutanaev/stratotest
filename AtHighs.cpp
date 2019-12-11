#include <iostream>
#include <algorithm>

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

	sort(stocks_.begin(), stocks_.end());
	sort(bonds_.begin(), bonds_.end());
	if (moneyEquiv_.size() != 1)
		THROW("Only 1 money equivalent supported");

	loadQuotes(stocks_);
	loadQuotes(bonds_);
	loadQuotes(moneyEquiv_);

	quotesStart_ += months{1};
	printIf(Settings::get().atHighs.logStrategy, "Quotes period", quotesStart_, quotesEnd_);
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

		printIf(Settings::get().atHighs.logFindHighs, "Analyze prices", ticker, date, onDate);

		for (; date < onDate; date += months{1}) {
			const auto price = Quotes::get().getQuote(ticker, date);
			printIf(Settings::get().atHighs.logFindHighs, date, onDate, price, priceOnDate);

			if (price > priceOnDate) {
				atHigh = false;
				break;
			}

		}
		if (atHigh) {
			const auto gain = 100 * (priceOnDate / Quotes::get().getQuote(ticker, onDate - months{1}) - 1);
			printIf(Settings::get().atHighs.logFindHighs, "At high, prev month gain", gain);

			atHighs.push_back(S{{}, ticker, gain});
		}
	}
	std::sort(atHighs.begin(), atHighs.end(), [](auto a, auto b) {
		return a.gain < b.gain;
	});
	printIf(Settings::get().atHighs.logStrategy, "At highs, sorted by prev month gain", atHighs);
	Tickers res;
	res.reserve(atHighs.size());
	for (const auto &it: atHighs)
		res.push_back(it.ticker);
	return res;
}

void AtHighs::run(Price cash, const Date &begin, const Date &end) {
	v2(cash, begin, end);
}

void AtHighs::v2(Price cash, const Date &begin, const Date &end) {
	// Усовершенствованный алгоритм:
	// есть стоки на хаях (с сортировкой и выборкой)?
	// продаём все бонды и эквивалент кеша из портфеля
	// продаём те стоки из портфеля, что не на хаях
	// докупаем на доступный кеш, стоки, что на хаях
	//
	// нет стоков на хаях
	// делаем то же с бондами

	// нет бондов на хаях
	// сидим в эквиваленте кеша
	const auto origCash = cash;
	CashAnalyzer cashAnalyzer;
	PortfolioAnalyzer portfolioAnalyzer;

	for (auto date = begin; date < end; date += months{1}) {
		printIf(
			Settings::get().atHighs.logStrategy,
			"Date",
			date
		);

		cashAnalyzer.addBalance(cash + portfolioAnalyzer.value(date));

		const auto rebalance = [&](const Tickers &atHighs) {
			if (atHighs.empty())
				return false;

			auto atHighsSorted = atHighs;
			sort(atHighsSorted.begin(), atHighsSorted.end());
			auto portfolio = portfolioAnalyzer.portfolio();
			// продажа всего, что не на хаях
			Tickers toSell;
			set_difference(
				portfolio.begin(), portfolio.end(),
				atHighsSorted.begin(), atHighsSorted.end(),
				back_inserter(toSell)
			);
			for (const auto &ticker: toSell)
				portfolioAnalyzer.sell(ticker, date, cash);

			const auto numPositions = portfolioAnalyzer.numPositions();
			printIf(
				Settings::get().atHighs.logStrategy,
				"Positions: has, need",
				numPositions, Settings::get().atHighs.numToBuy
			);

			if (Settings::get().atHighs.numToBuy > numPositions) {
				auto numToBuy = Settings::get().atHighs.numToBuy - numPositions;
				Tickers toBuy;
				const auto portfolio = portfolioAnalyzer.portfolio();
				for (const auto &ticker: atHighs)
					if (!binary_search(portfolio.begin(), portfolio.end(), ticker)) {
						toBuy.push_back(ticker);
						if (--numToBuy == 0)
							break;
					}

				Price sum = cash / toBuy.size();
				for (const auto &ticker: toBuy)
					portfolioAnalyzer.buy(ticker, sum, date, cash);
			}

			return true;
		};

		if (
			!rebalance(findAtHighs(stocks_, date)) &&
			!rebalance(findAtHighs(bonds_, date))
		) {
			portfolioAnalyzer.sellAll(date, cash);
			portfolioAnalyzer.buy(moneyEquiv_[0], cash, date, cash);

		}
	}

	printIf(
		Settings::get().atHighs.logStrategy,
		"End",
		end
	);

	portfolioAnalyzer.sellAll(end, cash);
	cashAnalyzer.addBalance(cash);
	cashAnalyzer.result(begin, end, cash, origCash);
	portfolioAnalyzer.result();
}

void AtHighs::v1(Price cash, const Date &begin, const Date &end) {
	const auto origCash = cash;
	CashAnalyzer cashAnalyzer;
	PortfolioAnalyzer portfolioAnalyzer;

	for (auto date = begin; date < end; date += months{1}) {
		portfolioAnalyzer.sellAll(date, cash);
		cashAnalyzer.addBalance(cash);

		auto stocksAtHighs = findAtHighs(stocks_, date);
		if (stocksAtHighs.size() > Settings::get().atHighs.numToBuy)
			stocksAtHighs.resize(Settings::get().atHighs.numToBuy);

		printIf(Settings::get().atHighs.logStrategy, "Stocks at highs on ", date, stocksAtHighs);

		if (!stocksAtHighs.empty()) {
			const auto sum = cash / stocksAtHighs.size();
			for (const auto &ticker: stocksAtHighs)
				portfolioAnalyzer.buy(ticker, sum, date, cash);
		} else {
			auto bondsAtHighs = findAtHighs(bonds_, date);
			if (bondsAtHighs.size() > Settings::get().atHighs.numToBuy)
				bondsAtHighs.resize(Settings::get().atHighs.numToBuy);

			printIf(Settings::get().atHighs.logStrategy, "Stocks at highs on ", date, bondsAtHighs);

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

