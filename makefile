CXX = g++
CXXFLAGS = -std=c++20 -Wall
SRC_FILES = $(wildcard *.cpp)
OBJ_FILES = $(SRC_FILES:.cpp=.o)
EXEC = main
FRAMEWORKS = -framework AudioToolbox -framework CoreFoundation

all: $(EXEC)

$(EXEC): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(FRAMEWORKS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run:
	clear
	./main script.txt

clean:
	rm -f $(EXEC) $(OBJ_FILES)
	clear

.PHONY: all clean
