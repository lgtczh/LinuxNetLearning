.PHONY : all clean
objects = chatsrv.o chatcli.o

all : $(objects)
chatcli.o:../../err_exit.h pub.h chatcli.cpp
	g++ chatcli.cpp -o chatcli.o

chatsrv.o:../../err_exit.h pub.h chatsrv.cpp
	g++ chatsrv.cpp -o chatsrv.o
clean :
	rm $(objects)
