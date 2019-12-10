#include <fstream>
#include "Log.h"
#include "Quotes.h"
#include "Settings.h"

using namespace std;
using namespace date;

Quotes::Quotes() {
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
		THROW("Quotes::getQuote tikcer " << ticker << " not found");
	return tickerIt->second.begin()->first;
}

Date Quotes::getLastDate(const Ticker &ticker) {
	const auto tickerIt = m_quotes.find(ticker);
	if (tickerIt == m_quotes.end())
		THROW("Quotes::getQuote tikcer " << ticker << " not found");
	return std::prev(tickerIt->second.end())->first;
}

Price Quotes::getQuote(const Ticker &ticker, const Date &date) {
	const auto tickerIt = m_quotes.find(ticker);
	if (tickerIt == m_quotes.end())
		THROW("Quotes::getQuote tikcer " << ticker << " not found");

	const auto priceIt = tickerIt->second.find(date);
	if (priceIt == tickerIt->second.end())
		THROW("Quotes::getQuote tikcer " << ticker << " date " << date << " not found");

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

	if (Settings::get().quotes.logParse)
		print("Ticker", "Date", "Open", "High", "Low", "Close", "AdjClose", "Volume");
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

		if (Settings::get().quotes.logParse)
			print(
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
		if (!headerPrinted) {
			print("Ticker", "Num", "Begin", "End");
			headerPrinted = true;
		}
		if (!m_quotes[ticker].empty())
			print(ticker, m_quotes[ticker].size(), m_quotes[ticker].begin()->first, prev(m_quotes[ticker].end())->first);
	}
}



