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
	v3(cash, begin, end);
}

void AtHighs::v2(Price cash, const Date &begin, const Date &end) {
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

		const auto rebalance = [&](Tickers atHighs) {
			if (atHighs.empty())
				return false;

			if (atHighs.size() > Settings::get().atHighs.numToBuy)
				atHighs.resize(Settings::get().atHighs.numToBuy);

			printIf(
				Settings::get().atHighs.logStrategy,
				"At highs on ",
				date,
				atHighs
			);

			const auto sum = cash / atHighs.size();
			for (const auto &ticker: atHighs)
				portfolioAnalyzer.buy(ticker, sum, date, cash);
			return true;
		};

		if (
			!rebalance(findAtHighs(stocks_, date)) &&
			!rebalance(findAtHighs(bonds_, date))
		)
			portfolioAnalyzer.buy(moneyEquiv_[0], cash, date, cash);
	}

	portfolioAnalyzer.sellAll(end, cash);
	cashAnalyzer.addBalance(cash);
	cashAnalyzer.result(begin, end, cash, origCash);
	portfolioAnalyzer.result();
}

void AtHighs::v3(Price cash, const Date &begin, const Date &end) {
	const auto origCash = cash;
	CashAnalyzer cashAnalyzer;
	PortfolioAnalyzer portfolioAnalyzer;

	for (auto date = begin; date < end; date += months{1}) {
		printIf(
			Settings::get().atHighs.logStrategy,
			"Date",
			date
		);

		const auto portfolio = portfolioAnalyzer.portfolio();

		portfolioAnalyzer.sellAll(date, cash);
		cashAnalyzer.addBalance(cash);

		const auto rebalance = [&](Tickers atHighs) {
			if (atHighs.empty())
				return false;

			for (const auto &position: portfolio) {
				const auto it = find(atHighs.begin(), atHighs.end(), position);
				if (it != atHighs.end()) {
					atHighs.erase(it);
					atHighs.insert(atHighs.empty()? atHighs.end() : atHighs.begin(), position);
				}
			}

			if (atHighs.size() > Settings::get().atHighs.numToBuy)
				atHighs.resize(Settings::get().atHighs.numToBuy);

			printIf(
				Settings::get().atHighs.logStrategy,
				"At highs on ",
				date,
				atHighs
			);

			const auto sum = cash / atHighs.size();
			for (const auto &ticker: atHighs)
				portfolioAnalyzer.buy(ticker, sum, date, cash);
			return true;
		};

		if (
			!rebalance(findAtHighs(stocks_, date)) &&
			!rebalance(findAtHighs(bonds_, date))
		)
			portfolioAnalyzer.buy(moneyEquiv_[0], cash, date, cash);
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

