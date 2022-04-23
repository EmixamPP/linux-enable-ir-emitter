all: driver-generator execute-driver

flags = -O3 -lstdc++ 
warning_flags = -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return -Wcast-qual -Wconversion -Wunreachable-code -Winit-self
opencv_flags = `pkg-config opencv --cflags --libs`

driver-generator: driver-generator.cpp getquery.h lenquery.h setquery.h executequery.h driver.hpp
	g++ $^ ${flags} ${warning_flags} ${opencv_flags} -o $@ 

execute-driver: execute-driver.cpp executequery.h driver.hpp
	gcc $^ ${flags} ${warning_flags} -o $@

clean:
	rm -f driver-generator execute-driver

.PHONY: clean