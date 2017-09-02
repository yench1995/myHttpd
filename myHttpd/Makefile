LIBS=-pthread
OBJECT=server client
all : $(OBJECT)

server : myHttpd.cpp parse.cpp serverMain.cpp 
		g++ -std=c++11 $^ -o $@ $(LIBS)

client : client.cpp
		g++ -std=c++11 $^ -o $@

