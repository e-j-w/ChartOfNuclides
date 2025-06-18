DEBUG_FLAGS = -g3 -pg -fsanitize=address,undefined -fsanitize-undefined-trap-on-error
CFLAGS = -O2 -Wall -Wextra -Wpedantic -Wc++-compat -Wdouble-promotion -Wshadow -Wunreachable-code -Wpointer-arith -Wcast-align -Wformat-security -Wstack-protector -Wnull-dereference -Wconversion -std=c99
#CFLAGS += $(DEBUG_FLAGS)
SDL = `pkg-config sdl3 --libs --cflags` -lSDL3_image -lSDL3_ttf
COMMON = include/formats.h include/enums.h include/gui_constants.h
OBJ = lib/fontcache.o lib/strops.o lib/juicer.o io_ops.o load_data.o data_ops.o search_ops.o gui.o drawing.o process_events.o thread_manager.o
INC =  -I./include -I./src -I./lib/fontcache -I./lib/juicer -I./lib/strops
CC = gcc
#CC = clang

run: chart
	./chart

install-linux: chart
	@echo "Will install executable to /usr/bin and data to /usr/share/chart."
	@echo "Run 'make uninstall-linux' to undo installation."
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo "This must be run with administrator privileges (eg. with 'sudo')." ; \
	else \
		mkdir /usr/share/chart ; \
		install -Dm755 chart -t /usr/bin ; \
		install -Dm644 chart.dat -t /usr/share/chart ; \
		install -Dm644 data/io.github.e_j_w.ChartOfNuclides.svg -t /usr/share/icons/hicolor/scalable/apps ; \
		install -Dm644 data/io.github.e_j_w.ChartOfNuclides.desktop -t /usr/share/applications ; \
		update-desktop-database /usr/share/applications ; \
		gtk-update-icon-cache -f -t /usr/share/icons/hicolor ; \
		echo "Done!" ; \
	fi

uninstall-linux:
	@echo "Will undo changes made from running 'make install-linux'."
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo "This must be run with administrator privileges (eg. with 'sudo')." ; \
	else \
		rm /usr/bin/chart ; \
		rm /usr/share/chart/chart.dat ; \
		rmdir /usr/share/chart ; \
		rm /usr/share/icons/hicolor/scalable/apps/io.github.e_j_w.ChartOfNuclides.svg ; \
		rm /usr/share/applications/io.github.e_j_w.ChartOfNuclides.desktop ; \
		update-desktop-database /usr/share/applications ; \
		echo "Done!" ; \
	fi

all: $(OBJ) proc_data chart

chart: src/*.c include/*.h $(OBJ) chart.dat
	$(CC) src/app.c $(INC) $(OBJ) $(SDL) $(CFLAGS) -lm -o chart

chart.dat: proc_data
	@if [ ! -f chart.dat ]; then \
		./proc_data ; \
	fi

lib/juicer.o: lib/juicer/*.c lib/juicer/*.h
	$(CC) lib/juicer/juicer.c $(CFLAGS) -c -o lib/juicer.o

lib/strops.o: lib/strops/*.c lib/strops/*.h
	$(CC) lib/strops/strops.c $(CFLAGS) -c -o lib/strops.o

lib/fontcache.o: lib/fontcache/*.c lib/fontcache/*.h
	$(CC) lib/fontcache/SDL_FontCache.c $(CFLAGS) -c -o lib/fontcache.o

io_ops.o: src/io_ops.c include/io_ops.h $(COMMON)
	$(CC) src/io_ops.c $(INC) $(CFLAGS) -c -o io_ops.o

load_data.o: src/load_data.c include/load_data.h $(COMMON)
	$(CC) src/load_data.c $(INC) $(CFLAGS) -c -o load_data.o

data_ops.o: src/data_ops.c include/data_ops.h $(COMMON)
	$(CC) src/data_ops.c $(INC) $(CFLAGS) -c -o data_ops.o

search_ops.o: src/search_ops.c include/search_ops.h $(COMMON)
	$(CC) src/search_ops.c $(INC) $(CFLAGS) -c -o search_ops.o

drawing.o: src/drawing.c include/drawing.h $(COMMON)
	$(CC) src/drawing.c $(INC) $(CFLAGS) -c -o drawing.o

gui.o: src/gui.c include/gui.h $(COMMON)
	$(CC) src/gui.c $(INC) $(CFLAGS) -c -o gui.o

process_events.o: src/process_events.c include/process_events.h $(COMMON)
	$(CC) src/process_events.c $(INC) $(CFLAGS) -c -o process_events.o

thread_manager.o: src/thread_manager.c include/thread_manager.h $(COMMON)
	$(CC) src/thread_manager.c $(INC) $(CFLAGS) -c -o thread_manager.o

proc_data: data_processor/proc_data.c data_processor/proc_data.h proc_data_parser.o $(OBJ)
	$(CC) data_processor/proc_data.c proc_data_parser.o $(OBJ) -I./data_processor $(INC) $(SDL) $(CFLAGS) -lm -o proc_data

proc_data_parser.o: data_processor/proc_data_parser.c data_processor/proc_data_parser.h $(COMMON)
	$(CC) data_processor/proc_data_parser.c $(INC) $(CFLAGS) -c -o proc_data_parser.o

clean:
	rm -rf *~ *# */*.o *.o chart proc_data chart.dat
