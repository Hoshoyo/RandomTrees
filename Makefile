CC=g++
FLAGS=-I./include -g -m64 -std=c++11
BINDIR=./bin
SRCDIR=./src

all:
	mkdir -p ./bin
	$(CC) $(FLAGS) $(SRCDIR)/main.cpp $(SRCDIR)/parser.cpp $(SRCDIR)/util.cpp -o $(BINDIR)/randtrees -lm
