CFLAGS += -g -DDEBUG -O -Wall -Werror -I${CLFSPROJ}/targets/include -I${CLFSPROJ}/custom_packages/cross_injector/src
LIBS =  -lpcap -lpthread
LFLAGS = -L ${CLFSPROJ}/targetfs/lib

all: lisa.o history.o queues.o main.o
	${CC} ${CFLAGS} ${LFLAGS} ${LIBS} -o bin/Debug/cross_injector obj/Debug/lisa.o obj/Debug/history.o obj/Debug/queues.o obj/Debug/main.o
	cp bin/Debug/cross_injector ${CLFSPROJ}/targetfs/root/ci

lisa.o:
	${CC} ${CFLAGS} -o obj/Debug/lisa.o -c src/lisa.c 

history.o:
	${CC} ${CFLAGS} -o obj/Debug/history.o -c src/history.c 


queues.o:
	${CC} ${CFLAGS} -o obj/Debug/queues.o -c src/queues.c 


main.o:
	${CC} ${CFLAGS} -o obj/Debug/main.o -c src/main.c 


clean:
	rm -f obj/Debug/*.o
	rm -f bin/Debug/cross_injector	