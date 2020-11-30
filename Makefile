#
# Makefile
#
SRC=src
CC=gcc
all: led_process.out button_process.out

$(SRC)/%.o: $(SRC)/%.c 
	gcc -g3  -Wall -c `pkg-config --cflags dbus-1` $< -o $@

led_process.out: $(SRC)/led_process.o
	$(CC) ${SRC}/led_process.o -o led_process.out `pkg-config --libs dbus-1` -lhardware

button_process.out: $(SRC)/button_process.o
	$(CC) $(SRC)/button_process.o -o button_process.out `pkg-config --libs dbus-1` -lhardware

.PHONY: clean
clean:
	rm $(SRC)/*.o *.out