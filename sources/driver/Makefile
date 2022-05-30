all: driver-generator execute-driver

# can't use c++20 because of opencv
flags = -O3 -std=c++17
warning_flags = -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wcast-qual -Wconversion -Wunreachable-code -Winit-self
opencv_flags = $(shell pkg-config opencv4 --cflags --libs)

driver-generator: driver-generator.cpp getquery.h lenquery.h setquery.h executequery.h driver.hpp
	g++ $^ ${flags} ${warning_flags} ${opencv_flags} -o $@ 

execute-driver: execute-driver.cpp executequery.h driver.hpp
	g++ $^ ${flags} ${warning_flags} -o $@

clean:
	rm -f driver-generator execute-driver

.PHONY: clean
