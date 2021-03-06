#ifndef STRATOTEST_SETTINGS_H_
#define STRATOTEST_SETTINGS_H_

#include "Quotes.h"

struct Settings {
	Price comission{.5};

	struct {
		bool logParse{false};
		bool logEma{false};
		bool logResult{false};
	} quotes;

	struct {
		size_t numLookBack{12};
		size_t numToBuy{3};
		bool logStrategy{false};
		bool logFindHighs{false};
	} atHighs;

	struct {
		bool logBuy{false};
		bool logSell{false};
		bool analyze{false};
	} positionAnalyzer;

	struct {
		bool log{false};
	} cashAnalyzer;

	static Settings get() {
		static Settings settings;
		return settings;
	}
private:
	Settings() {}
};

#endif
