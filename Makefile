############################################################
APP=cobalt
CC=g++
CFLAGS=-W -Wall -c -std=c++0x
LFLAGS=-W

############################################################
all:
	${CC} ${CFLAGS} ${APP}.cpp -o ${APP}.o
	${CC} ${CFLAGS} str.cpp -o str.o
	${CC} ${CFLAGS} irc.cpp -o irc.o
	${CC} ${CFLAGS} scanner.cpp -o scanner.o
	${CC} ${CFLAGS} config.cpp -o config.o
	${CC} ${CFLAGS} commands.cpp -o commands.o
	${CC} ${CFLAGS} utility.cpp -o utility.o
	${CC} ${CFLAGS} wad.cpp -o wad.o
	${CC} ${LFLAGS} ${APP}.o str.o irc.o scanner.o config.o commands.o utility.o \
		wad.o -o ${APP}

############################################################
clean:
	rm -f *.o ${APP}
