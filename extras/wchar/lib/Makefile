
all: libwcs.a

libwcs.a::
	${CC} -c -I../h *.c
	ar rv libwcs.a *.o
	ranlib libwcs.a
	rm -f *.o

clean::
	rm -f *.a *.o
