CC=g++
CPPV=-std=c++17
CF=-Wall

all:main.o requests.o
	$(CC) $(CPPV) requests.o main.o -o main -L/usr/local/ssl/lib64 -lcrypto -lssl $(CF)
	rm main.o requests.o requests.hpp.gch
main.o: main.cpp
	$(CC) $(CPPV) -c main.cpp $(CF)

requests.o: requests.cpp requests.hpp
	$(CC) $(CPPV) -c requests.cpp requests.hpp $(CF)
clean:
	rm -rf main

# Checking files
check: check.o requests.o
	$(CC) $(CPPV) requests.o check.o -o check -L/usr/local/ssl/lib64 -lcrypto -lssl $(CF)
	rm check.o requests.o requests.hpp.gch
check.o: check.cpp
	$(CC) $(CPPV) -c check.cpp $(CF)

clean_check:
	rm -rf check
