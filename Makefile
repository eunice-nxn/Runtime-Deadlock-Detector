all:
	gcc -o test test.c -pthread
	gcc -shared -fPIC -o mypthread.so mypthread.c -ldl -pthread
