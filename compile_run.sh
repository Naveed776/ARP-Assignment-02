## Naveed Manzoor Afridi	5149575	##
## Abdul Rauf	 		5226212	##


# Compile process A
gcc src/processA.c -lncurses -lbmp -lm -lrt -pthread -o bin/processA &

# Compile process B
gcc src/processB.c -lncurses -lbmp -lm -lrt -pthread -o bin/processB &

# Compile master process
gcc src/master.c -lrt -pthread -o bin/master

#Run the program
./bin/master
