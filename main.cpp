#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <date/date.h>

#include "CashAnalyzer.h"
#include "Log.h"
#include "Quotes.h"

using namespace std;
using namespace date;

struct Strategy {
	struct Module {
		std::vector<Ticker> instruments;
		double weight;
	};
	using Modules = std::vector<Module>;
	Modules modules;
};

using  Portfolio = std::map<Ticker, size_t>;

void buy(const Ticker &ticker, Price sum, const Date &date, Price &cash, Portfolio &portfolio) {
	Price price = Quotes::get().getQuote(ticker, date);
	size_t num = sum / price;
	portfolio[ticker] += num;
	cash -= num * price;
}

void buy(const Strategy &strategy, const Date &date, Price &cash, Portfolio &portfolio) {
	const auto prevDate = date - months{6};
	const auto cashOrig = cash;
	for (const auto &module: strategy.modules) {
		double maxChange = 0;
		Price price = NoPrice;
		decltype(module.instruments.begin()) bestInstrumentIt;
		for (auto instrumentIt = module.instruments.begin(); instrumentIt != module.instruments.end(); ++instrumentIt) {
			const auto nowPrice = Quotes::get().getQuote(*instrumentIt, date);
			const auto prevPrice = Quotes::get().getQuote(*instrumentIt, prevDate);
			const auto change = nowPrice/prevPrice - 1;
			LOG0(
				"Performance of " << *instrumentIt
				<< " from " << prevDate << " to " << date
				<< " is " << prevPrice << " - " << nowPrice
				<< " change " << change
			);

			if (change >= 0 && maxChange < change) {
				maxChange = change;
				price = nowPrice;
				bestInstrumentIt = instrumentIt;
			}
		}

		if (price != NoPrice) {
			unsigned num = cashOrig * module.weight / price;
			LOG(
				"Best instrument in module " << (&module - &strategy.modules.front())
				<< " is " << *bestInstrumentIt
				<< " weight " << module.weight
				<< " BUY on " << date << " num " << num << " for " << price
			);
			cash -= num * price;
			portfolio[*bestInstrumentIt] += num;
		}
	}
}

void sellAll(const Date &date, Price &cash, Portfolio &portfolio) {
	for (auto &position: portfolio) {
		const auto price = Quotes::get().getQuote(position.first, date);
		cash += position.second * price;
	}
	portfolio.clear();
	static Price maxCash = 0.;
	if (maxCash < cash)
		maxCash = cash;

	LOG("CASH on " << date << " " << cash << " draw down " << (100. * (cash / maxCash - 1.)) << "%");
}

void testStrategy(const Strategy &strategy, const Date &startDate) {
	const auto endDate = year{2019}/10/1;
	Price cash = 10000.;
	auto cashOrig = cash;
	Portfolio portfolio;
	CashAnalyzer cashAnalyzer;
	const auto printPortfolio = [&](const auto &date){
		for (const auto &position: portfolio) {
			LOG("ON " << date << " TICKER " << position.first << " " << position.second);
		}
	};
	buy(strategy, startDate, cash, portfolio);
	for (auto date = startDate + months{1}; date < endDate; date += months{1}) {
		printPortfolio(date);
		sellAll(date, cash, portfolio);
		cashAnalyzer.addBalance(cash);
		buy(strategy, date, cash, portfolio);
	}
	sellAll(endDate, cash, portfolio);
	cashAnalyzer.addBalance(cash);
	cashAnalyzer.result(startDate, endDate, cash, cashOrig);
}

void strategySGCapital() {
	const Strategy strategy {
		{
			{
				{
					"VTI",
					"VEA",
					"VWO",
					"SHV",
				},
				0.35,
			},
			Strategy::Module{
				{
					"LQD",
					"TLT",
					"EMB",
					"SHV",
				},
				0.35

			},
			Strategy::Module{
				{
					"IAU",
					"DBC",
					"VNQ",
					"SHV",
				},
				0.3,
			},
		}
	};

	for (const auto &module: strategy.modules)
		for (const auto &instrument: module.instruments)
			Quotes::get().parseQuotes(instrument);

	cout << fixed << setprecision(2);

	for (auto date = year{2008}/7/1; date < year{2018}/10/1; date += months{1})
		testStrategy(strategy, date);
}


using Tickers = vector<Ticker>;

Tickers findAtHighs(const Tickers &tickers, const Date &onDate, size_t numBack = 12) {
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

void strategyBuyAtHigh() {
	Tickers stocks = {
		//"VTI",
		//"VEA",
		//"VWO",

		"SPY",
		"GDX",
		"IAU",
		"QQQ",
		//"MOAT",
		//"MTUM",
		//"NOBL",
		//"QUAL",
		//"SOXX",
		"SPLV",
		//"IBB",
		//"IWM",
		"VYM",
		"VIG",
		"VNQ",
		"XLB",
		//"XLC",
		"XLE",
		"XLF",
		"XLI",
		"XLK",
		"XLP",
		//"XLRE",
		"XLU",
		"XLV",
		"XLY",
	};
	Tickers bonds = {
		"TLT",
		"JNK",
		"HYG",
		"LQD",
		"EMB",
		"BND",
	};

	Tickers moneyEquiv = {"SHV"};

	Date quotesStart = year{1900}/1/1;
	Date quotesEnd = year{2099}/1/1;

	const auto loadQuotes = [&](const Tickers &tickers) {
		for (const auto &ticker: tickers) {
			Quotes::get().parseQuotes(ticker);
			const auto tickerStart = Quotes::get().getFisrtDate(ticker);
			const auto tickerEnd = Quotes::get().getLastDate(ticker);
			if (quotesStart < tickerStart)
				quotesStart = tickerStart;
			if (quotesEnd > tickerEnd)
				quotesEnd = tickerEnd;
		}
	};

	loadQuotes(stocks);
	loadQuotes(bonds);
	loadQuotes(moneyEquiv);

	cout << "Quotes period " << quotesStart << ' ' << quotesEnd << endl;
	quotesStart += months{1};

	const auto runStrategy = [&] (const Date &start, const Date &end) {
		Price cash = 10000.;
		Price origCash = cash;
		Portfolio portfolio;
		cout << fixed << setprecision(2);
		cout << "Period " << start << ' ' << end << " cash " << cash << endl;
		CashAnalyzer cashAnalyzer;

		for (auto date = start; date < end; date += months{1}) {
			sellAll(date, cash, portfolio);
			cashAnalyzer.addBalance(cash);

			const auto stocksAtHighs = findAtHighs(stocks, date);
			cout << "Stocks at highs on " << date;
			for (const auto &ticker: stocksAtHighs)
				cout << ' ' << ticker;
			cout << endl;

			if (!stocksAtHighs.empty())
				for (const auto &ticker: stocksAtHighs)
					buy(ticker, cash / stocksAtHighs.size(), date, cash, portfolio);
			else {
				const auto bondsAtHighs = findAtHighs(bonds, date);
				cout << "Bonds  at highs on " << date;
				for (const auto &ticker: bondsAtHighs)
					cout << ' ' << ticker;
				cout << endl;

				if (!bondsAtHighs.empty())
					for (const auto &ticker: bondsAtHighs)
						buy(ticker, cash / bondsAtHighs.size(), date, cash, portfolio);
				else
					buy(moneyEquiv[0], cash, date, cash, portfolio);
			}

		}

		sellAll(end, cash, portfolio);
		cashAnalyzer.addBalance(cash);
		cashAnalyzer.result(start, end, cash, origCash);
	};

	runStrategy(quotesStart, quotesEnd);
}

int main() try {
	strategyBuyAtHigh();
	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
