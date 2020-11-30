#
# Makefile
#
SRC=src
CC=gcc
all: led-server.out button-client.out

$(SRC)/%.o: $(SRC)/%.c 
	gcc -g3  -Wall -c `pkg-config --cflags dbus-1` $< -o $@

led-server.out: $(SRC)/led_process.o
	$(CC) ${SRC}/led_process.o -o led-server.out `pkg-config --libs dbus-1` -lhardware

button-client.out: $(SRC)/button_process.o
	$(CC) $(SRC)/button_process.o -o button-client.out `pkg-config --libs dbus-1` -lhardware

.PHONY: clean
clean:
	rm $(SRC)/*.o *.out