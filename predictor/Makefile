all: graph.h
	gcc -o ddpred ddpred.c graph.c
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl -pthread
	gcc -g -o new_single_target_1 target/new_single_target.c -pthread
	gcc -g -o single_target_1 target/single_target_1.c -pthread
	gcc -g -o single_target_2 target/single_target_2.c -pthread
	gcc -g -o single_target_3 target/single_target_3.c -pthread
	gcc -g -o single_target_4 target/single_target_4.c -pthread
	gcc -g -o single_target_5 target/single_target_5.c -pthread
	gcc -g -o deadlock_target ../target/deadlock_target.c -pthread
	gcc -g -o circular_target ../target/circular_target.c -pthread
	gcc -g -o new_target ../target/new_target.c -pthread
clean:
	rm -rf single_target_1 single_target_2 single_target_3
	rm -rf single_target_4 single_target_5
	rm -rf deadlock_target circular_target new_target
	rm -rf ddmon.so
