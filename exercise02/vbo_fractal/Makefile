CC = clang++
CFLAGS = -Wall
LDFLAGS = -framework OpenGL -framework GLUT
BIN = $(shell basename `pwd`)
OBJ = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

all: $(BIN)
	./$(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm $(OBJ) $(BIN)
