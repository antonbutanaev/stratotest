#include <fstream>
#include "Log.h"
#include "Quotes.h"
#include "Settings.h"

using namespace std;
using namespace date;

Quotes::Quotes() {
}

void Quotes::calcEMA(size_t period) {
	const auto k = 2. / (period + 1);
	for (const auto &quotes: m_quotes) {
		const auto &ticker = quotes.first;
		const auto &prices = quotes.second;
		if (prices.empty())
			continue;

		static bool titlePrinted = false;
		printIf(
			!titlePrinted && Settings::get().quotes.logEma,
			"Ticker", "Date", "Price", "Ema"
		);
		titlePrinted = true;

		auto prevEma = prices.begin()->second;
		m_ema[ticker][prices.begin()->first] = prevEma;
		printIf(
			Settings::get().quotes.logEma,
			ticker,
			prices.begin()->first,
			prevEma,
			prevEma
		);

		for (auto priceIt = next(prices.begin()); priceIt != prices.end(); ++priceIt) {
			const auto price = priceIt->second;
			const auto ema = k * price + (1 - k) * prevEma;
			m_ema[ticker][priceIt->first] = ema;
			printIf(
				Settings::get().quotes.logEma,
				ticker,
				priceIt->first,
				price,
				ema
			);
			prevEma = ema;
		}
	}
}

Price Quotes::getEMA(const Ticker &ticker, const Date &date) {
	const auto tickerIt = m_ema.find(ticker);
	if (tickerIt == m_ema.end())
		THROW("Quotes::getEMA ticker " << ticker << " not found");

	const auto priceIt = tickerIt->second.find(date);
	if (priceIt == tickerIt->second.end())
		THROW("Quotes::getEma ticker " << ticker << " date " << date << " not found");

	return priceIt->second;
}

void Quotes::loadQuotes(const Ticker &ticker, Date &begin, Date &end) {
	get().parseQuotes(ticker);
	const auto tickerBegin = get().getFisrtDate(ticker);
	const auto tickerEnd = get().getLastDate(ticker);
	if (!begin.ok() || begin < tickerBegin)
		begin = tickerBegin;
	if (!end.ok() || end > tickerEnd)
		end = tickerEnd;
}

Date Quotes::getFisrtDate(const Ticker &ticker) {
	const auto tickerIt = m_quotes.find(ticker);
	if (tickerIt == m_quotes.end())
		THROW("Quotes::getQuote ticker " << ticker << " not found");
	return tickerIt->second.begin()->first;
}

Date Quotes::getLastDate(const Ticker &ticker) {
	const auto tickerIt = m_quotes.find(ticker);
	if (tickerIt == m_quotes.end())
		THROW("Quotes::getQuote ticker " << ticker << " not found");
	return std::prev(tickerIt->second.end())->first;
}

Price Quotes::getQuote(const Ticker &ticker, const Date &date) {
	const auto tickerIt = m_quotes.find(ticker);
	if (tickerIt == m_quotes.end())
		THROW("Quotes::getQuote ticker " << ticker << " not found");

	const auto priceIt = tickerIt->second.find(date);
	if (priceIt == tickerIt->second.end())
		THROW("Quotes::getQuote ticker " << ticker << " date " << date << " not found");

	return priceIt->second;
}

void Quotes::parseQuotes(const std::string &ticker) {
	const auto filePath = "/home/anton/yadisk/inv/quotes/" + ticker + ".csv";
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

	printIf(
		Settings::get().quotes.logParse,
		"Ticker", "Date", "Open", "High", "Low", "Close", "AdjClose", "Volume"
	);
	while (in.peek(), !in.eof()) {
		int yi, mi, di;
		char dash;
		in >> yi >> dash >> mi >> dash >> di;

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

		printIf(
			Settings::get().quotes.logParse,
			ticker,
			year{yi}/mi/di,
			open,
			high,
			low,
			close,
			adjClose,
			volume
		);

		m_quotes[ticker][year{yi}/mi/di] = adjClose;
	}

	if (Settings::get().quotes.logResult) {
		static bool headerPrinted = false;
		printIf(!headerPrinted, "Ticker", "Num", "Begin", "End");
		headerPrinted = true;

		if (!m_quotes[ticker].empty())
			print(ticker, m_quotes[ticker].size(), m_quotes[ticker].begin()->first, prev(m_quotes[ticker].end())->first);
	}
}



