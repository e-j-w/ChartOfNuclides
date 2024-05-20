DEBUG_FLAGS = -g3 -pg -fsanitize=address,undefined -fsanitize-undefined-trap-on-error
CFLAGS = -O2 -Wall -Wextra -Wpedantic -Wc++-compat -Wdouble-promotion -Wshadow -Wunreachable-code -Wpointer-arith -Wcast-align -Wformat-security -Wstack-protector -Wnull-dereference -Wconversion -std=c99
#CFLAGS += $(DEBUG_FLAGS)
SDL = `pkg-config sdl3 --libs --cflags` -lSDL3_image -lSDL3_ttf
COMMON = include/formats.h include/enums.h include/gui_constants.h
OBJ = lib/juicer.o lib/basename.o io_ops.o load_data.o data_ops.o gui.o drawing.o process_events.o
INC =  -I./include -I./src -I./lib/basename -I./lib/juicer
CC = gcc
#CC = clang

run: con
	./con

install: con
	echo "Installation from this Makefile is not supported."

all: $(OBJ) proc_data con

con: src/*.c include/*.h $(OBJ)
	$(CC) src/app.c $(INC) $(OBJ) $(SDL) $(CFLAGS) -lm -o con

lib/juicer.o: lib/juicer/*.c lib/juicer/*.h
	$(CC) lib/juicer/juicer.c $(CFLAGS) -c -o lib/juicer.o

lib/basename.o: lib/basename/*.c lib/basename/*.h
	$(CC) lib/basename/basename.c $(CFLAGS) -c -o lib/basename.o

io_ops.o: src/io_ops.c include/io_ops.h $(COMMON)
	$(CC) src/io_ops.c $(INC) $(CFLAGS) -c -o io_ops.o

load_data.o: src/load_data.c include/load_data.h $(COMMON)
	$(CC) src/load_data.c $(INC) $(CFLAGS) -c -o load_data.o

data_ops.o: src/data_ops.c include/data_ops.h $(COMMON)
	$(CC) src/data_ops.c -I./lib/juicer $(INC) $(CFLAGS) -c -o data_ops.o

drawing.o: src/drawing.c include/drawing.h $(COMMON)
	$(CC) src/drawing.c -I./lib/juicer $(INC) $(CFLAGS) -c -o drawing.o

gui.o: src/gui.c include/gui.h $(COMMON)
	$(CC) src/gui.c -I./lib/juicer $(INC) $(CFLAGS) -c -o gui.o

process_events.o: src/process_events.c include/process_events.h $(COMMON)
	$(CC) src/process_events.c $(INC) $(CFLAGS) -c -o process_events.o

proc_data: data_processor/proc_data.c data_processor/proc_data.h proc_data_parser.o $(OBJ)
	$(CC) data_processor/proc_data.c proc_data_parser.o $(OBJ) -I./data_processor $(INC) $(SDL) $(CFLAGS) -lm -o proc_data

proc_data_parser.o: data_processor/proc_data_parser.c data_processor/proc_data_parser.h $(COMMON)
	$(CC) data_processor/proc_data_parser.c $(INC) $(CFLAGS) -c -o proc_data_parser.o

clean:
	rm -rf *~ *# */*.o *.o con proc_data
