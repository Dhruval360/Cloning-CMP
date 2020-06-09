CMP : cmp_client.o cmp_server.o 
	gcc cmp_client.o cmp_server.o cmp_server.h -lm -o CMP
cmp_client.o : cmp_client.c cmp_server.h 
	gcc -c cmp_client.c cmp_server.h -lm
cmp_server.o : cmp_server.c cmp_server.h 
	gcc -c cmp_server.c cmp_server.h -lm
