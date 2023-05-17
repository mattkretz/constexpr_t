check:
	$(CXX) -c -Iinclude -O2 -Wall -Wextra -std=gnu++23 $(CXXFLAGS) test.cpp -o test.o

help:
	echo "... check"
