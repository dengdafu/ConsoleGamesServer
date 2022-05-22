CC=g++

PROGRAM_NAME=consoleGamesServer

OBJ=main.o

consoleGamesServer: main.o
	$(CC) -o $(PROGRAM_NAME) $(OBJ) -pthread

main.o: main.cpp
	$(CC) -c main.cpp

clean:
	rm *.o consoleGamesServer