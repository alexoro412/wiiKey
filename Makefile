CC=gcc
CFLAGS=-I/usr/local/include/hidapi -Wall -Wextra -pedantic -fsanitize=address 
LDFLAGS=-framework CoreGraphics -framework CoreFoundation -framework Carbon -framework IOKit -lhidapi

main: keyboard.o wiimote.o main.c
	$(CC) $(CFLAGS) -o main wiimote.o keyboard.o main.c $(LDFLAGS)

wiimote.o: wiimote.c wiimote.h
	$(CC) $(CFLAGS) -c wiimote.c

keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c keyboard.c 

.PHONY: clean
clean:
	rm *.o main
