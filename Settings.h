#ifndef STRATOTEST_SETTINGS_H_
#define STRATOTEST_SETTINGS_H_

#include "Quotes.h"

struct Settings {
	Price comission{.5};

	struct {
		bool logParse{false};
		bool logResult{true};
	} quotes;

	struct {
		size_t numLookBack{12};
		size_t numToBuy{3};
		bool logStrategy{true};
	} atHighs;

	struct {
		bool logBuy{true};
		bool logSell{true};
		bool analyze{true};
	} positionAnalyzer;

	struct {
		bool log{true};
	} cashAnalyzer;

	static Settings get() {
		static Settings settings;
		return settings;
	}
private:
	Settings() {}
};

#endif
