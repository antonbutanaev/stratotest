#ifndef STRATOTEST_SETTINGS_H_
#define STRATOTEST_SETTINGS_H_

#include "Quotes.h"

struct Settings {
	Price comission{.5};
	struct {
		size_t numLookBack{12};
	} atHighs;
	struct {
		bool positionAnalyzer{true};
	} log;

	static Settings get() {
		static Settings settings;
		return settings;
	}
private:
	Settings() {}
};

#endif
