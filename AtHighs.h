#ifndef STRATOTEST_ATHIGHS_H_
#define STRATOTEST_ATHIGHS_H_

#include <vector>
#include "Quotes.h"

using Tickers = std::vector<Ticker>;

class AtHighs {
public:
	AtHighs();
	void run(Price cash, const Date &begin, const Date &end);

	Date quotesStart() const {return quotesStart_;}
	Date quotesEnd() const {return quotesEnd_;}

private:
	Tickers stocks_{
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
		//"EFAV",
		//"USMV",
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
	Tickers bonds_{
		"TLT",
		"IEF",
		"SHY",
		"JNK",
		"HYG",
		"LQD",
		"EMB",
		"BND",
	};

	Tickers moneyEquiv_{"SHV"};

	Date quotesStart_;
	Date quotesEnd_;

	Tickers findAtHighs(const Tickers &tickers, const Date &onDate);
	void v1(Price cash, const Date &begin, const Date &end);
	void v2(Price cash, const Date &begin, const Date &end);
	void v3(Price cash, const Date &begin, const Date &end);
};

#endif
