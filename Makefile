CC=g++
CPPV=-std=c++17
CF=-Wall

all:main.o requests.o
	$(CC) $(CPPV) requests.o main.o -o main -L/usr/local/ssl/lib64 -lcrypto -lssl $(CF)

main.o: main.cpp
	$(CC) $(CPPV) -c main.cpp $(CF)

requests.o: requests.cpp requests.hpp
	$(CC) $(CPPV) -c requests.cpp requests.hpp $(CF)

clean:
	rm -rf main

meme: meme.o requests.o
	$(CC) $(CPPV) requests.o meme.o -o meme -L/usr/local/ssl/lib64 -lcrypto -lssl $(CF)
meme.o: meme.cpp
	$(CC) $(CPPV) -c meme.cpp $(CF)

brute: brute-force.o requests.o
	$(CC) $(CPPV) requests.o brute-force.o -o brute -L/usr/local/ssl/lib64 -lcrypto -lssl $(CF)
brute.o: brute-force.cpp
	$(CC) $(CPPV) -c brute-force.cpp $(CF)


# Checking files
check: check.o requests.o
	$(CC) $(CPPV) requests.o check.o -o check -L/usr/local/ssl/lib64 -lcrypto -lssl $(CF)

check.o: check.cpp
	$(CC) $(CPPV) -c check.cpp $(CF)

clean_check:
	rm -rf check
