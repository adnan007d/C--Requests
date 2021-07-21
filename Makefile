CC=g++

all:main.o requests.o
	$(CC) requests.o main.o -o main
	rm main.o requests.o requests.hpp.gch
main.o: main.cpp
	$(CC) -c main.cpp

requests.o: requests.cpp requests.hpp
	$(CC) -c requests.cpp requests.hpp 
clean:
	rm -rf main

# Checking files
check: check.o requests.o
	$(CC) requests.o check.o -o check
	rm check.o requests.o requests.hpp.gch
check.o: check.cpp
	$(CC) -c check.cpp

clean_check:
	rm -rf check
