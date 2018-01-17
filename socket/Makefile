.PHONY : all clean
objects = 03echosrv_ppc_defineprotocol.o 03echocli_defineprotocol.o my_function.o \
03echosrv_recv.o 03echocli_recv.o 03echosrv_ppc_nostick.o 03echocli_nostick.o 03echocli_nostick_select.o 03echosrv_nostick_select.o 03echosrv_nostick_select_shutdown.o 03echocli_nostick_select_shutdown.o# \
#03echocli.o 03echosrv.o
all : $(objects)

#03echosrv.o:03echosrv.c err_exit.h
#	gcc 03echosrv.c -o 03echosrv.o
#03echocli.o:03echocli.c err_exit.h
#	gcc 03echocli.c -o 03echocli.o

03echosrv_nostick_select_shutdown.o:my_function.o ../err_exit.h 03echosrv_nostick_select_shutdown.c
	gcc my_function.o 03echosrv_nostick_select_shutdown.c -o 03echosrv_nostick_select_shutdown.o

03echocli_nostick_select_shutdown.o:my_function.o ../err_exit.h 03echocli_nostick_select_shutdown.c
	gcc my_function.o 03echocli_nostick_select_shutdown.c -o 03echocli_nostick_select_shutdown.o

03echosrv_nostick_select.o:my_function.o ../err_exit.h 03echosrv_nostick_select.c
	gcc my_function.o 03echosrv_nostick_select.c -o 03echosrv_nostick_select.o

03echosrv_ppc_nostick.o:my_function.o ../err_exit.h 03echosrv_ppc_nostick.c
	gcc my_function.o 03echosrv_ppc_nostick.c -o 03echosrv_ppc_nostick.o
03echocli_nostick.o:my_function.o ../err_exit.h 03echocli_nostick.c
	gcc my_function.o 03echocli_nostick.c -o 03echocli_nostick.o

03echocli_nostick_select.o:my_function.o ../err_exit.h 03echocli_nostick.c
	gcc my_function.o 03echocli_nostick_select.c -o 03echocli_nostick_select.o

03echosrv_ppc_defineprotocol.o:my_function.o 03echosrv_ppc_defineprotocol.c ../err_exit.h packet.h
	gcc my_function.o 03echosrv_ppc_defineprotocol.c -o 03echosrv_ppc_defineprotocol.o
03echocli_defineprotocol.o:my_function.o 03echocli_defineprotocol.c ../err_exit.h packet.h
	gcc my_function.o 03echocli_defineprotocol.c -o 03echocli_defineprotocol.o

03echosrv_recv.o:my_function.o 03echosrv_recv.c ../err_exit.h
	gcc my_function.o 03echosrv_recv.c -o 03echosrv_recv.o
03echocli_recv.o:my_function.o 03echocli_recv.c ../err_exit.h
	gcc my_function.o 03echocli_recv.c -o 03echocli_recv.o

my_function.o:my_function.c
	gcc -c my_function.c

clean :
	rm $(objects)

