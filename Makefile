CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -Isrc

SRCS = $(filter-out src/quest.cpp,$(wildcard src/*.cpp))
OBJS = $(SRCS:.cpp=.o)
TARGET = galactic_miner

SFML_LIBS = $(shell pkg-config --libs sfml-graphics sfml-window sfml-system sfml-audio 2>/dev/null)
SFML_CFLAGS = $(shell pkg-config --cflags sfml-graphics sfml-window sfml-system sfml-audio 2>/dev/null)

all: check_sfml $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(SFML_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SFML_CFLAGS) -c $< -o $@

check_sfml:
	@pkg-config --exists sfml-graphics || (\
	echo "SFML not found, installing..." && \
	sudo apt-get update && sudo apt-get install -y libsfml-dev)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re check_sfml
