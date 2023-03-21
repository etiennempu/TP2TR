CXX=gcc
CXXFLAGS= -Wall -O3
EXEC=main

all: $(EXEC)

main : main.o
        $(CXX) -pthread -o main main.o serveur.o $(CXXFLAGS)

main.o : serveur.o main.c
        $(CXX) -c main.c

serveur.o : serveur.c
        $(CXX) -c serveur.c

clean:
        rm main