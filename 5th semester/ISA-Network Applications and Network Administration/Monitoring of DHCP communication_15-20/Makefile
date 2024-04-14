# Makefile
# Makefile for dhcp-stats.cpp
# Kateřina Čepelková, xcepel03@stud.fit.vutbr.cz  

CXX = g++

CXXFLAGS = -std=c++20 -Wall -Wextra 

LDFLAGS = -lncurses -lpcap

all: dhcp-stats

dhcp-stats: dhcp-stats.cpp
	 $(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
tar:
	tar -cf xcepel03.tar Makefile dhcp-stats.cpp dhcp-stats.1 manual.pdf README

clean:
	rm dhcp-stats dhcp-stats.o
