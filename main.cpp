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
	atHighs.run(10000., atHighs.quotesStart(), atHighs.quotesEnd());
	return 0;
} catch (const std::exception &x) {
	cout << x.what() << endl;
	return 1;
}
