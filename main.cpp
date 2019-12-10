#include <iomanip>
#include "AtHighs.h"
#include "CashAnalyzer.h"
#include "Log.h"
#include "Quotes.h"

using namespace std;
using namespace date;

int main() try {
	cout << fixed << setprecision(2);
	AtHighs atHighs;

	Quotes::get().calcEMA(10);

	if (1)
		for (Date d = atHighs.quotesStart(); d < atHighs.quotesEnd(); d += months{1})
			atHighs.run(10000., d, atHighs.quotesEnd());
	if (1) {
		size_t span = 24;
		for (Date d = atHighs.quotesStart(); d < atHighs.quotesEnd() - months{span} ; d += months{1})
			atHighs.run(10000., d, d + months{span});

	}

	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
