all: myHttpd 

myHttpd: myHttpd.c
			gcc -W -Wall -o myHttpd myHttpd.c -lpthread

clean:
		    rm myHttpd
