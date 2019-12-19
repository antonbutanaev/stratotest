#ifndef STRATOTEST_UTILS_H_
#define STRATOTEST_UTILS_H_

#include <numeric>
#include <cmath>

template<class T>
double sharpe(const T &gaines, const double risklessGain = .02/12) {
	const auto n = std::distance(std::begin(gaines), std::end(gaines));
	if (n == 0)
		return 0;

	const auto avg = std::accumulate(
		std::begin(gaines), std::end(gaines), 0.) / n;

	const auto dev = sqrt(
		std::accumulate(
			std::begin(gaines), std::end(gaines), 0., [&](auto cum, auto gain){
				const auto delta = gain - risklessGain - avg;
				return cum + delta * delta;
		}) / n
	);

	return (avg - risklessGain) / dev;
}

#endif
