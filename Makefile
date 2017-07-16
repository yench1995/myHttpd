all: myHttpd client

myHttpd: myHttpd.c
			gcc -W -Wall -o myHttpd myHttpd.c -lpthread

client:  client.c
			gcc -o client client.c
clean:
		    rm myHttpd
