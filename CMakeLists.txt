cmake_minimum_required(VERSION 3.6)
project(stratotest)

add_subdirectory(date)

add_executable(stratotest
	main.cpp
	Quotes.cpp
	CashAnalyzer.cpp
	PortfolioAnalyzer.cpp
	AtHighs.cpp
	Settings.cpp
)

target_link_libraries(stratotest date)

target_compile_options(stratotest
	PRIVATE
	-Werror -Wall -Wextra
	-Wno-deprecated-declarations
	-Wno-unused-parameter
)

target_compile_features(stratotest PRIVATE cxx_std_17)
