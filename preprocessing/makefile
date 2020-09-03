all:
	# Stop after preprocessing stage; do not run compiler
	gcc -E main.c -o main.i
	# Stop after the stage of compilation proper; do not assemble
	gcc -S main.i -o main.s
	# Compile or assemble the sourcefile, but do not link.
	gcc -c main.s -o main.o
	gcc main.o -o main