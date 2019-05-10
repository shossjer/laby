
CXX=clang++

all: main.cpp
	${CXX} -std=c++14 -Wall -o laby main.cpp
