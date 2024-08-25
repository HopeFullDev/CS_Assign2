build: main.cpp
	g++ main.cpp -std=c++23 -Wall

run: build
	./a.out