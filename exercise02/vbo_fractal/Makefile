CC = clang++
CFLAGS = -Wall
LDFLAGS = -framework OpenGL -framework GLUT
BIN = vbo_fractal
OBJ = vbo_fractal.o triangle.o vector.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(BIN)
