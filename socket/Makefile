.PHONY : all clean
objects = 03echosrv_ppc_defineprotocol.o 03echocli_defineprotocol.o my_function.o \
03echosrv_recv.o 03echocli_recv.o

all : $(objects)

03echosrv_ppc_defineprotocol.o:my_function.o 03echosrv_ppc_defineprotocol.c err_exit.h packet.h
	gcc my_function.o 03echosrv_ppc_defineprotocol.c -o 03echosrv_ppc_defineprotocol.o
03echocli_defineprotocol.o:my_function.o 03echocli_defineprotocol.c err_exit.h packet.h
	gcc my_function.o 03echocli_defineprotocol.c -o 03echocli_defineprotocol.o

03echosrv_recv.o:my_function.o 03echosrv_recv.c err_exit.h
	gcc my_function.o 03echosrv_recv.c -o 03echosrv_recv.o
03echocli_recv.o:my_function.o 03echocli_recv.c err_exit.h
	gcc my_function.o 03echocli_recv.c -o 03echocli_recv.o

my_function.o:my_function.c
	gcc -c my_function.c

clean :
	rm $(objects)

