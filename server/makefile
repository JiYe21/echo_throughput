CC = g++
FLAGS =   -g
BIN=server
FILE:= ev.cpp \
     main.cpp\
     sess.cpp\
     pkt.cpp\
     tcpManager.cpp\
     app.cpp

$(BIN):
	$(CC) -O2 -o$@ $(FLAGS) $(FILE)  -lpthread -ljemalloc
clean:
	rm -rf *.o server
