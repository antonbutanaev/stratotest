#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>
#include <chrono>
#include <vector>
#include <date/date.h>

#define LOG(a) std::cout << a << std::endl
#define LOG0(a)
#define THROW(a) {std::stringstream s; s << a; throw std::runtime_error(s.str());}

using Time = date::year_month_day;
using Price = double;
using Ticker = std::string;
const Price NoPrice = 0.;

struct Quote {
	Time date;
	Price price;
};

struct Quotes {
	Ticker ticker;
	std::vector<Quote> quotes;
};

using namespace std;
using namespace date;

struct Strategy {
	struct Module {
		std::vector<Quotes> instruments;
		double weight;
	};
	using Modules = std::vector<Module>;
	Modules modules;
};

using  Portfolio = std::map<Ticker, size_t>;

std::vector<Quote> parseQuotes(const std::string &file) {
	std::vector<Quote> quotes;
	const auto filePath = "/home/anton/temp/quotes/" + file + ".csv";
	ifstream in;
	in.exceptions(ios::failbit|ios::badbit);
	in.unsetf(std::ios_base::skipws);
	in.open(filePath);
	if (!in)
		throw std::runtime_error(filePath + " not found");
	string header;
	getline(in, header);
	if (header != "Date,Open,High,Low,Close,Adj Close,Volume")
		throw std::runtime_error("Wrong quote file format");

	while (in.peek(), !in.eof()) {
		int yi, mi, di;
		char dash;
		in >> yi >> dash >> mi >> dash >> di;

		LOG0(yi << ':' << mi << ':' << di << ' ');

		Quote quote;
		quote.date = year{yi}/mi/di;

		char comma, nl;
		Price open, high, low, close, adjClose;
		long volume;
		in >> comma
			>> open >> comma
			>> high >> comma
			>> low >> comma
			>> close >> comma
			>> adjClose >> comma
			>> volume >> nl;

		LOG0(
			open << ':'
			<< high << ':'
			<< low << ':'
			<< close << ':'
			<< adjClose << ':'
			<< volume
		);

		quote.price = adjClose;
		quotes.push_back(quote);
	}

	LOG(file << ": quotes num " << quotes.size() << " first date " << quotes.front().date);
	return quotes;
}

void buy(const Strategy &strategy, const year_month_day &date, Price &cash, Portfolio &portfolio) {
	const auto prevDate = date - months{6};
	const auto cashOrig = cash;
	for (const auto &module: strategy.modules) {
		double maxChange = 0;
		Price price;
		decltype(module.instruments.begin()) bestInstrumentIt;
		for (auto instrumentIt = module.instruments.begin(); instrumentIt != module.instruments.end(); ++instrumentIt) {
			Price nowPrice = NoPrice;
			Price prevPrice = NoPrice;
			for (const auto &quote: instrumentIt->quotes) {
				if (quote.date == date)
					nowPrice = quote.price;
				if (quote.date == prevDate)
					prevPrice = quote.price;
			}
			if (nowPrice == NoPrice)
				THROW("Price not found for " << instrumentIt->ticker << " on " << date);

			if (prevPrice == NoPrice)
				THROW("Price not found for " << instrumentIt->ticker << " on " << prevDate);

			const auto change = nowPrice/prevPrice - 1;
			LOG0(
				"Performance of " << instrumentIt->ticker
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

		if (maxChange > 0) {
			unsigned num = cashOrig * module.weight / price;
			LOG(
				"Best instrument in module " << (&module - &strategy.modules.front())
				<< " is " << bestInstrumentIt->ticker
				<< " by weight " << module.weight
				<< " buy on " << date << " num " << num << " for " << price
			);
			cash -= num * price;
			portfolio[bestInstrumentIt->ticker] += num;
		}
	}
}

void sell(const Strategy &strategy, const year_month_day &date, Price &cash, Portfolio &portfolio) {
	for (auto &position: portfolio) {
		for (const auto &module: strategy.modules)
			for (const auto &instrument: module.instruments)
				if (instrument.ticker == position.first) {
					for (const auto &quote: instrument.quotes)
						if (quote.date == date) {
							LOG(
								"Sell " << position.first
								<< " price " << quote.price
								<< " on " << quote.date
							);
							cash += position.second * quote.price;
							position.second = 0;
							goto nextPosition;
						}
				}
		THROW("Cannot sell " << position.first << " on " << date);
nextPosition:
		;
	}
}

int main() try {

	const Strategy strategy {
		Strategy::Modules{
			Strategy::Module{
				{
					Quotes{"VTI", parseQuotes("VTI")},
					Quotes{"VEA", parseQuotes("VEA")},
					Quotes{"VWO", parseQuotes("VWO")},
					Quotes{"SHV", parseQuotes("SHV")},
				},
				0.35,
			},
			Strategy::Module{
				{
					Quotes{"LQD", parseQuotes("VTI")},
					Quotes{"TLT", parseQuotes("VEA")},
					Quotes{"EMB", parseQuotes("EMB")},
					Quotes{"SHV", parseQuotes("SHV")},
				},
				0.35

			},
			Strategy::Module{
				{
					Quotes{"IAU", parseQuotes("IAU")},
					Quotes{"DBC", parseQuotes("DBC")},
					Quotes{"VNQ", parseQuotes("VNQ")},
					Quotes{"SHV", parseQuotes("SHV")},
				},
				0.3,
			},
		}
	};

	cout << fixed << setprecision(2);

	const auto startDate = year{2013}/6/1;
	const auto endDate = year{2019}/10/1;
	Price cash = 10000.;
	Portfolio portfolio;
	const auto printCash = [&](const auto &date){
		LOG("ON " << date << " CASH " << cash);
	};
	const auto printPortfolio = [&](const auto &date){
		for (const auto &position: portfolio) {
			LOG("ON " << date << " TICKER " << position.first << " " << position.second);
		}
	};
	buy(strategy, startDate, cash, portfolio);
	for (auto date = startDate + months{1}; date < endDate; date += months{1}) {
		printPortfolio(date);
		sell(strategy, date, cash, portfolio);
		printCash(date);
		buy(strategy, date, cash, portfolio);
	}
	sell(strategy, endDate, cash, portfolio);
	printCash(endDate);

	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
