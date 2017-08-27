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
	$(CC) -o$@ $(FLAGS) $(FILE)  -lpthread -ljemalloc
clean:
	rm -rf *.o server
