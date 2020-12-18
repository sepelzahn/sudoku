target = sudoku

all: $(target)

CXX = gcc
CXXFLAGS = -std=c++11 -g
LDLIBS = -lstdc++

$(target): $(target).cpp
	scl enable devtoolset-4 "$(CXX) -o $@ $(CXXFLAGS) $< $(LDLIBS)"

.PHONY: clean
clean:
	rm -f $(target)
