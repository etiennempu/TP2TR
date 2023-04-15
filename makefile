CXX=gcc
CXXFLAGS= -Wall -pedantic -Og -pthread
EXEC=main

all: $(EXEC)

main : main.o
	$(CXX) $(CXXFLAGS) -o main main.o gaz.o serveur.o client.o utils.o Led.o

main.o : utilities main.c
	$(CXX) -c main.c

utilities : gaz.c serveur.c client.c utils.c Led.c
	$(CXX) -c gaz.c serveur.c client.c utils.c Led.c

clean:
	rm utils.o client.o serveur.o gaz.o Led.o main.o main
