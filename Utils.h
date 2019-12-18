#ifndef STRATOTEST_UTILS_H_
#define STRATOTEST_UTILS_H_

#include <numeric>
#include <cmath>

template<class T>
double sharpe(const T &gaines, const double risklessGain = 0.) {
	const auto n = std::distance(std::begin(gaines), std::end(gaines));
	if (n == 0)
		return 0;

	const auto avg = std::accumulate(
		std::begin(gaines), std::end(gaines), 0., [&] (auto cum, auto gain) {
			return cum + 100 * gain;
		}
	) / n;

	const auto dev = sqrt(
		std::accumulate(
			std::begin(gaines), std::end(gaines), 0., [&](auto cum, auto gain){
				const auto delta = 100 * (gain - risklessGain);
				return cum + delta * delta;
		}) / n
	);

	return (avg - 100 * risklessGain) / dev;
}

#endif
