CXX=gcc
CXXFLAGS= -Wall -O3
EXEC=main

all: $(EXEC)

main : main.o
	$(CXX) -pthread -o main main.o serveur.o client.o $(CXXFLAGS)

main.o : serveur.o client.o main.c
	$(CXX) -c main.c

serveur.o : serveur.c
	$(CXX) -c serveur.c

client.o : client.c
	$(CXX) -c client.c

clean:
	rm client.o serveur.o main.o main