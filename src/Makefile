# makefile created automaticaly by LFMakeMaker
# LFMakeMaker 1.4 (Apr  3 2018 23:14:49) (c)LFSoft 1997

gotoall: all


#The compiler (may be customized for compiler's options).
cc=cc
opts=`pkg-config --cflags json-c` `pkg-config --libs json-c` -lssl -lcrypto -lcurl -Wall -DxDEBUG -Wno-parentheses

FCMSubmit.o : FCMSubmit.c Makefile 
	$(cc) -c -o FCMSubmit.o FCMSubmit.c $(opts) 

../FCMSubmit : FCMSubmit.o Makefile 
	 $(cc) -o ../FCMSubmit FCMSubmit.o $(opts) 

all: ../FCMSubmit 
