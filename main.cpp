#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <date/date.h>

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
}

void testStrategy(const Strategy &strategy, const Date &startDate) {
	const auto endDate = year{2019}/10/1;
	Price cash = 10000.;
	auto cashOrig = cash;
	Portfolio portfolio;
	const auto printCash = [&](const auto &date){
		LOG("=== " << date << " CASH " << cash);
	};
	const auto printPortfolio = [&](const auto &date){
		for (const auto &position: portfolio) {
			LOG("ON " << date << " TICKER " << position.first << " " << position.second);
		}
	};
	buy(strategy, startDate, cash, portfolio);
	for (auto date = startDate + months{1}; date < endDate; date += months{1}) {
		printPortfolio(date);
		sellAll(date, cash, portfolio);
		printCash(date);
		buy(strategy, date, cash, portfolio);
	}
	sellAll(endDate, cash, portfolio);
	printCash(endDate);

	const auto rate = cash / cashOrig;
	const auto numMonths = (endDate.year()/endDate.month() - startDate.year()/startDate.month()).count();
	const auto annual = pow(rate, 12./numMonths);

	LOG("RESULT from " << startDate << " years " << numMonths/12. <<  " CASH " << cash << " rate " << rate << " annual " << 100 * (annual-1));
}

int main() try {
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

	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
