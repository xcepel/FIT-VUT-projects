CXX = g++
CXXFLAGS = -std=c++11 -Wall -g -Wextra
LIBS = -lm
SRC = generation.cpp all_matrix.cpp
HEADERS = generation.h all_matrix.h
EXECUTABLE = deforestation

$(EXECUTABLE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

all: $(EXECUTABLE)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:	
	rm -f *.core $(EXECUTABLE) *~ *.o