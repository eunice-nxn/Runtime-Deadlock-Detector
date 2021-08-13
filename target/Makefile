all:
	gcc -g deadlock_target.c -o deadlock_target -pthread
	gcc -g -o circular_target circular_target.c -pthread
	gcc -g -o new_target new_target.c -pthread
	gcc -o ddchck ../ddchck.c ../graph.c
	gcc -shared -fPIC -o ddmon.so ../ddmon.c -ldl -pthread
target:
	gcc deadlock_target.c -o deadlock_target -pthread
	gcc -g -o circular_target circular_target.c -pthread
	gcc -o new_target new_target.c -pthread
dd:
	gcc -o ddchck ../graph.c ../ddchck.c
	gcc -shared -fPIC -o ddmon.so ../ddmon.c -ldl -pthread

clean:
	rm -rf deadlock_target circular_target new_target
	rm -rf ddchck ddmon.so 
