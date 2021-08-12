all:
	gcc -o ddchck ../ddchck.c
	gcc -o new_target new_target.c -pthread
	gcc -shared -fPIC -o ddmon.so ../ddmon.c -ldl -pthread
