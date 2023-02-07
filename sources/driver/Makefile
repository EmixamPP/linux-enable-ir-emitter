all: driver-generator execute-driver

# can't use c++20 because of opencv
flags = -O3 -std=c++17

warning_flags = -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings -Wcast-qual -Wconversion -Wunreachable-code -Winit-self
test_cmd = ./driver-generator /dev/video2 1 4 ~ 1

opencv_flags = $(shell pkg-config opencv --cflags --libs)
ifeq ($(opencv_flags),)
  opencv_flags = $(shell pkg-config opencv4 --cflags --libs)
endif

driver-generator: driver-generator.cpp camera.cpp logger.cpp finder.cpp driver.cpp
	g++ $^ ${flags} ${warning_flags} ${opencv_flags} -o $@ 

execute-driver: execute-driver.cpp camera.cpp logger.cpp driver.cpp
	g++ $^ ${flags} ${warning_flags} ${opencv_flags} -o $@

test: driver-generator
	${test_cmd}

valgrind: driver-generator
	valgrind --leak-check=full --show-leak-kinds=all --suppressions=/usr/share/opencv4/valgrind.supp --suppressions=/usr/share/opencv4/valgrind_3rdparty.supp ${test_cmd}

clean:
	rm -f driver-generator execute-driver

.PHONY: clean test valgrind

