#
# Specifiy the target
all:	cache-sim

# Specify the object files that the target depends on
# Also specify the object files needed to create the executable
cache-sim:	project2.o
	g++ project2.o  -o cache-sim

# Specify how the object files should be created from source files
project2.o:	project2.cpp
	gcc -g -c project2.cpp -o project2.o

# Specify the object files and executables that are generated
# and need to be removed to re-compile the whole thing
clean:
	rm -f *.o *~ cache-sim
