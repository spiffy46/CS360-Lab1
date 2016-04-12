# Makefile for Lab1

CXX =			g++ $(CCFLAGS)

SERVER=			msgd.o
CLIENT=			msg.o
OBJS=			$(SERVER) $(CLIENT)

LIBS=

CCFLAGS = -g

all:			server client

server:$(SERVER)
	$(CXX) -o server $(SERVER) $(LIBS)

client:$(CLIENT)
	$(CXX) -o client $(CLIENT) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) server client
