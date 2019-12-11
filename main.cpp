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

	const auto cash = 10000.;
	if (1)
		for (Date d = atHighs.quotesStart(); d < atHighs.quotesEnd(); d += months{1}) {
			atHighs.run(cash, d, atHighs.quotesEnd());
			break;
		}
	if (0) {
		for (size_t years = 1; years <= 5; ++years) {
			const auto spanMonths = years * 12;
			for (Date d = atHighs.quotesStart(); d < atHighs.quotesEnd() - months{spanMonths} ; d += months{1})
				atHighs.run(cash, d, d + months{spanMonths});
		}
	}

	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
