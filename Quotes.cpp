#include <fstream>
#include "Log.h"
#include "Quotes.h"

using namespace std;
using namespace date;

Quotes::Quotes() {
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

	while (in.peek(), !in.eof()) {
		int yi, mi, di;
		char dash;
		in >> yi >> dash >> mi >> dash >> di;

		LOG0("Date: " << yi << ':' << mi << ':' << di << ' ');

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

		LOG0("Quote: "
			open << ' '
			<< high << ' '
			<< low << ' '
			<< close << ' '
			<< adjClose << ' '
			<< volume
		);

		m_quotes[ticker][year{yi}/mi/di] = adjClose;
	}

	cout << "Quotes for " << ticker << " num " << m_quotes[ticker].size();
	if (!m_quotes[ticker].empty())
		cout << " begin on " << m_quotes[ticker].begin()->first;
	cout << endl;
}



