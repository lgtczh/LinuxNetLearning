.PHONY : all clean
objects = echosrv.o echocli.o

all : $(objects)

echocli.o:my_function.c ../err_exit.h 03echocli_nostick_select.c
	gcc my_function.o 03echocli_nostick_select.c -o echocli.o

echosrv.o:my_function.c ../err_exit.h 03echosrv_nostick_poll.c
	gcc my_function.o 03echosrv_nostick_poll.c -o echosrv.o

my_function.o:my_function.c ../err_exit.h
	gcc -c my_function.c
clean :
	rm $(objects)

