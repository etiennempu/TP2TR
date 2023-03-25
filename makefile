CXX=gcc
CXXFLAGS= -Wall -O3
EXEC=main

all: $(EXEC)

main : main.o
        $(CXX) -pthread -o main main.o serveur.o client.o Led.o $(CXXFLAGS)

main.o : Led.o serveur.o client.o main.c
        $(CXX) -c main.c

serveur.o : serveur.c
        $(CXX) -c serveur.c

client.o : client.c
        $(CXX) -c client.c

Led.o : Led.c
        $(CXX) -c Led.c

clean:
        rm client.o serveur.o Led.o main.o main

