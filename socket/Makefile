objects = 03echosrv_ppc_defineprotocol.o 03echocli_defineprotocol.o my_function.o
edit : $(objects)
	gcc -o edit $(objects)
03echosrv_ppc_defineprotocol.o:my_function.o 03echosrv_ppc_defineprotocol.c
	gcc my_function.o 03echosrv_ppc_defineprotocol.c -o 03echosrv_ppc_defineprotocol.o
03echocli_defineprotocol.o:my_function.o 03echocli_defineprotocol.c
	gcc my_function.o 03echocli_defineprotocol.c -o 03echocli_defineprotocol.o
my_function.o:my_function.c
	gcc -c my_function.c

