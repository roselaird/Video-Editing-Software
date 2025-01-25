CC = gcc
CFLAGS = -Wall -Wextra -O3 -fopenmp -msse2 -mavx -mavx2
LDFLAGS = -fopenmp

LIBRARY = libFilmMaster2000.a
EXECUTABLE = runme

SRC = film_library.c film_library_plus.c runme.c
OBJ = $(SRC:.c=.o)

all: $(LIBRARY) $(EXECUTABLE)

$(LIBRARY): film_library.o film_library_plus.o
	ar rcs $(LIBRARY) film_library.o film_library_plus.o

$(EXECUTABLE): runme.o $(LIBRARY)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) runme.o -L. -lFilmMaster2000

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIBRARY) $(EXECUTABLE)

test: $(EXECUTABLE)
	@echo "Running tests..."
	./$(EXECUTABLE) test.bin output.bin reverse
	./$(EXECUTABLE) test.bin output.bin swap_channel 1,2
	./$(EXECUTABLE) test.bin output.bin clip_channel 1 [10,200]
	./$(EXECUTABLE) test.bin output.bin scale_channel 1 1.5
	./$(EXECUTABLE) test.bin output.bin speed_up 2
	./$(EXECUTABLE) test.bin output.bin crop_aspect 16:9
